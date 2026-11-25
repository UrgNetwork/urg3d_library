#include <array>
#include <cstdint>
#include <string>
#include <utility>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Urg3dErrno.h"
#include "Urg3dSensor.h"
#include "Urg3d_t.h"

namespace py = pybind11;

/**
 * @brief Bind the measurement type enumeration.
 * @param module Python module to extend.
 */
static void BindMeasurementType(py::module_ &module)
{
    py::enum_<URG3D_MEASUREMENT_TYPE>(module, "MeasurementType")
        .value("NO_REQUEST", URG3D_NO_REQUEST)
        .value("DISTANCE", URG3D_DISTANCE)
        .value("DISTANCE_INTENSITY", URG3D_DISTANCE_INTENSITY)
        .value("AUXILIARY", URG3D_AUXILIARY)
        .value("AUGMENTED_DISTANCE_INTENSITY", URG3D_AUGMENTED_DISTANCE_INTENSITY)
        .export_values();
}

/**
 * @brief Bind the auxiliary data type enumeration.
 * @param module Python module to extend.
 */
static void BindAuxiliaryType(py::module_ &module)
{
    py::enum_<URG3D_AUXILIARY_TYPE>(module, "AuxiliaryType", py::arithmetic())
        .value("NO_RECORD", URG3D_NO_RECORD)
        .value("GYRO_DATA", URG3D_GYRO_DATA)
        .value("ACCEL_DATA", URG3D_ACCEL_DATA)
        .value("COMPASS_DATA", URG3D_COMPASS_DATA)
        .value("TEMPERATURE_DATA", URG3D_TEMPERATURE_DATA)
        .export_values();
}

/**
 * @brief Bind sensor information structures.
 * @param module Python module to extend.
 */
static void BindSensorInformation(py::module_ &module)
{
    py::class_<URG3D_SENSOR_SPEC_T>(module, "SensorSpec")
        .def(py::init([]() { return URG3D_SENSOR_SPEC_T{}; }))
        .def_readwrite("motor_interlace_count", &URG3D_SENSOR_SPEC_T::motorInterlaceCount)
        .def_readwrite("rem_interlace_count", &URG3D_SENSOR_SPEC_T::remInterlaceCount)
        .def_readwrite("first_line_number", &URG3D_SENSOR_SPEC_T::firstLineNumber)
        .def_readwrite("last_line_number", &URG3D_SENSOR_SPEC_T::lastLineNumber)
        .def_readwrite("line_count", &URG3D_SENSOR_SPEC_T::lineCount)
        .def_readwrite("spot_count", &URG3D_SENSOR_SPEC_T::spotCount)
        .def_readwrite("echo_count", &URG3D_SENSOR_SPEC_T::echoCount)
        .def_readwrite("region_count", &URG3D_SENSOR_SPEC_T::regionCount)
        .def_readwrite("is_lsb_result", &URG3D_SENSOR_SPEC_T::isLsbResult)
        .def_readwrite("accel_scale", &URG3D_SENSOR_SPEC_T::accelScale)
        .def_readwrite("gyro_scale", &URG3D_SENSOR_SPEC_T::gyroScale)
        .def_readwrite("swing_dir", &URG3D_SENSOR_SPEC_T::swingDir);

    py::class_<URG3D_SENSOR_VERSION_T>(module, "SensorVersion")
        .def(py::init([]() { return URG3D_SENSOR_VERSION_T{}; }))
        .def_property("vendor", [](const URG3D_SENSOR_VERSION_T &value) {
            return std::string(value.vendor);
        }, [](URG3D_SENSOR_VERSION_T &value, const std::string &text) {
            const std::size_t size = text.copy(value.vendor, URG3D_MAX_STRING_SIZE - 1U);
            value.vendor[size] = '\0';
        })
        .def_property("product", [](const URG3D_SENSOR_VERSION_T &value) {
            return std::string(value.product);
        }, [](URG3D_SENSOR_VERSION_T &value, const std::string &text) {
            const std::size_t size = text.copy(value.product, URG3D_MAX_STRING_SIZE - 1U);
            value.product[size] = '\0';
        })
        .def_property("serial", [](const URG3D_SENSOR_VERSION_T &value) {
            return std::string(value.serial);
        }, [](URG3D_SENSOR_VERSION_T &value, const std::string &text) {
            const std::size_t size = text.copy(value.serial, URG3D_MAX_STRING_SIZE - 1U);
            value.serial[size] = '\0';
        })
        .def_property("firmware", [](const URG3D_SENSOR_VERSION_T &value) {
            return std::string(value.firmware);
        }, [](URG3D_SENSOR_VERSION_T &value, const std::string &text) {
            const std::size_t size = text.copy(value.firmware, URG3D_MAX_STRING_SIZE - 1U);
            value.firmware[size] = '\0';
        })
        .def_property("protocol", [](const URG3D_SENSOR_VERSION_T &value) {
            return std::string(value.protocol);
        }, [](URG3D_SENSOR_VERSION_T &value, const std::string &text) {
            const std::size_t size = text.copy(value.protocol, URG3D_MAX_STRING_SIZE - 1U);
            value.protocol[size] = '\0';
        });

    py::class_<URG3D_SENSOR_INFO_T>(module, "SensorInfo")
        .def(py::init([]() { return URG3D_SENSOR_INFO_T{}; }))
        .def_readwrite("spec", &URG3D_SENSOR_INFO_T::spec)
        .def_readwrite("version", &URG3D_SENSOR_INFO_T::version);
}

