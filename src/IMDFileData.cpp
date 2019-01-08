#include "IMDFileData.h"

namespace imd {

    IMDFileData::IMDFileData(const std::vector<std::string> &markerNames)
            : markerNames(markerNames) {
    }

    const IMDFileData::CSRAccessor IMDFileData::getPulses() const {
        return {*this, pulseValues};
    }

    const IMDFileData::CSRAccessor IMDFileData::getIntensities() const {
        return {*this, intensityValues};
    }

    std::size_t IMDFileData::getNumPushes() const {
        return pushOffsets.back();
    }

    std::size_t IMDFileData::getNumMarkers() const {
        return markerNames.size();
    }

    IMDFileData::CSRAccessor::CSRAccessor(const IMDFileData &data, const std::vector<std::uint16_t> &values)
            : data(data), values(values) {
    }

    std::uint16_t IMDFileData::CSRAccessor::operator()(std::size_t pushIndex, std::size_t markerIndex) const {
        auto markerIndicesFirst = data.markerIndices.begin() + data.pushOffsets[pushIndex];
        const auto markerIndicesLast = data.markerIndices.begin() + data.pushOffsets[pushIndex + 1];
        const auto markerIndicesIter = std::lower_bound(markerIndicesFirst, markerIndicesLast, markerIndex);
        if (markerIndicesIter != markerIndicesLast && *markerIndicesIter == markerIndex) {
            return values[markerIndicesIter - data.markerIndices.begin()];
        }
        return 0;
    }

    std::uint16_t IMDFileData::CSRAccessor::operator()(std::size_t pushIndex, std::string markerName) const {
        const auto markerNamesIter = std::find(data.markerNames.begin(), data.markerNames.end(), markerName);
        if (markerNamesIter == data.markerNames.end()) {
            throw std::out_of_range("Marker not found: " + markerName);
        }
        std::size_t markerIndex = static_cast<size_t>(std::distance(data.markerNames.begin(), markerNamesIter));
        return (*this)(pushIndex, markerIndex);
    }

}
