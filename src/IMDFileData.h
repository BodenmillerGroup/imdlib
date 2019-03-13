#ifndef IMD_IMDDATA_H
#define IMD_IMDDATA_H


#include <algorithm>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace imd {

    struct IMDFileData {
    private:
        const std::vector<std::string> markerNames;
        const std::map<std::string, std::size_t> markerNameIndices;

        static std::map<std::string, std::size_t> createMarkerNameIndices(const std::vector<std::string> &markerNames);

    public:
        class CSRAccessor {
        private:
            const IMDFileData &data;
            const std::vector<std::uint16_t> &values;

        public:
            CSRAccessor(const IMDFileData &data, const std::vector<std::uint16_t> &values);

            std::vector<std::uint16_t> operator[](std::size_t pushIndex) const;

            std::vector<std::uint16_t> operator[](const std::string &markerName) const;

            std::uint16_t operator()(std::size_t pushIndex, std::string markerName) const;

            std::vector<std::uint16_t> getByMarkerIndex(std::size_t markerIndex) const;

            std::uint16_t getByMarkerIndex(std::size_t pushIndex, std::size_t markerIndex) const;

            std::vector<std::uint16_t> toDense() const;

        };

        std::vector<std::size_t> pushOffsets;
        std::vector<std::size_t> markerIndices;
        std::vector<std::uint16_t> pulseValues;
        std::vector<std::uint16_t> intensityValues;

        explicit IMDFileData(const std::vector<std::string> &markerNames);

        const std::vector<std::string> &getMarkerNames() const;

        const CSRAccessor getPulses() const;

        const CSRAccessor getIntensities() const;

        std::size_t getNumPushes() const;

        std::size_t getNumMarkers() const;
    };

}


#endif //IMD_IMDDATA_H
