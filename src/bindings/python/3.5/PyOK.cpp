#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <okFrontPanelDLL.h>

class BD_OK: public okCFrontPanel{
public:
    long WriteToBlockPipeIn(int endpoint, int block_size, pybind11::buffer b){
            pybind11::buffer_info info = b.request();
            if (info.format != pybind11::format_descriptor<unsigned char>::format())
                throw std::runtime_error("Incompatible format: expected a byte array!");
            unsigned char* payload = static_cast<unsigned char*>(info.ptr);
            int length = info.shape[0];
            return okCFrontPanel::WriteToBlockPipeIn(endpoint, block_size, length, payload);
    }

    long ReadFromBlockPipeOut(int endpoint, int block_size, pybind11::buffer b) {
            pybind11::buffer_info info = b.request();
            if (info.format != pybind11::format_descriptor<unsigned char>::format())
                throw std::runtime_error("Incompatible format: expected a byte array!");
            unsigned char* buffer = static_cast<unsigned char*>(info.ptr);
            int length = info.shape[0];
            return okCFrontPanel::ReadFromBlockPipeOut(endpoint, block_size, length, buffer);
    }

    long ReadFromPipeOut(int endpoint, pybind11::buffer b) {
            pybind11::buffer_info info = b.request();
            if (info.format != pybind11::format_descriptor<unsigned char>::format())
                throw std::runtime_error("Incompatible format: expected a byte array!");
            unsigned char* buffer = static_cast<unsigned char*>(info.ptr);
            int length = info.shape[0];
            return okCFrontPanel::ReadFromPipeOut(endpoint, length, buffer);
    }
};

PYBIND11_MODULE(PyOK, m)
{
    m.def("okFrontPanelDLL_LoadLib", okFrontPanelDLL_LoadLib);
    m.def("okFrontPanelDLL_GetVersion", []() {
        char lib_date[32], lib_time[32];
        okFrontPanelDLL_GetVersion(lib_date, lib_time);
        return std::tuple<std::string, std::string>(lib_date, lib_time);
    });

    pybind11::enum_<BD_OK::ErrorCode>(m, "ErrorCode", "")
        .value("NoError", BD_OK::ErrorCode::NoError)
        .value("Failed", BD_OK::ErrorCode::Failed)
        .value("Timeout", BD_OK::ErrorCode::Timeout)
        .value("DoneNotHigh", BD_OK::ErrorCode::DoneNotHigh)
        .value("TransferError", BD_OK::ErrorCode::TransferError)
        .value("CommunicationError", BD_OK::ErrorCode::CommunicationError)
        .value("InvalidBitstream", BD_OK::ErrorCode::InvalidBitstream)
        .value("FileError", BD_OK::ErrorCode::FileError)
        .value("DeviceNotOpen", BD_OK::ErrorCode::DeviceNotOpen)
        .value("InvalidEndpoint", BD_OK::ErrorCode::InvalidEndpoint)
        .value("InvalidBlockSize", BD_OK::ErrorCode::InvalidBlockSize)
        .value("I2CRestrictedAddress", BD_OK::ErrorCode::I2CRestrictedAddress)
        .value("I2CBitError", BD_OK::ErrorCode::I2CBitError)
        .value("I2CNack", BD_OK::ErrorCode::I2CNack)
        .value("I2CUnknownStatus", BD_OK::ErrorCode::I2CUnknownStatus)
        .value("UnsupportedFeature", BD_OK::ErrorCode::UnsupportedFeature)
        .value("FIFOUnderflow", BD_OK::ErrorCode::FIFOUnderflow)
        .value("FIFOOverflow", BD_OK::ErrorCode::FIFOOverflow)
        .value("DataAlignmentError", BD_OK::ErrorCode::DataAlignmentError)
        .value("InvalidResetProfile", BD_OK::ErrorCode::InvalidResetProfile)
        .value("InvalidParameter", BD_OK::ErrorCode::InvalidParameter)
        .export_values();

    pybind11::class_<BD_OK>(m, "okCFrontPanel")
        .def(pybind11::init<>())
        .def("WriteToBlockPipeIn", (long (BD_OK::*) (int, int, pybind11::buffer b)) &BD_OK::WriteToBlockPipeIn)
        .def("ReadFromBlockPipeOut", (long (BD_OK::*) (int, int, pybind11::buffer b)) &BD_OK::ReadFromBlockPipeOut)
        .def("ReadFromPipeOut", (long (BD_OK::*) (int, int, pybind11::buffer b)) &BD_OK::ReadFromPipeOut)
        .def("OpenBySerial", &BD_OK::OpenBySerial)
        .def("GetDeviceInfo", &BD_OK::GetDeviceInfo)
        .def("LoadDefaultPLLConfiguration", &BD_OK::LoadDefaultPLLConfiguration)
        .def("ConfigureFPGA", &BD_OK::ConfigureFPGA)
        .def("IsFrontPanelEnabled", &BD_OK::IsFrontPanelEnabled)
        ;

    pybind11::class_<okTDeviceInfo >(m, "_okTDeviceInfo")
        .def_readonly("productName", &okTDeviceInfo::productName)
        .def_readonly("deviceMajorVersion", &okTDeviceInfo::deviceMajorVersion)
        .def_readonly("deviceMinorVersion", &okTDeviceInfo::deviceMinorVersion)
        .def_readonly("serialNumber", &okTDeviceInfo::serialNumber)
        .def_readonly("productID", &okTDeviceInfo::productID)
        ;

    m.def("okTDeviceInfo", []() { return new okTDeviceInfo; }, pybind11::return_value_policy::take_ownership);
}
