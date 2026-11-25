"""Get and display sensor information."""

import argparse

from urg3d import Sensor


def ParseArguments() -> argparse.Namespace:
    """@brief Parse command-line arguments."""
    parser = argparse.ArgumentParser(description="URG-3D sensor information sample")
    parser.add_argument("--ip", default="192.168.0.10", help="sensor IP address")
    parser.add_argument("--port", type=int, default=10940, help="sensor TCP port")
    return parser.parse_args()


def Main() -> int:
    """@brief Get and display sensor information."""
    arguments = ParseArguments()
    sensor = Sensor()

    try:
        result = sensor.open(arguments.ip, arguments.port)
        if result != 0:
            print(f"Failed to connect to the sensor: {result}")
            return 1

        result, initial_data = sensor.initialize()
        if result < 0:
            print(f"Failed to initialize the sensor: {result}")
            return 1

        result, info = sensor.get_sensor_info()
        if result < 0:
            print(f"Failed to get sensor information: {result}")
            return 1

        specification = info.spec
        version = info.version
        print(f"Vendor: {version.vendor}")
        print(f"Product: {version.product}")
        print(f"Serial: {version.serial}")
        print(f"Firmware: {version.firmware}")
        print(f"Protocol: {version.protocol}")
        print(f"Motor interlace count: {specification.motor_interlace_count}")
        print(f"REM interlace count: {specification.rem_interlace_count}")
        print(f"Line range: {specification.first_line_number} - {specification.last_line_number}")
        print(f"Line count: {specification.line_count}")
        print(f"Spot count: {specification.spot_count}")
        print(f"Echo count: {specification.echo_count}")
        print(f"Region count: {specification.region_count}")
        return 0
    finally:
        sensor.close()


if __name__ == "__main__":
    raise SystemExit(Main())
