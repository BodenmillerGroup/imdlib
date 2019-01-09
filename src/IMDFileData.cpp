#include "IMDFileData.h"

namespace imd {

    IMDFileData::IMDFileData(const std::vector<std::string> &markerNames)
            : markerNames(markerNames), markerNameIndices(createMarkerNameIndices(markerNames)) {
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

    const std::vector<std::string> &IMDFileData::getMarkerNames() const {
        return markerNames;
    }

    std::map<std::string, std::size_t>
    IMDFileData::createMarkerNameIndices(const std::vector<std::string> &markerNames) {
        std::map<std::string, std::size_t> markerNameIndices;
        for (std::size_t i = 0; i < markerNames.size(); ++i) {
            markerNameIndices[markerNames[i]] = i;
        }
        return markerNameIndices;
    }

    IMDFileData::CSRAccessor::CSRAccessor(const IMDFileData &data, const std::vector<std::uint16_t> &values)
            : data(data), values(values) {
    }

    std::vector<std::uint16_t> IMDFileData::CSRAccessor::operator[](std::size_t pushIndex) const {
        if (pushIndex >= data.pushOffsets.size()) {
            throw std::out_of_range("Push index out of range: " + std::to_string(pushIndex));
        }
        return {values.begin() + data.pushOffsets[pushIndex], values.begin() + data.pushOffsets[pushIndex]};
    }

    std::vector<std::uint16_t> IMDFileData::CSRAccessor::operator[](const std::string &markerName) const {
        return getByMarkerIndex(data.markerNameIndices.at(markerName));
    }

    std::uint16_t IMDFileData::CSRAccessor::operator()(std::size_t pushIndex, std::string markerName) const {
        return getByMarkerIndex(pushIndex, data.markerNameIndices.at(markerName));
    }

    std::vector<std::uint16_t> IMDFileData::CSRAccessor::getByMarkerIndex(std::size_t markerIndex) const {
        if (markerIndex >= data.markerNames.size()) {
            throw std::out_of_range("Marker index out of range: " + std::to_string(markerIndex));
        }
        std::vector<std::uint16_t> result;
        result.reserve(data.getNumPushes());
        for (std::size_t pushIndex = 0; pushIndex < data.getNumPushes(); ++pushIndex) {
            result.push_back(getByMarkerIndex(pushIndex, markerIndex));
        }
        return result;
    }

    std::uint16_t IMDFileData::CSRAccessor::getByMarkerIndex(std::size_t pushIndex, std::size_t markerIndex) const {
        if (pushIndex >= data.pushOffsets.size()) {
            throw std::out_of_range("Push index out of range: " + std::to_string(pushIndex));
        }
        if (markerIndex >= data.markerNames.size()) {
            throw std::out_of_range("Marker index out of range: " + std::to_string(markerIndex));
        }
        auto first = data.markerIndices.begin() + data.pushOffsets[pushIndex];
        const auto last = data.markerIndices.begin() + data.pushOffsets[pushIndex + 1];
        const auto iter = std::lower_bound(first, last, markerIndex);
        if (iter != last && *iter == markerIndex) {
            return values[iter - data.markerIndices.begin()];
        }
        return 0;
    }

    std::vector<std::uint16_t> IMDFileData::CSRAccessor::toDense() const {
        std::vector<uint16_t> matrix(data.getNumPushes() * data.getNumMarkers(), 0);
        for (std::size_t pushIndex = 0; pushIndex < data.getNumPushes(); ++pushIndex) {
            for (std::size_t i = data.pushOffsets[pushIndex]; i < data.pushOffsets[pushIndex + 1]; ++i) {
                std::size_t markerIndex = data.markerIndices[i];
                matrix[pushIndex * data.getNumMarkers() + markerIndex] = values[i];
            }
        }
        return matrix;
    }

}