/**
 * @brief Resize a measurement result for the connected sensor.
 * @param data Measurement result to resize.
 * @param specification Sensor specification.
 */
static void ResizeMeasurementData(URG3D_MEASUREMENT_DATA_T &data,
                                  const URG3D_SENSOR_SPEC_T &specification)
{
    data.spots.resize(specification.spotCount);
    data.totalDetectionArea.resize(specification.regionCount);

    for (std::uint16_t spotIndex = 0U;
         spotIndex < specification.spotCount;
         ++spotIndex) {
        data.spots[spotIndex].polar.resize(specification.echoCount);
        data.spots[spotIndex].point.resize(specification.echoCount);
        data.spots[spotIndex].detectionResult.resize(specification.echoCount);

        for (std::uint8_t echoIndex = 0U;
             echoIndex < specification.echoCount;
             ++echoIndex) {
            data.spots[spotIndex].detectionResult[echoIndex].isDetected.resize(
                specification.regionCount);
        }
    }
}

/**
 * @brief Bind measurement and auxiliary result structures.
 * @param module Python module to extend.
 */
static void BindDataStructures(py::module_ &module)
{
    py::class_<URG3D_RAW_RANGE_INTENSITY_T>(module, "RawRangeIntensity")
        .def(py::init([]() { return URG3D_RAW_RANGE_INTENSITY_T{}; }))
        .def_readwrite("range_mm", &URG3D_RAW_RANGE_INTENSITY_T::rangeMm)
        .def_readwrite("intensity", &URG3D_RAW_RANGE_INTENSITY_T::intensity);

    py::class_<URG3D_RAW_RANGE_T>(module, "RawRange")
        .def(py::init([]() { return URG3D_RAW_RANGE_T{}; }))
        .def_readwrite("range_mm", &URG3D_RAW_RANGE_T::rangeMm);

    py::class_<URG3D_POLAR_T>(module, "Polar")
        .def(py::init([]() { return URG3D_POLAR_T{}; }))
        .def_readwrite("range_m", &URG3D_POLAR_T::rangeM)
        .def_readwrite("vertical_rad", &URG3D_POLAR_T::verticalRad)
        .def_readwrite("horizontal_rad", &URG3D_POLAR_T::horizontalRad)
        .def_readwrite("intensity", &URG3D_POLAR_T::intensity);

    py::class_<URG3D_POINT_T>(module, "Point")
        .def(py::init([]() { return URG3D_POINT_T{}; }))
        .def_readwrite("x_m", &URG3D_POINT_T::xm)
        .def_readwrite("y_m", &URG3D_POINT_T::ym)
        .def_readwrite("z_m", &URG3D_POINT_T::zm)
        .def_readwrite("intensity", &URG3D_POINT_T::intensity);

    py::class_<URG3D_LN_RESULT_T>(module, "LineResult")
        .def(py::init([]() { return URG3D_LN_RESULT_T{}; }))
        .def_readwrite("is_detected", &URG3D_LN_RESULT_T::isDetected);

    py::class_<URG3D_SPOT_T>(module, "Spot")
        .def(py::init([]() { return URG3D_SPOT_T{}; }))
        .def_readwrite("polar", &URG3D_SPOT_T::polar)
        .def_readwrite("point", &URG3D_SPOT_T::point)
        .def_readwrite("detection_result", &URG3D_SPOT_T::detectionResult)
        .def_readwrite("echo_count", &URG3D_SPOT_T::echoCount);

    py::class_<URG3D_MEASUREMENT_DATA_T>(module, "MeasurementData")
        .def(py::init([]() { return URG3D_MEASUREMENT_DATA_T{}; }))
        .def_readwrite("spots", &URG3D_MEASUREMENT_DATA_T::spots)
        .def_readwrite("timestamp", &URG3D_MEASUREMENT_DATA_T::timestamp)
        .def_readwrite("frame_number", &URG3D_MEASUREMENT_DATA_T::frameNumber)
        .def_readwrite("motor_field_number", &URG3D_MEASUREMENT_DATA_T::motorFieldNumber)
        .def_readwrite("rem_field_number", &URG3D_MEASUREMENT_DATA_T::remFieldNumber)
        .def_readwrite("line_number", &URG3D_MEASUREMENT_DATA_T::lineNumber)
        .def_readwrite("spot_count", &URG3D_MEASUREMENT_DATA_T::spotCount)
        .def_readwrite("dust_state", &URG3D_MEASUREMENT_DATA_T::dustState)
        .def_readwrite("dust_ratio_x10", &URG3D_MEASUREMENT_DATA_T::dustRatioX10)
        .def_readwrite("output", &URG3D_MEASUREMENT_DATA_T::output)
        .def_readwrite("input", &URG3D_MEASUREMENT_DATA_T::input)
        .def_readwrite("dirt_state", &URG3D_MEASUREMENT_DATA_T::dirtState)
        .def_readwrite("dirt_ratio_x10", &URG3D_MEASUREMENT_DATA_T::dirtRatioX10)
        .def_readwrite("total_detection_area", &URG3D_MEASUREMENT_DATA_T::totalDetectionArea)
        .def_readwrite("is_line_data_ready", &URG3D_MEASUREMENT_DATA_T::isLineDataReady);

    py::class_<URG3D_AUXILIARY_RECORD_T>(module, "AuxiliaryRecord")
        .def(py::init([]() { return URG3D_AUXILIARY_RECORD_T{}; }))
        .def_readwrite("timestamp_ms", &URG3D_AUXILIARY_RECORD_T::timestampMs)
        .def_readwrite("gyro_x", &URG3D_AUXILIARY_RECORD_T::gyroX)
        .def_readwrite("gyro_y", &URG3D_AUXILIARY_RECORD_T::gyroY)
        .def_readwrite("gyro_z", &URG3D_AUXILIARY_RECORD_T::gyroZ)
        .def_readwrite("accel_x", &URG3D_AUXILIARY_RECORD_T::accelX)
        .def_readwrite("accel_y", &URG3D_AUXILIARY_RECORD_T::accelY)
        .def_readwrite("accel_z", &URG3D_AUXILIARY_RECORD_T::accelZ)
        .def_readwrite("compass_x", &URG3D_AUXILIARY_RECORD_T::compassX)
        .def_readwrite("compass_y", &URG3D_AUXILIARY_RECORD_T::compassY)
        .def_readwrite("compass_z", &URG3D_AUXILIARY_RECORD_T::compassZ)
        .def_readwrite("temperature", &URG3D_AUXILIARY_RECORD_T::temperature);

    py::class_<URG3D_AUXILIARY_DATA_T>(module, "AuxiliaryData")
        .def(py::init([]() { return URG3D_AUXILIARY_DATA_T{}; }))
        .def_readwrite("timestamp_ms", &URG3D_AUXILIARY_DATA_T::timestampMs)
        .def_readwrite("type", &URG3D_AUXILIARY_DATA_T::type)
        .def_readwrite("record_count", &URG3D_AUXILIARY_DATA_T::recordCount)
        .def_property_readonly("type_value", [](const URG3D_AUXILIARY_DATA_T &data) {
            return static_cast<std::uint32_t>(data.type);
        })
        .def_property_readonly("records", [](const URG3D_AUXILIARY_DATA_T &data) {
            py::list records;
            for (std::uint32_t recordIndex = 0U;
                 recordIndex < data.recordCount && recordIndex < URG3D_MAX_AUX_COUNT;
                 ++recordIndex) {
                records.append(data.records[recordIndex]);
            }
            return records;
        });
}

