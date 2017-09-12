#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <okFrontPanelDLL.h>

PYBIND11_MODULE(OpalKelly, m)
{
    m.def("okFrontPanelDLL_LoadLib", okFrontPanelDLL_LoadLib);
    m.def("okFrontPanelDLL_GetVersion", []() {
        char lib_date[32], lib_time[32];
        okFrontPanelDLL_GetVersion(lib_date, lib_time);
        return std::tuple<std::string, std::string>(lib_date, lib_time);
    });

    pybind11::enum_<okCFrontPanel::ErrorCode>(m, "ErrorCode", "")
        .value("NoError", okCFrontPanel::ErrorCode::NoError)
        .value("Failed", okCFrontPanel::ErrorCode::Failed)
        .value("Timeout", okCFrontPanel::ErrorCode::Timeout)
        .value("DoneNotHigh", okCFrontPanel::ErrorCode::DoneNotHigh)
        .value("TransferError", okCFrontPanel::ErrorCode::TransferError)
        .value("CommunicationError", okCFrontPanel::ErrorCode::CommunicationError)
        .value("InvalidBitstream", okCFrontPanel::ErrorCode::InvalidBitstream)
        .value("FileError", okCFrontPanel::ErrorCode::FileError)
        .value("DeviceNotOpen", okCFrontPanel::ErrorCode::DeviceNotOpen)
        .value("InvalidEndpoint", okCFrontPanel::ErrorCode::InvalidEndpoint)
        .value("InvalidBlockSize", okCFrontPanel::ErrorCode::InvalidBlockSize)
        .value("I2CRestrictedAddress", okCFrontPanel::ErrorCode::I2CRestrictedAddress)
        .value("I2CBitError", okCFrontPanel::ErrorCode::I2CBitError)
        .value("I2CNack", okCFrontPanel::ErrorCode::I2CNack)
        .value("I2CUnknownStatus", okCFrontPanel::ErrorCode::I2CUnknownStatus)
        .value("UnsupportedFeature", okCFrontPanel::ErrorCode::UnsupportedFeature)
        .value("FIFOUnderflow", okCFrontPanel::ErrorCode::FIFOUnderflow)
        .value("FIFOOverflow", okCFrontPanel::ErrorCode::FIFOOverflow)
        .value("DataAlignmentError", okCFrontPanel::ErrorCode::DataAlignmentError)
        .value("InvalidResetProfile", okCFrontPanel::ErrorCode::InvalidResetProfile)
        .value("InvalidParameter", okCFrontPanel::ErrorCode::InvalidParameter)
        .export_values();

    pybind11::class_<okCFrontPanel>(m, "okCFrontPanel")
        .def(pybind11::init<>())
        .def("WriteToBlockPipeIn", &okCFrontPanel::WriteToBlockPipeIn)
        .def("ReadFromBlockPipeOut", &okCFrontPanel::ReadFromBlockPipeOut)
        .def("OpenBySerial", &okCFrontPanel::OpenBySerial)
        .def("GetDeviceInfo", &okCFrontPanel::GetDeviceInfo)
        .def("LoadDefaultPLLConfiguration", &okCFrontPanel::LoadDefaultPLLConfiguration)
        .def("ConfigureFPGA", &okCFrontPanel::ConfigureFPGA)
        .def("IsFrontPanelEnabled", &okCFrontPanel::IsFrontPanelEnabled)
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