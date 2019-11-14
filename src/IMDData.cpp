#include "IMDData.h"

namespace imd {

    IMDData::IMDData(const std::vector<std::string> &markerNames,
                     const std::vector<std::double_t> &markerSlopes,
                     const std::vector<std::double_t> &markerIntercepts)
            : markerNames(markerNames), markerSlopes(markerSlopes), markerIntercepts(markerIntercepts),
              markerNameIndices(createMarkerNameIndices(markerNames)) {
    }

    const std::map<std::string, std::size_t>
    IMDData::createMarkerNameIndices(const std::vector<std::string> &markerNames) {
        std::map<std::string, std::size_t> markerNameIndices;
        for (std::size_t i = 0; i < markerNames.size(); ++i) {
            markerNameIndices[markerNames[i]] = i;
        }
        return markerNameIndices;
    }

    std::size_t IMDData::getNumPushes() const {
        return pushOffsets.size() - 1;
    }

    std::size_t IMDData::getNumMarkers() const {
        return markerNames.size();
    }

    const std::vector<std::string> &IMDData::getMarkerNames() const {
        return markerNames;
    }

    const IMDData::CSRValueAccessor IMDData::getPulses() const {
        return CSRValueAccessor(*this, pulseValues);
    }

    const IMDData::CSRValueAccessor IMDData::getIntensities() const {
        return CSRValueAccessor(*this, intensityValues);
    }

    const IMDData::CSRDualCountAccessor IMDData::getDualCounts() const {
        return getDualCounts(DEFAULT_PULSE_THRESHOLD);
    }

    const IMDData::CSRDualCountAccessor IMDData::getDualCounts(std::double_t pulseThreshold) const {
        return getDualCounts(pulseThreshold, markerSlopes, markerIntercepts);
    }

    const IMDData::CSRDualCountAccessor
    IMDData::getDualCounts(std::double_t pulseThreshold, const std::vector<std::double_t> &markerSlopes,
                           const std::vector<std::double_t> &markerIntercepts) const {
        return CSRDualCountAccessor(*this, pulseThreshold, markerSlopes, markerIntercepts);
    }

    template<typename T>
    IMDData::CSRAccessor<T>::CSRAccessor(const IMDData &data) : data(data) {
    }

    template<typename T>
    std::vector<T> IMDData::CSRAccessor<T>::operator[](const std::string &markerName) const {
        return getByMarkerIndex(data.markerNameIndices.at(markerName));
    }

    template<typename T>
    std::vector<T> IMDData::CSRAccessor<T>::getByPushIndex(std::size_t pushIndex) const {
        if (pushIndex >= data.pushOffsets.size()) {
            throw std::out_of_range("Push index out of range: " + std::to_string(pushIndex));
        }
        std::vector<T> result(data.getNumMarkers(), 0);
        for (std::size_t i = data.pushOffsets[pushIndex]; i < data.pushOffsets[pushIndex + 1]; ++i) {
            result[data.markerIndices[i]] = getValue(i);
        }
        return result;
    }

    template<typename T>
    std::vector<T> IMDData::CSRAccessor<T>::getByMarkerIndex(std::size_t markerIndex) const {
        if (markerIndex >= data.markerNames.size()) {
            throw std::out_of_range("Marker index out of range: " + std::to_string(markerIndex));
        }
        std::vector<T> result(data.getNumPushes());
        for (std::size_t pushIndex = 0; pushIndex < data.getNumPushes(); ++pushIndex) {
            result[pushIndex] = (*this)(pushIndex, markerIndex);
        }
        return result;
    }

    template<typename T>
    T IMDData::CSRAccessor<T>::operator()(std::size_t pushIndex, std::size_t markerIndex) const {
        if (pushIndex >= data.pushOffsets.size()) {
            throw std::out_of_range("Push index out of range: " + std::to_string(pushIndex));
        }
        if (markerIndex >= data.markerNames.size()) {
            throw std::out_of_range("Marker index out of range: " + std::to_string(markerIndex));
        }
        auto itFirst = data.markerIndices.begin() + data.pushOffsets[pushIndex];
        const auto itLast = data.markerIndices.begin() + data.pushOffsets[pushIndex + 1];
        const auto it = std::lower_bound(itFirst, itLast, markerIndex);
        if (it != itLast && *it == markerIndex) {
            return getValue((std::size_t) std::distance(data.markerIndices.begin(), it));
        }
        return T();
    }

    template<typename T>
    T IMDData::CSRAccessor<T>::operator()(std::size_t pushIndex, const std::string &markerName) const {
        return (*this)(pushIndex, data.markerNameIndices.at(markerName));
    }

    template<typename T>
    std::vector<T> IMDData::CSRAccessor<T>::toDense() const {
        std::vector<T> matrix(data.getNumPushes() * data.getNumMarkers(), 0);
        for (std::size_t pushIndex = 0; pushIndex < data.getNumPushes(); ++pushIndex) {
            for (std::size_t i = data.pushOffsets[pushIndex]; i < data.pushOffsets[pushIndex + 1]; ++i) {
                std::size_t markerIndex = data.markerIndices[i];
                matrix[pushIndex * data.getNumMarkers() + markerIndex] = getValue(i);
            }
        }
        return matrix;
    }

    IMDData::CSRValueAccessor::CSRValueAccessor(const IMDData &data, const std::vector<std::uint16_t> &values)
            : CSRAccessor(data), values(values) {
    }

    std::uint16_t IMDData::CSRValueAccessor::getValue(std::size_t index) const {
        return values[index];
    }

    IMDData::CSRDualCountAccessor::CSRDualCountAccessor(const IMDData &data, std::double_t pulseThreshold,
                                                        const std::vector<std::double_t> &markerSlopes,
                                                        const std::vector<std::double_t> &markerIntercepts)
            : CSRAccessor(data), pulseThreshold(pulseThreshold), markerSlopes(markerSlopes),
              markerIntercepts(markerIntercepts) {
    }

    std::double_t IMDData::CSRDualCountAccessor::getValue(std::size_t index) const {
        const auto pulseValue = data.pulseValues[index];
        const auto markerIndex = data.markerIndices[index];
        const auto dualCount = markerIntercepts[markerIndex] + data.intensityValues[index] * markerSlopes[markerIndex];
        if (dualCount < pulseValue && pulseValue < pulseThreshold) {
            return pulseValue;
        }
        return dualCount;
    }

    // template class realizations (for pybind11)
    template
    class IMDData::CSRAccessor<std::uint16_t>;

    template
    class IMDData::CSRAccessor<std::double_t>;

}