/**
 * @brief Bind the sensor class and Python-friendly output methods.
 * @param module Python module to extend.
 */
static void BindSensor(py::module_ &module)
{
    py::class_<Urg3dSensor>(module, "Sensor")
        .def(py::init([]() { return Urg3dSensor(Urg3dSensor::TCP); }))
        .def("open", [](Urg3dSensor &sensor, const std::string &ip, std::int32_t port) {
            return sensor.open(ip.c_str(), static_cast<int>(port));
        })
        .def("close", &Urg3dSensor::close)
        .def("is_active", &Urg3dSensor::isActive)
        .def("next_receive_ready", [](Urg3dSensor &sensor) {
            std::string type{};
            std::string status{};
            const std::int32_t result = sensor.nextReceiveReady(type, status);
            return py::make_tuple(result, type, status);
        })
        .def("purge", &Urg3dSensor::lowPurge)
        .def("request_command", [](Urg3dSensor &sensor, const std::string &command) {
            return sensor.lowRequestCommand(command.c_str());
        })
        .def("get_binary", [](Urg3dSensor &sensor) {
            URG3D_VSSP_HEADER_T header{};
            std::array<char, URG3D_MAX_RX_LENGTH> data{};
            // The C++ API requires int& for the output length.
            int lengthData = 0;
            const std::int32_t result = sensor.lowGetBinary(
                header,
                data.data(),
                lengthData);
            py::dict headerData;
            headerData["mark"] = std::string(header.mark, sizeof(header.mark));
            headerData["type"] = std::string(header.type, sizeof(header.type));
            headerData["status"] = std::string(header.status, sizeof(header.status));
            return py::make_tuple(
                result,
                headerData,
                py::bytes(data.data(), static_cast<std::size_t>(lengthData)),
                lengthData);
        })
        .def("set_blocking_timeout_ms", &Urg3dSensor::highSetBlockingTimeoutMs)
        .def("initialize", [](Urg3dSensor &sensor) {
            URG3D_MEASUREMENT_DATA_T data{};
            const std::int32_t result = sensor.highBlockingInit(data);
            return py::make_tuple(result, data);
        })
        .def("wait_finished_initialize", &Urg3dSensor::highBlockingWaitFinishedInitialize)
        .def("get_sensor_specification", [](Urg3dSensor &sensor) {
            URG3D_SENSOR_SPEC_T specification{};
            const std::int32_t result = sensor.highBlockingGetSensorSpecification(specification);
            return py::make_tuple(result, specification);
        })
        .def("get_sensor_version", [](Urg3dSensor &sensor) {
            URG3D_SENSOR_VERSION_T version{};
            const std::int32_t result = sensor.highBlockingGetSensorVersion(version);
            return py::make_tuple(result, version);
        })
        .def("get_motor_interlace_count", [](Urg3dSensor &sensor) {
            std::uint8_t count = 0U;
            const std::int32_t result = sensor.highBlockingGetMotorInterlaceCount(count);
            return py::make_tuple(result, count);
        })
        .def("get_rem_interlace_count", [](Urg3dSensor &sensor) {
            std::uint8_t count = 0U;
            const std::int32_t result = sensor.highBlockingGetRemInterlaceCount(count);
            return py::make_tuple(result, count);
        })
        .def("get_motor_angle_table", &Urg3dSensor::highBlockingGetMotorAngleTable)
        .def("get_motor_angle_table_multi", &Urg3dSensor::highBlockingGetMotorAngleTableMulti)
        .def("get_rem_angle_table", &Urg3dSensor::highBlockingGetRemAngleTable)
        .def("get_rem_angle_table_multi", &Urg3dSensor::highBlockingGetRemAngleTableMulti)
        .def("set_motor_interlace_count", &Urg3dSensor::highBlockingSetMotorInterlaceCount)
        .def("set_rem_interlace_count", &Urg3dSensor::highBlockingSetRemInterlaceCount)
        .def("start_data_async", &Urg3dSensor::highStartData)
        .def("start_data", &Urg3dSensor::highBlockingStartData)
        .def("stop_data_async", &Urg3dSensor::highStopData)
        .def("stop_data", &Urg3dSensor::highBlockingStopData)
        .def("get_measurement_data", [](Urg3dSensor &sensor) {
            URG3D_MEASUREMENT_DATA_T data{};
            URG3D_SENSOR_INFO_T info{};
            const std::int32_t infoResult = sensor.highGetSensorInfo(info);
            if (infoResult < 0) {
                return py::make_tuple(infoResult, data);
            }
            ResizeMeasurementData(data, info.spec);
            const std::int32_t result = sensor.highGetMeasurementData(data);
            return py::make_tuple(result, data);
        })
        .def("get_auxiliary_data", [](Urg3dSensor &sensor) {
            URG3D_AUXILIARY_DATA_T data{};
            const std::int32_t result = sensor.highGetAuxiliaryData(data);
            return py::make_tuple(result, data);
        })
        .def("restart", &Urg3dSensor::highBlockingRestart)
        .def("get_sensor_info", [](Urg3dSensor &sensor) {
            URG3D_SENSOR_INFO_T info{};
            const std::int32_t result = sensor.highGetSensorInfo(info);
            return py::make_tuple(result, info);
        });
}

/**
 * @brief Initialize the native Python module.
 * @param module Python module to initialize.
 */
PYBIND11_MODULE(_native, module)
{
    module.doc() = "Native bindings for the URG-3D C++ library.";
    BindMeasurementType(module);
    BindAuxiliaryType(module);
    BindSensorInformation(module);
    BindDataStructures(module);
    BindSensor(module);
}
