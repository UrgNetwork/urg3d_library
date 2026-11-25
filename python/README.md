# URG-3D Python Library

This library allows Hokuyo URG-3D sensors to be used from Python.

Sensor communication and protocol handling are provided by the C++ `urg3d_library`. The Python package supports sensor connection and initialization, measurement acquisition, auxiliary data acquisition, and sensor information queries.

## Supported environments

- Python 3.9 or later
- Windows or Linux
- A network connection between the sensor and the computer

Prebuilt wheels normally require only a Python runtime. Source builds additionally require CMake, a C++ compiler, `pybind11`, and `scikit-build-core`.

## Installation

### Installing a wheel

Install the package into the Python environment you intend to use. A virtual environment is not required. Use one when you want to isolate dependencies for each project.

```powershell
mkdir my_urg3d_app
cd my_urg3d_app
python -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
python -m pip install urg3d
```

Without a virtual environment, omit the creation and activation steps and run the following command in the Python environment you use:

```powershell
python -m pip install urg3d
```

On Linux, activate a virtual environment with:

```bash
source .venv/bin/activate
```

### Building from source

Source builds require the following development tools:

- Python 3.9 or later
- CMake 3.18 or later
- A C++ compiler
- `pybind11`
- `scikit-build-core`

On Windows, MSVC Build Tools, LLVM/Clang, or MinGW-w64 can be used as the C++ compiler. For example:

- [CMake downloads](https://cmake.org/download/)
- [Microsoft C++ Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/)

On Linux, install CMake and a C++ compiler such as GCC or Clang using your distribution's package manager.

From the repository's `python` directory, install the required Python packages and the library as follows:

```powershell
python -m pip install pybind11 scikit-build-core
python -m pip install .
```

`pip install .` builds the package from the local source and installs it. It can also be used when a prebuilt wheel is not available. Run it again after changing the source.

For an editable installation during development, use `python -m pip install -e .`. This keeps the source tree connected to the installed package. On Windows, an ASCII-only source path is recommended because generated editable-install `.pth` files may be read using the system encoding.

## Basic usage

See the executable samples for concrete usage. The basic flow with `Sensor` is:

1. Connect to the sensor with `open()`.
2. Initialize the sensor with `initialize()`.
3. Select the data type with `start_data()`.
4. Acquire data with `get_measurement_data()` or `get_auxiliary_data()`.
5. Finish with `stop_data()` and `close()`.

Methods return an integer result code. In general, a positive value means that data was acquired, `0` means that no data has arrived yet, and a negative value indicates an error. Methods that return data return a tuple containing the result code and the data object.

## Samples

Samples are available in `python/samples`. Run them with the `python` directory as the current directory.

Display sensor information:

```powershell
python .\\samples\\get_sensor_info.py --ip 192.168.0.10 --port 10940
```

Acquire range and intensity data a specified number of times:

```powershell
python .\\samples\\get_measurement.py --ip 192.168.0.10 --port 10940 --count 10
```

Continuously acquire a selected spot. The sample defaults are an example for one sensor configuration; adjust them to match the sensor's field, line, and spot definition:

```powershell
python .\\samples\\get_spot.py --ip 192.168.0.10 --port 10940
```

Adjust the field, line, and spot values to match the sensor you use. Press `Ctrl+C` to stop.

Acquire auxiliary data:

```powershell
python .\\samples\\get_auxiliary.py --ip 192.168.0.10 --port 10940 --count 1
```

Send a VSSP command directly:

```powershell
python .\\samples\\low_user_request.py VER --ip 192.168.0.10 --port 10940
```

## Main Python API

- `Sensor`: sensor connection, initialization, and data acquisition
- `MeasurementType`: types of data that can be acquired
- `SensorInfo`, `SensorSpec`, `SensorVersion`: sensor information
- `MeasurementData`, `Spot`, `Point`, `Polar`: measurement results
- `AuxiliaryData`, `AuxiliaryRecord`: auxiliary data

## Notes for library developers

A wheel is a distribution file that packages the library in an installable format. Wheels distributed to users are built separately for each target operating system, Python version, and architecture.

To build a wheel, run the following commands from the repository's `python` directory:

```powershell
python -m pip install build pybind11 scikit-build-core
python -m build --wheel
```

Wheel creation also requires CMake and a C++ compiler. When wheels are built in CI, provide these tools in the CI environment. Users who only install a prebuilt wheel normally do not need them.

The generated wheel is written to the `dist` directory. Install the generated wheel to verify it with:

```powershell
python -m pip install .\\dist\\urg3d-2.1.0-*.whl
```
