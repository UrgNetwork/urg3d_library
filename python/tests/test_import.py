from urg3d import MeasurementType, Sensor


def test_native_module_imports() -> None:
    sensor = Sensor()

    assert sensor.is_active() == 0
    assert MeasurementType.DISTANCE_INTENSITY.value == 2
