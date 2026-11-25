"""Get and display auxiliary data."""

import argparse
import time

from urg3d import MeasurementType, Sensor


c_gyroScaleDenominator = 32767.0
c_accelScaleDenominator = 32767.0
c_temperatureScalingFactor = 0.0029951776
c_temperatureShiftValue = 21.0


def ParseArguments() -> argparse.Namespace:
    """@brief Parse command-line arguments."""
    parser = argparse.ArgumentParser(description="URG-3D auxiliary data sample")
    parser.add_argument("--ip", default="192.168.0.10", help="sensor IP address")
    parser.add_argument("--port", type=int, default=10940, help="sensor TCP port")
    parser.add_argument("--count", type=int, default=1, help="number of auxiliary packets")
    return parser.parse_args()


def Main() -> int:
    """@brief Get and display auxiliary data."""
    arguments = ParseArguments()
    sensor = Sensor()
    data_started = False

    try:
        result = sensor.open(arguments.ip, arguments.port)
        if result != 0:
            print(f"Failed to connect to the sensor: {result}")
            return 1

        result, initial_data = sensor.initialize()
        if result < 0:
            print(f"Failed to initialize the sensor: {result}")
            return 1

        result, sensor_info = sensor.get_sensor_info()
        if result < 0:
            print(f"Failed to get sensor specification: {result}")
            return 1

        print(f"accel_scale={sensor_info.spec.accel_scale}")
        print(f"gyro_scale={sensor_info.spec.gyro_scale}")

        result = sensor.start_data(MeasurementType.AUXILIARY)
        if result < 0:
            print(f"Failed to start auxiliary data: {result}")
            return 1
        data_started = True

        received_count = 0
        while received_count < arguments.count:
            result, auxiliary_data = sensor.get_auxiliary_data()
            if result > 0:
                print(
                    f"timestamp={auxiliary_data.timestamp_ms}, "
                    f"record_count={auxiliary_data.record_count}, "
                    f"type_value={auxiliary_data.type_value}"
                )
                if auxiliary_data.records:
                    record = auxiliary_data.records[0]
                    gyro_scale = sensor_info.spec.gyro_scale / c_gyroScaleDenominator
                    accel_scale = sensor_info.spec.accel_scale / c_accelScaleDenominator
                    gyro = (
                        record.gyro_x * gyro_scale,
                        record.gyro_y * gyro_scale,
                        record.gyro_z * gyro_scale,
                    )
                    acceleration = (
                        record.accel_x * accel_scale,
                        record.accel_y * accel_scale,
                        record.accel_z * accel_scale,
                    )
                    temperature = (
                        record.temperature * c_temperatureScalingFactor
                        + c_temperatureShiftValue
                    )
                    print(
                        f"first_record_raw: gyro=({record.gyro_x}, {record.gyro_y}, "
                        f"{record.gyro_z}), accel=({record.accel_x}, "
                        f"{record.accel_y}, {record.accel_z}), "
                        f"compass=({record.compass_x}, {record.compass_y}, "
                        f"{record.compass_z}), temperature={record.temperature}"
                    )
                    print(
                        f"first_record_scaled: gyro_dps={gyro}, "
                        f"accel_g={acceleration}, "
                        f"temperature_c={temperature}"
                    )
                received_count += 1
            elif result < 0:
                print(f"Failed to get auxiliary data: {result}")
                return 1
            else:
                time.sleep(0.01)

        return 0
    finally:
        if data_started:
            sensor.stop_data(MeasurementType.AUXILIARY)
        sensor.close()


if __name__ == "__main__":
    raise SystemExit(Main())
