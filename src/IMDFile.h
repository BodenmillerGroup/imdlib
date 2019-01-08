#ifndef IMD_IMDFILE_H
#define IMD_IMDFILE_H


#include <fstream>
#include <pugixml.hpp>
#include <string>
#include <vector>
#include "IMDFileData.h"
#include "IMDFileIOException.h"
#include "IMDFileMalformedException.h"

#define SEARCH_BUFFER_SIZE 8192
#define EXPERIMENT_SCHEMA_START "<ExperimentSchema"
#define EXPERIMENT_SCHEMA_END "</ExperimentSchema>"
#define MARKER_SHORTNAME_XPATH "/ExperimentSchema/AcquisitionMarkers/ShortName"

namespace imd {

    class IMDFile {
    private:
        const std::string path;

        static std::vector<char> toUTF16(const std::string &s);

        static std::string toUTF8(const std::vector<char> &vec);

        template<class T>
        static bool
        searchVectorBackwards(const std::vector<T> &vec, const std::vector<T> &pattern, std::size_t *foundPos);

        static bool searchFileBackwards(std::ifstream &file, const std::string &pattern, std::streamoff *foundPos,
                                        std::streamoff startPos = 0);

        static std::string readText(std::ifstream &file, const std::streamoff &startPos, const std::streamoff &endPos);

    public:
        explicit IMDFile(const std::string &path);

        const IMDFileData readData() const;

        std::string readMetadata() const;
    };

}


#endif //IMD_IMDFILE_H
