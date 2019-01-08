#include <algorithm>
#include <pybind11/stl.h>
#include <pybind11/pybind11.h>

#include "IMDFile.h"

namespace py = pybind11;

namespace imd {

    namespace py {

        std::vector<uint16_t> getValuesByPushIndex(const IMDFileData::CSRAccessor &accessor, std::size_t pushIndex) {
            return accessor[pushIndex];
        }

        std::vector<uint16_t>
        getValuesByMarkerName(const IMDFileData::CSRAccessor &accessor, const std::string &markerName) {
            return accessor[markerName];
        }

        std::vector<uint16_t>
        getValuesByMarkerIndex(const IMDFileData::CSRAccessor &accessor, std::size_t markerIndex) {
            return accessor.getByMarkerIndex(markerIndex);
        }

        uint16_t getValueByPushIndexAndMarkerName(const IMDFileData::CSRAccessor &accessor,
                                                  const std::tuple<std::size_t, const std::string &> &index) {
            std::size_t pushIndex = std::get<0>(index);
            const std::string &markerName = std::get<1>(index);
            return accessor(pushIndex, markerName);
        }

        uint16_t getValueByPushIndexAndMarkerIndex(const IMDFileData::CSRAccessor &accessor,
                                                   const std::tuple<std::size_t, std::size_t> &index) {
            std::size_t pushIndex = std::get<0>(index);
            std::size_t markerIndex = std::get<1>(index);
            return accessor.getByMarkerIndex(pushIndex, markerIndex);
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
            .def("__getitem__", imd::py::getValuesByPushIndex, py::arg("pushIndex"), "Value access by push index")
            .def("__getitem__", imd::py::getValuesByMarkerName, py::arg("markerName"), "Value access by marker name")
            .def("getByMarkerIndex", imd::py::getValuesByMarkerIndex, py::arg("markerIndex"), "Value access by marker index")
            .def("__getitem__", imd::py::getValueByPushIndexAndMarkerName, py::arg("pushIndex"), py::arg("markerName"), "Value access by push index and marker name")
            .def("getByMarkerIndex", imd::py::getValueByPushIndexAndMarkerIndex, py::arg("pushIndex"), py::arg("markerIndex"), "Value access by push index and marker index");
}