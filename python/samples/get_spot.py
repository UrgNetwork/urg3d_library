"""Continuously display range and intensity for a selected spot."""

import argparse
import time

from urg3d import MeasurementType, Sensor


c_defaultMotorField = 0
c_defaultRemField = 0
c_defaultLine = 5
c_defaultSpot = 96


def ParseArguments() -> argparse.Namespace:
    """@brief Parse command-line arguments."""
    parser = argparse.ArgumentParser(description="URG-3D spot measurement sample")
    parser.add_argument("--ip", default="192.168.0.10", help="sensor IP address")
    parser.add_argument("--port", type=int, default=10940, help="sensor TCP port")
    parser.add_argument("--motor-field", type=int, default=c_defaultMotorField, help="motor field number")
    parser.add_argument("--rem-field", type=int, default=c_defaultRemField, help="REM field number")
    parser.add_argument("--line", type=int, default=c_defaultLine, help="line number")
    parser.add_argument("--spot", type=int, default=c_defaultSpot, help="spot number")
    return parser.parse_args()


def Main() -> int:
    """@brief Display range and intensity for the selected spot until stopped."""
    arguments = ParseArguments()
    sensor = Sensor()
    data_started = False

    try:
        result = sensor.open(arguments.ip, arguments.port)
        if result != 0:
            print(f"Failed to connect to the sensor: {result}")
            return 1

        sensor.set_blocking_timeout_ms(2000)
        result, initial_data = sensor.initialize()
        if result < 0:
            print(f"Failed to initialize the sensor: {result}")
            return 1

        result = sensor.wait_finished_initialize()
        if result < 0:
            print(f"Failed to wait for sensor initialization: {result}")
            return 1

        result = sensor.set_motor_interlace_count(1)
        if result < 0:
            print(f"Failed to set the motor interlace count: {result}")
            return 1

        result = sensor.set_rem_interlace_count(1)
        if result < 0:
            print(f"Failed to set the ReM interlace count: {result}")
            return 1

        result = sensor.start_data(MeasurementType.DISTANCE_INTENSITY)
        if result < 0:
            print(f"Failed to start measurement data: {result}")
            return 1
        data_started = True

        print(
            f"motor_field={arguments.motor_field}, "
            f"rem_field={arguments.rem_field}, line={arguments.line}, "
            f"spot={arguments.spot}"
        )
        print("Press Ctrl+C to stop.")

        while True:
            result, measurement_data = sensor.get_measurement_data()
            if result > 0:
                if not measurement_data.is_line_data_ready:
                    continue
                if measurement_data.motor_field_number != arguments.motor_field:
                    continue
                if measurement_data.rem_field_number != arguments.rem_field:
                    continue
                if measurement_data.line_number != arguments.line:
                    continue
                if arguments.spot < 0 or arguments.spot >= len(measurement_data.spots):
                    print(f"Spot number is out of range: {arguments.spot}")
                    return 1

                spot = measurement_data.spots[arguments.spot]
                for echo_index, polar in enumerate(spot.polar):
                    print(
                        f"frame={measurement_data.frame_number}, "
                        f"echo={echo_index}, range_m={polar.range_m}, "
                        f"intensity={polar.intensity}"
                    )
            elif result < 0:
                print(f"Failed to get measurement data: {result}")
                return 1
            else:
                time.sleep(0.01)
    except KeyboardInterrupt:
        print("Stop request received.")
        return 0
    finally:
        if data_started:
            sensor.stop_data(MeasurementType.DISTANCE_INTENSITY)
        sensor.close()


if __name__ == "__main__":
    raise SystemExit(Main())

