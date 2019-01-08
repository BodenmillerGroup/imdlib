#include <algorithm>
#include <pybind11/stl.h>
#include <pybind11/pybind11.h>

#include "IMDFile.h"

namespace py = pybind11;

namespace imd {

    namespace py {

        uint16_t getValueByMarkerName(const IMDFileData::CSRAccessor &accessor,
                                      const std::tuple<std::size_t, const std::string &> &index) {
            std::size_t pushIndex = std::get<0>(index);
            const std::string &markerName = std::get<1>(index);
            return accessor(pushIndex, markerName);
        }

        uint16_t getValueByMarkerIndex(const IMDFileData::CSRAccessor &accessor,
                                       const std::tuple<std::size_t, std::size_t> &index) {
            std::size_t pushIndex = std::get<0>(index);
            std::size_t markerIndex = std::get<1>(index);
            return accessor(pushIndex, markerIndex);
        }

    }

}

PYBIND11_MODULE(imdpy, m) {
    m.doc() = "IMD file parser";

    py::class_<imd::IMDFile>(m, "IMDFile")
            .def(py::init<const std::string &>(), py::arg("path"))
            .def("read_data", &imd::IMDFile::readData, "Read the full data set into memory")
            .def("read_metadata", &imd::IMDFile::readMetadata, "Read the metadata into memory");

    py::class_<imd::IMDFileData>(m, "IMDFileData")
            .def_readonly("markerNames", &imd::IMDFileData::markerNames, "Marker names")
            .def_property_readonly("pulses", &imd::IMDFileData::getPulses, "Pulse values")
            .def_property_readonly("intensities", &imd::IMDFileData::getIntensities, "Intensity values")
            .def_property_readonly("num_markers", &imd::IMDFileData::getNumMarkers, "Number of markers")
            .def_property_readonly("num_pushes", &imd::IMDFileData::getNumPushes, "Number of pushes");

    py::class_<imd::IMDFileData::CSRAccessor>(m, "IMDFileDataCSRAccessor")
            .def("__getitem__", imd::py::getValueByMarkerName, "Value access by push index and marker name")
            .def("__getitem__", imd::py::getValueByMarkerIndex, "Value access by push index and marker index");
}