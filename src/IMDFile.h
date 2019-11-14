#ifndef IMD_IMDFILE_H
#define IMD_IMDFILE_H


#include <fstream>
#include <pugixml.hpp>
#include <regex>
#include <string>
#include <vector>

#include "IMDData.h"
#include "IMDFileIOException.h"
#include "IMDFileMalformedException.h"

#define SEARCH_BUFFER_SIZE 8192
#define EXPERIMENT_SCHEMA_START "<ExperimentSchema"
#define EXPERIMENT_SCHEMA_END "</ExperimentSchema>"

#define ACQUISITION_MARKERS_XPATH "/ExperimentSchema/AcquisitionMarkers"
#define ACQUISITION_MARKERS_SHORT_NAME_ELEMENT_NAME "ShortName"
#define ACQUISITION_MARKERS_MASS_ELEMENT_NAME "Mass"

#define DUAL_ANALYTES_SNAPSHOT_XPATH "/ExperimentSchema/DualAnalytesSnapshot"
#define DUAL_ANALYTES_SNAPSHOT_MASS_ELEMENT_NAME "Mass"
#define DUAL_ANALYTES_SNAPSHOT_DUAL_SLOPE_ELEMENT_NAME "DualSlope"
#define DUAL_ANALYTES_SNAPSHOT_DUAL_INTERCEPT_ELEMENT_NAME "DualIntercept"

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

        static std::string
        readMetadataInternal(std::ifstream &file, std::streamoff *xmlStartPos, std::streamoff *xmlEndPos);

    public:
        explicit IMDFile(const std::string &path);

        std::string readMetadata() const;

        const IMDData readData() const;
    };

}


#endif //IMD_IMDFILE_H
