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

    std::string IMDFile::readText(std::ifstream &file, const std::streamoff &startPos, const std::streamoff &endPos) {
        std::vector<char> buffer((std::size_t) endPos - startPos);
        file.seekg(startPos, std::ios_base::beg);
        file.read(buffer.data(), buffer.size());
        return toUTF8(buffer);
    }

    std::string
    IMDFile::readMetadataInternal(std::ifstream &file, std::streamoff *xmlStartPos, std::streamoff *xmlEndPos) {
        bool xmlEndFound = searchFileBackwards(file, EXPERIMENT_SCHEMA_END, xmlEndPos);
        if (!xmlEndFound) {
            throw IMDFileMalformedException("Could not find XML end tag " EXPERIMENT_SCHEMA_END);
        }
        bool xmlStartFound = searchFileBackwards(file, EXPERIMENT_SCHEMA_START, xmlStartPos, *xmlEndPos);
        if (!xmlStartFound) {
            throw IMDFileMalformedException("Could not find XML start tag " EXPERIMENT_SCHEMA_START);
        }
        std::string metadata = readText(file, *xmlStartPos, *xmlEndPos + toUTF16(EXPERIMENT_SCHEMA_END).size());
        metadata = std::regex_replace(metadata, std::regex("&lt;"), "<");
        metadata = std::regex_replace(metadata, std::regex("&gt;"), ">");
        return metadata;
    }

    IMDFile::IMDFile(const std::string &path) : path(path) {
    }

    std::string IMDFile::readMetadata() const {
        std::ifstream file(path, std::ios_base::binary);
        if (!file) {
            throw IMDFileIOException("Could not open file " + path);
        }
        std::streamoff xmlStartPos, xmlEndPos;
        return readMetadataInternal(file, &xmlStartPos, &xmlEndPos);
    }

    const IMDData IMDFile::readData() const {
        // open file
        std::ifstream file(path, std::ios_base::binary);
        if (!file) {
            throw IMDFileIOException("Could not open file " + path);
        }
        // parse XML
        pugi::xml_document doc;
        std::streamoff xmlStartPos, xmlEndPos;
        std::string metadata = readMetadataInternal(file, &xmlStartPos, &xmlEndPos);
        pugi::xml_parse_result parse_result = doc.load_string(metadata.c_str());
        if (!parse_result) {
            std::string error_message(parse_result.description());
            throw IMDFileMalformedException("Failed to parse experiment schema xml: " + error_message);
        }
        // collect marker names and masses
        std::vector<std::string> markerNames;
        std::vector<std::double_t> markerMasses;
        for (const pugi::xpath_node &node : doc.select_nodes(ACQUISITION_MARKERS_XPATH)) {
            markerNames.emplace_back(node.node().child(ACQUISITION_MARKERS_SHORT_NAME_ELEMENT_NAME).text().as_string());
            markerMasses.emplace_back(node.node().child(ACQUISITION_MARKERS_MASS_ELEMENT_NAME).text().as_double());
        }
        // collect calibration masses, slopes and intercepts
        std::vector<std::array<std::double_t, 3>> calibrationData;
        for (const pugi::xpath_node &node : doc.select_nodes(DUAL_ANALYTES_SNAPSHOT_XPATH)) {
            calibrationData.emplace_back(std::array<std::double_t, 3>{
                    node.node().child(DUAL_ANALYTES_SNAPSHOT_MASS_ELEMENT_NAME).text().as_double(),
                    node.node().child(DUAL_ANALYTES_SNAPSHOT_DUAL_SLOPE_ELEMENT_NAME).text().as_double(),
                    node.node().child(DUAL_ANALYTES_SNAPSHOT_DUAL_INTERCEPT_ELEMENT_NAME).text().as_double()
            });
        }
        std::sort(calibrationData.begin(), calibrationData.end(),
                  [](const std::array<std::double_t, 3> &lhs, const std::array<std::double_t, 3> &rhs) {
                      return lhs[0] < rhs[0];
                  }
        );
        // compute marker slopes and intercepts
        std::vector<std::double_t> markerSlopes(markerNames.size());
        std::vector<std::double_t> markerIntercepts(markerNames.size());
        for (std::size_t markerIndex = 0; markerIndex < markerNames.size(); ++markerIndex) {
            if (markerMasses[markerIndex] <= calibrationData.front()[0]) {
                markerSlopes[markerIndex] = calibrationData.front()[1];
                markerIntercepts[markerIndex] = calibrationData.front()[2];
            } else if (markerMasses[markerIndex] >= calibrationData.back()[0]) {
                markerSlopes[markerIndex] = calibrationData.back()[1];
                markerIntercepts[markerIndex] = calibrationData.back()[2];
            }
        }
        for (std::size_t i = 0; i < calibrationData.size() - 1; ++i) {
            const auto lowerCalibrationMass = calibrationData[i][0];
            const auto upperCalibrationMass = calibrationData[i + 1][0];
            const auto calibrationMassDelta = upperCalibrationMass - lowerCalibrationMass;
            const auto slopeStep = (calibrationData[i + 1][1] - calibrationData[i][1]) / calibrationMassDelta;
            const auto interceptStep = (calibrationData[i + 1][2] - calibrationData[i][2]) / calibrationMassDelta;
            for (std::size_t markerIndex = 0; markerIndex < markerNames.size(); ++markerIndex) {
                const auto markerMass = markerMasses[markerIndex];
                if (markerMass >= lowerCalibrationMass && markerMass <= upperCalibrationMass) {
                    const auto markerCalibrationMassDelta = markerMass - lowerCalibrationMass;
                    markerSlopes[markerIndex] = calibrationData[i][1] + markerCalibrationMassDelta * slopeStep;
                    markerIntercepts[markerIndex] = calibrationData[i][2] + markerCalibrationMassDelta * interceptStep;
                }
            }
        }
        // read data
        IMDData data(markerNames, markerSlopes, markerIntercepts);
        file.seekg(0, std::ios_base::beg);
        std::vector<std::uint16_t> buffer(2 * markerNames.size());
        const auto bufferSize = buffer.size() * sizeof(std::uint16_t);
        for (std::size_t pushIndex = 0; pushIndex < xmlStartPos / bufferSize; ++pushIndex) {
            file.read((char *) buffer.data(), bufferSize);
            data.pushOffsets.push_back(data.markerIndices.size());
            for (std::size_t markerIndex = 0; markerIndex < markerNames.size(); ++markerIndex) {
                const std::uint16_t intensityValue = buffer[2 * markerIndex];
                const std::uint16_t pulseValue = buffer[2 * markerIndex + 1];
                if (intensityValue > 0) {
                    data.markerIndices.push_back(markerIndex);
                    data.intensityValues.push_back(intensityValue);
                    data.pulseValues.push_back(pulseValue);
                }
            }
        }
        data.pushOffsets.push_back(data.markerIndices.size());
        return data;
    }

}
