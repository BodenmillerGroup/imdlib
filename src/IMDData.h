#ifndef IMD_IMDDATA_H
#define IMD_IMDDATA_H


#include <algorithm>
#include <map>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#define DEFAULT_PULSE_THRESHOLD 3.

namespace imd {

    struct IMDData {
    public:

        template<typename T>
        class CSRAccessor {
        protected:
            const IMDData &data;

            virtual T getValue(std::size_t index) const = 0;

        public:
            explicit CSRAccessor(const IMDData &data);

            std::vector<T> operator[](const std::string &markerName) const;

            std::vector<T> getByPushIndex(std::size_t pushIndex) const;

            std::vector<T> getByMarkerIndex(std::size_t markerIndex) const;

            T operator()(std::size_t pushIndex, std::size_t markerIndex) const;

            T operator()(std::size_t pushIndex, const std::string &markerName) const;

            std::vector<T> toDense() const;

        };

        class CSRValueAccessor : public CSRAccessor<std::uint16_t> {
        private:
            const std::vector<std::uint16_t> &values;

            std::uint16_t getValue(std::size_t index) const override;

        public:
            CSRValueAccessor(const IMDData &data, const std::vector<std::uint16_t> &values);

        };

        class CSRDualCountAccessor : public CSRAccessor<std::double_t> {
        private:
            const std::double_t pulseThreshold;
            const std::vector<std::double_t> markerSlopes;
            const std::vector<std::double_t> markerIntercepts;

            std::double_t getValue(std::size_t index) const override;

        public:
            CSRDualCountAccessor(const IMDData &data, std::double_t pulseThreshold,
                                 const std::vector<std::double_t> &markerSlopes,
                                 const std::vector<std::double_t> &markerIntercepts);

        };

    private:
        static const std::map<std::string, std::size_t>
        createMarkerNameIndices(const std::vector<std::string> &markerNames);

    public:
        const std::vector<std::string> markerNames;
        const std::vector<std::double_t> markerSlopes;
        const std::vector<std::double_t> markerIntercepts;
        const std::map<std::string, std::size_t> markerNameIndices;

        std::vector<std::size_t> pushOffsets;
        std::vector<std::size_t> markerIndices;
        std::vector<std::uint16_t> pulseValues;
        std::vector<std::uint16_t> intensityValues;

        IMDData(const std::vector<std::string> &markerNames, const std::vector<std::double_t> &markerSlopes,
                const std::vector<std::double_t> &markerIntercepts);

        std::size_t getNumPushes() const;

        std::size_t getNumMarkers() const;

        const std::vector<std::string> &getMarkerNames() const;

        const CSRValueAccessor getPulses() const;

        const CSRValueAccessor getIntensities() const;

        const CSRDualCountAccessor getDualCounts() const;

        const CSRDualCountAccessor getDualCounts(std::double_t pulseThreshold) const;

        const CSRDualCountAccessor
        getDualCounts(std::double_t pulseThreshold, const std::vector<std::double_t> &markerSlopes,
                      const std::vector<std::double_t> &markerIntercepts) const;

    };

}


#endif //IMD_IMDDATA_H
