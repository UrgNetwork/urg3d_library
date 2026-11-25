"""Send a VSSP command and display the response."""

import argparse
import time

from urg3d import Sensor


def ParseArguments() -> argparse.Namespace:
    """@brief Parse command-line arguments."""
    parser = argparse.ArgumentParser(description="URG-3D low-level request sample")
    parser.add_argument("command", nargs="?", default="VER", help="VSSP command")
    parser.add_argument("--ip", default="192.168.0.10", help="sensor IP address")
    parser.add_argument("--port", type=int, default=10940, help="sensor TCP port")
    parser.add_argument("--timeout", type=int, default=2000, help="timeout in milliseconds")
    return parser.parse_args()


def Main() -> int:
    """@brief Send a VSSP command and display the response."""
    arguments = ParseArguments()
    sensor = Sensor()

    try:
        result = sensor.open(arguments.ip, arguments.port)
        if result != 0:
            print(f"Failed to connect to the sensor: {result}")
            return 1

        sensor.set_blocking_timeout_ms(arguments.timeout)
        command = arguments.command.rstrip("\r\n") + "\n"
        result = sensor.request_command(command)
        if result < 0:
            print(f"Failed to send the command: {result}")
            return 1

        for _ in range(200):
            ready, packet_type, status = sensor.next_receive_ready()
            if ready > 0:
                result, header, data, length_data = sensor.get_binary()
                if result <= 0:
                    print(f"Failed to get response data: {result}")
                    return 1
                print(f"type={packet_type}, status={status}")
                print(f"header={header}")
                print(f"length={length_data}")
                print(f"data={data!r}")
                return 0
            if ready < 0:
                print(f"Failed to check the receive state: {ready}")
                return 1
            time.sleep(0.01)

        print("No response was received.")
        return 1
    finally:
        sensor.close()


if __name__ == "__main__":
    raise SystemExit(Main())
