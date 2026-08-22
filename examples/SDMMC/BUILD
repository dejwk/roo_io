load("@roo_testing//roo_testing/emulation:arduino.bzl", "roo_arduino_example")

roo_arduino_example(
    name = "SDMMC",
    sketch = "SDMMC.ino",
    visibility = ["//visibility:public"],
    deps = [
        "//:esp32_arduino_sdmmc",
        "//:roo_io",
        "@roo_testing//roo_testing/microcontrollers/esp32",
    ],
)
