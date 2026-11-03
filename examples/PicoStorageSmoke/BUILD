load("@roo_testing//roo_testing/emulation:arduino.bzl", "roo_arduino_example")

roo_arduino_example(
    name = "PicoStorageSmoke",
    sketch = "PicoStorageSmoke.ino",
    target_compatible_with = ["@roo_testing//roo_testing/platforms:rp2040"],
    deps = ["//:arduino_fs"],
)
