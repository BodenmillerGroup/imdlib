#include <algorithm>
#include <pybind11/stl.h>
#include <pybind11/pybind11.h>

#include <IMDFile.h>

namespace py = pybind11;

namespace imd {

    namespace py {

        template<class TCSRAccessor, typename TValue>
        const std::vector<TValue> getByMarkerName(const TCSRAccessor &accessor, const std::string &markerName) {
            return accessor[markerName];
        }

        template<class TCSRAccessor, typename TValue>
        TValue getByPushIndexAndMarkerName(const TCSRAccessor &accessor, const std::tuple<std::size_t, const std::string &> &pushIndexAndMarkerName) {
            return accessor(std::get<0>(pushIndexAndMarkerName), std::get<1>(pushIndexAndMarkerName));
        }

        template<class TCSRAccessor, typename TValue>
        TValue getByPushIndexAndMarkerIndex(const TCSRAccessor &accessor, const std::tuple<std::size_t, std::size_t> &pushIndexAndMarkerIndex) {
            return accessor(std::get<0>(pushIndexAndMarkerIndex), std::get<1>(pushIndexAndMarkerIndex));
        }

//        alternative implementation to expose CSRAccessor via pybind11

//        template<typename TValue, template<typename T> class TCSRAccessor = IMDData::CSRAccessor>
//        std::vector<TValue> getByMarkerName(const TCSRAccessor<TValue> &accessor, const std::string &markerName) {
//            return accessor[markerName];
//        }
//
//        template<typename TValue, template<typename T> class TCSRAccessor = IMDData::CSRAccessor>
//        TValue getByPushIndexAndMarkerName(const TCSRAccessor<TValue> &accessor, const std::tuple<std::size_t, const std::string &> &pushIndexAndMarkerName) {
//            return accessor(std::get<0>(pushIndexAndMarkerName), std::get<1>(pushIndexAndMarkerName));
//        }
//
//        template<typename TValue, template<typename T> class TCSRAccessor = IMDData::CSRAccessor>
//        TValue getByPushIndexAndMarkerIndex(const TCSRAccessor<TValue> &accessor, const std::tuple<std::size_t, std::size_t> &pushIndexAndMarkerIndex) {
//            return accessor(std::get<0>(pushIndexAndMarkerIndex), std::get<1>(pushIndexAndMarkerIndex));
//        }

    }

}

// There are no vectors that need to be opaque
//PYBIND11_MAKE_OPAQUE(std::vector<std::string>);
//PYBIND11_MAKE_OPAQUE(std::vector<std::size_t>);
//PYBIND11_MAKE_OPAQUE(std::vector<std::uint16_t>);
//PYBIND11_MAKE_OPAQUE(std::vector<std::double_t>);

