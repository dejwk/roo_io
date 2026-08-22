load("@roo_testing//roo_testing/emulation:arduino.bzl", "roo_arduino_example")

roo_arduino_example(
    name = "SD",
    sketch = "SD.ino",
    visibility = ["//visibility:public"],
    deps = [
        "//:arduino_fs",
        "//:roo_io",
    ],
)
