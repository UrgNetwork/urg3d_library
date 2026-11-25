"""Get range and intensity measurement data."""

import argparse
import time

from urg3d import MeasurementType, Sensor


def ParseArguments() -> argparse.Namespace:
    """@brief Parse command-line arguments."""
    parser = argparse.ArgumentParser(description="URG-3D measurement sample")
    parser.add_argument("--ip", default="192.168.0.10", help="sensor IP address")
    parser.add_argument("--port", type=int, default=10940, help="sensor TCP port")
    parser.add_argument("--count", type=int, default=10, help="number of measurements")
    return parser.parse_args()


def Main() -> int:
    """@brief Get and display range and intensity data."""
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

        result = sensor.start_data(MeasurementType.DISTANCE_INTENSITY)
        if result < 0:
            print(f"Failed to start measurement data: {result}")
            return 1
        data_started = True

        received_count = 0
        while received_count < arguments.count:
            result, measurement_data = sensor.get_measurement_data()
            if result > 0:
                print(
                    f"line={measurement_data.line_number}, "
                    f"spot_count={measurement_data.spot_count}, "
                    f"timestamp={measurement_data.timestamp}"
                )
                if measurement_data.spots:
                    first_spot = measurement_data.spots[0]
                    if first_spot.polar:
                        first_polar = first_spot.polar[0]
                        print(
                            f"first_spot: range_m={first_polar.range_m}, "
                            f"vertical_rad={first_polar.vertical_rad}, "
                            f"horizontal_rad={first_polar.horizontal_rad}, "
                            f"intensity={first_polar.intensity}"
                        )
                received_count += 1
            elif result < 0:
                print(f"Failed to get measurement data: {result}")
                return 1
            else:
                time.sleep(0.01)

        return 0
    finally:
        if data_started:
            sensor.stop_data(MeasurementType.DISTANCE_INTENSITY)
        sensor.close()


if __name__ == "__main__":
    raise SystemExit(Main())
