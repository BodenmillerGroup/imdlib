#include "IMDFile.h"

namespace imd {

    std::string IMDFile::toUTF8(const std::vector<char> &vec) {
        std::string s(vec.size() / 2, ' ');
        for (std::size_t i = 0; i < s.size(); ++i) {
            s[i] = vec[2 * i];
        }
        return std::move(s);
    }

    std::vector<char> IMDFile::toUTF16(const std::string &s) {
        std::vector<char> vec;
        vec.reserve(2 * s.length());
        for (const char &c : s) {
            vec.push_back(c);
            vec.push_back(0x00);
        }
        return std::move(vec);
    }

    template<class T>
    bool
    IMDFile::searchVectorBackwards(const std::vector<T> &vec, const std::vector<T> &pattern, std::size_t *foundPos) {
        std::size_t patternPos = 0;
        std::size_t currentOffset = pattern.size();
        while (currentOffset <= vec.size()) {
            const std::size_t currentPos = vec.size() - currentOffset;
            if (vec[currentPos + patternPos] == pattern[patternPos]) {
                patternPos += 1;
                if (patternPos == pattern.size()) {
                    *foundPos = currentPos;
                    return true;
                }
            } else {
                patternPos = 0;
                currentOffset++;
            }
        }
        return false;
    }

    bool IMDFile::searchFileBackwards(std::ifstream &file, const std::string &pattern, std::streamoff *foundPos,
                                      std::streamoff startPos) {
        // convert pattern
        const std::vector<char> patternVector = toUTF16(pattern);
        // prepare buffer
        std::vector<char> searchBuffer(SEARCH_BUFFER_SIZE + patternVector.size());
        char *const cachePtr = searchBuffer.data() + SEARCH_BUFFER_SIZE;
        // determine file size
        file.seekg(0, std::ios_base::end);
        const std::streamoff fileSize = file.tellg();
        // initialize search position
        std::streamoff chunkEndPos = startPos;
        if (startPos <= 0 || startPos >= fileSize) {
            chunkEndPos = fileSize;
        }
        // search file
        while (chunkEndPos >= SEARCH_BUFFER_SIZE) {
            // read chunk
            const std::streamoff chunkStartPos = chunkEndPos - SEARCH_BUFFER_SIZE;
            file.seekg(chunkStartPos, std::ios_base::beg);
            file.read(searchBuffer.data(), SEARCH_BUFFER_SIZE);
            // find in chunk
            std::size_t relativeFoundPos;
            if (IMDFile::searchVectorBackwards(searchBuffer, patternVector, &relativeFoundPos)) {
                *foundPos = chunkStartPos + relativeFoundPos;
                return true;
            }
            // update buffer
            std::copy(searchBuffer.data(), searchBuffer.data() + patternVector.size(), cachePtr);
            chunkEndPos = chunkStartPos;
        }
        // search remainder
        if (chunkEndPos > 0) {
            // update buffer
            char *const remainderCachePtr = searchBuffer.data() + chunkEndPos;
            std::copy(cachePtr, cachePtr + patternVector.size(), remainderCachePtr);
            searchBuffer.resize(chunkEndPos + patternVector.size());
            // read remainder
            file.seekg(0, std::ios_base::beg);
            file.read(searchBuffer.data(), chunkEndPos);
            // find in remainder
            std::size_t relativeFoundPos;
            if (IMDFile::searchVectorBackwards(searchBuffer, patternVector, &relativeFoundPos)) {
                *foundPos = relativeFoundPos;
                return true;
            }
        }
        return false;
    }

    IMDFile::IMDFile(const std::string &path)
            : path(path) {
    }

    std::string IMDFile::readText(std::ifstream &file, const std::streamoff &startPos, const std::streamoff &endPos) {
        std::vector<char> buffer((std::size_t) endPos - startPos);
        file.seekg(startPos, std::ios_base::beg);
        file.read(buffer.data(), buffer.size());
        return toUTF8(buffer);
    }

    const IMDFileData IMDFile::readData() const {
        // open file
        std::ifstream file(path, std::ios_base::binary);
        if (!file) {
            throw IMDFileIOException("Could not open file " + path);
        }
        // find xml end tag
        std::streamoff xmlEndPos;
        bool xmlEndFound = searchFileBackwards(file, EXPERIMENT_SCHEMA_END, &xmlEndPos);
        if (!xmlEndFound) {
            throw IMDFileMalformedException("Could not find XML end tag " EXPERIMENT_SCHEMA_END);
        }
        // find xml start tag
        std::streamoff xmlStartPos;
        bool xmlStartFound = searchFileBackwards(file, EXPERIMENT_SCHEMA_START, &xmlStartPos, xmlEndPos);
        if (!xmlStartFound) {
            throw IMDFileMalformedException("Could not find XML start tag " EXPERIMENT_SCHEMA_START);
        }
        // parse metadata
        pugi::xml_document doc;
        std::string metadata = readText(file, xmlStartPos, xmlEndPos + toUTF16(EXPERIMENT_SCHEMA_END).size());
        pugi::xml_parse_result parse_result = doc.load_string(metadata.c_str());
        if (!parse_result) {
            std::string error_message(parse_result.description());
            throw IMDFileMalformedException("Failed to parse experiment schema xml: " + error_message);
        }
        // collect marker names
        std::vector<std::string> markerNames;
        for (const pugi::xpath_node &node : doc.select_nodes(MARKER_SHORTNAME_XPATH)) {
            markerNames.emplace_back(node.node().text().get());
        }
        // read data
        IMDFileData data(markerNames);
        file.seekg(0, std::ios_base::beg);
        std::vector<uint16_t> buffer(2 * markerNames.size());
        const auto numPushes = xmlStartPos / (4 * markerNames.size());
        for (std::size_t push = 0; push < numPushes; ++push) {
            data.pushOffsets.push_back(data.markerIndices.size());
            file.read((char *) buffer.data(), 2 * buffer.size());
            for (std::size_t markerIndex = 0; markerIndex < markerNames.size(); ++markerIndex) {
                std::uint16_t intensityValue = buffer[2 * markerIndex];
                if (intensityValue > 0) {
                    data.markerIndices.push_back(markerIndex);
                    data.intensityValues.push_back(intensityValue);
                    data.pulseValues.push_back(buffer[2 * markerIndex + 1]);
                }
            }
        }
        data.pushOffsets.push_back(data.markerIndices.size());
        return data;
    }

    std::string IMDFile::readMetadata() const {
        // open file
        std::ifstream file(path, std::ios_base::binary);
        if (!file) {
            throw IMDFileIOException("Could not open file " + path);
        }
        // find xml end tag
        std::streamoff xmlEndPos;
        bool xmlEndFound = searchFileBackwards(file, EXPERIMENT_SCHEMA_END, &xmlEndPos);
        if (!xmlEndFound) {
            throw IMDFileMalformedException("Could not find XML end tag " EXPERIMENT_SCHEMA_END);
        }
        // find xml start tag
        std::streamoff xmlStartPos;
        bool xmlStartFound = searchFileBackwards(file, EXPERIMENT_SCHEMA_START, &xmlStartPos, xmlEndPos);
        if (!xmlStartFound) {
            throw IMDFileMalformedException("Could not find XML start tag " EXPERIMENT_SCHEMA_START);
        }
        // read metadata
        return readText(file, xmlStartPos, xmlEndPos + toUTF16(EXPERIMENT_SCHEMA_END).size());
    }

}