PYBIND11_MODULE(imdpy, m) {
    m.doc() = "IMD file parser";


    // Don't bind STL containers (in favor of pickle support)
    //py::bind_vector<std::vector<std::string>>(m, "StringVector");
    //py::bind_vector<std::vector<std::size_t>>(m, "SizeVector");
    //py::bind_vector<std::vector<std::uint16_t>>(m, "UInt16Vector");
    //py::bind_vector<std::vector<std::double_t>>(m, "DoubleVector");

    py::class_<imd::IMDFile>(m, "IMDFile")
        .def(py::init<const std::string &>(), py::arg("path"))
        .def("read_data", &imd::IMDFile::readData, "Read the full data set into memory")
        .def("read_metadata", &imd::IMDFile::readMetadata, "Read the raw metadata as text");

    py::class_<imd::IMDData>(m, "IMDData")
        .def_property_readonly("num_markers", &imd::IMDData::getNumMarkers, "Number of markers")
        .def_property_readonly("num_pushes", &imd::IMDData::getNumPushes, "Number of pushes")
        .def_property_readonly("marker_names", &imd::IMDData::getMarkerNames, "Marker names")
        .def_property_readonly("pulses", &imd::IMDData::getPulses, "Pulse values")
        .def_property_readonly("intensities", &imd::IMDData::getIntensities, "Intensity values")
        .def_property_readonly("dual_counts", (const imd::IMDData::CSRDualCountAccessor (imd::IMDData::*)() const) &imd::IMDData::getDualCounts, "Dual counts")
        .def("get_dual_counts", (const imd::IMDData::CSRDualCountAccessor (imd::IMDData::*)(std::double_t) const) &imd::IMDData::getDualCounts, py::arg("pulseThreshold"), "Dual counts with custom pulse threshold")
        .def("get_dual_counts", (const imd::IMDData::CSRDualCountAccessor (imd::IMDData::*)(std::double_t, const std::vector<double_t> &, const std::vector<double_t> &) const) &imd::IMDData::getDualCounts, py::arg("pulseThreshold"), py::arg("markerSlopes"), py::arg("markerIntensities"), "Dual counts with custom pulse threshold and calibration curve")
        .def(py::pickle(
                [](const imd::IMDData &imdData) {
                    return py::make_tuple(
                            imdData.markerNames, imdData.markerSlopes, imdData.markerIntercepts,
                            imdData.pushOffsets, imdData.markerIndices, imdData.pulseValues, imdData.intensityValues);
                },
                [](py::tuple t) {
                    const auto markerNames = t[0].cast<std::vector<std::string>>();
                    const auto markerSlopes = t[1].cast<std::vector<std::double_t>>();
                    const auto markerIntercepts = t[2].cast<std::vector<std::double_t>>();
                    imd::IMDData imdData(markerNames, markerSlopes, markerIntercepts);
                    imdData.pushOffsets = t[3].cast<std::vector<std::size_t>>();
                    imdData.markerIndices = t[4].cast<std::vector<std::size_t>>();
                    imdData.pulseValues = t[5].cast<std::vector<std::uint16_t>>();
                    imdData.intensityValues = t[6].cast<std::vector<std::uint16_t>>();
                    return imdData;
                }));

    py::class_<imd::IMDData::CSRValueAccessor>(m, "CSRValueAccessor")
        .def("__getitem__", imd::py::getByMarkerName<imd::IMDData::CSRValueAccessor, std::uint16_t>, py::arg("markerName"), "Value access by marker name")
        .def("get_by_push_index", &imd::IMDData::CSRValueAccessor::getByPushIndex, py::arg("pushIndex"), "Value access by push index")
        .def("get_by_marker_index", &imd::IMDData::CSRValueAccessor::getByMarkerIndex, py::arg("markerIndex"), "Value access by marker index")
        .def("__getitem__", imd::py::getByPushIndexAndMarkerName<imd::IMDData::CSRValueAccessor, std::uint16_t>, py::arg("pushIndexAndMarkerName"), "Value access by push index and marker name")
        .def("__getitem__", imd::py::getByPushIndexAndMarkerIndex<imd::IMDData::CSRValueAccessor, std::uint16_t>, py::arg("pushIndexAndMarkerIndex"), "Value access by push index and marker index")
        .def("to_dense", &imd::IMDData::CSRValueAccessor::toDense, "Converts the value matrix to a dense row-major representation");

    py::class_<imd::IMDData::CSRDualCountAccessor>(m, "CSRDualCountAccessor")
        .def("__getitem__", imd::py::getByMarkerName<imd::IMDData::CSRDualCountAccessor, std::double_t>, py::arg("markerName"), "Dual count access by marker name")
        .def("get_by_push_index", &imd::IMDData::CSRDualCountAccessor::getByPushIndex, py::arg("pushIndex"), "Dual count access by push index")
        .def("get_by_marker_index", &imd::IMDData::CSRDualCountAccessor::getByMarkerIndex, py::arg("markerIndex"), "Dual count access by marker index")
        .def("__getitem__", imd::py::getByPushIndexAndMarkerName<imd::IMDData::CSRDualCountAccessor, std::double_t>, py::arg("pushIndexAndMarkerName"), "Dual count access by push index and marker name")
        .def("__getitem__", imd::py::getByPushIndexAndMarkerIndex<imd::IMDData::CSRDualCountAccessor, std::double_t>, py::arg("pushIndexAndMarkerIndex"), "Dual count access by push index and marker index")
        .def("to_dense", &imd::IMDData::CSRDualCountAccessor::toDense, "Converts the dual count matrix to a dense row-major representation");

    py::register_exception<imd::IMDFileIOException>(m, "IMDFileIOException");
    py::register_exception<imd::IMDFileMalformedException>(m, "IMDFileMalformedException");

//    alternative implementation to expose CSRAccessor via pybind11

//    py::class_<imd::IMDData::CSRAccessor<std::uint16_t>>(m, "CSRAccessorUINT16")
//        .def("__getitem__", imd::py::getByMarkerName<std::uint16_t>, py::arg("markerName"), "Value access by marker name")
//        .def("get_by_push_index", &imd::IMDData::CSRAccessor<std::uint16_t>::getByPushIndex, py::arg("pushIndex"), "Value access by push index")
//        .def("get_by_marker_index", &imd::IMDData::CSRAccessor<std::uint16_t>::getByMarkerIndex, py::arg("markerIndex"), "Value access by marker index")
//        .def("__getitem__", imd::py::getByPushIndexAndMarkerName<std::uint16_t>, py::arg("pushIndexAndMarkerName"), "Value access by push index and marker name")
//        .def("__getitem__", imd::py::getByPushIndexAndMarkerIndex<std::uint16_t>, py::arg("pushIndexAndMarkerIndex"), "Value access by push index and marker index")
//        .def("to_dense", &imd::IMDData::CSRAccessor<std::uint16_t>::toDense, "Converts the value matrix to a dense row-major representation");
//
//    py::class_<imd::IMDData::CSRAccessor<std::double_t>>(m, "CSRAccessorDOUBLE")
//        .def("__getitem__", imd::py::getByMarkerName<std::double_t>, py::arg("markerName"), "Value access by marker name")
//        .def("get_by_push_index", &imd::IMDData::CSRAccessor<std::double_t>::getByPushIndex, py::arg("pushIndex"), "Value access by push index")
//        .def("get_by_marker_index", &imd::IMDData::CSRAccessor<std::double_t>::getByMarkerIndex, py::arg("markerIndex"), "Value access by marker index")
//        .def("__getitem__", imd::py::getByPushIndexAndMarkerName<std::double_t>, py::arg("pushIndexAndMarkerName"), "Value access by push index and marker name")
//        .def("__getitem__", imd::py::getByPushIndexAndMarkerIndex<std::double_t>, py::arg("pushIndexAndMarkerIndex"), "Value access by push index and marker index")
//        .def("to_dense", &imd::IMDData::CSRAccessor<std::double_t>::toDense, "Converts the value matrix to a dense row-major representation");
//
//    py::class_<imd::IMDData::CSRValueAccessor, imd::IMDData::CSRAccessor<std::uint16_t>>(m, "CSRValueAccessor");
//
//    py::class_<imd::IMDData::CSRDualCountAccessor, imd::IMDData::CSRAccessor<std::double_t>>(m, "CSRDualCountAccessor");

}