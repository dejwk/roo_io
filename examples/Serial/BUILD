load("@roo_testing//roo_testing/emulation:arduino.bzl", "roo_arduino_example")

roo_arduino_example(
    name = "Serial",
    sketch = "Serial.ino",
    target_compatible_with = ["@roo_testing//roo_testing/platforms:freertos"] + select({
        "@roo_testing//roo_testing/platforms:is_arduino_esp32c3": ["@platforms//:incompatible"],
        "//conditions:default": [],
    }),
    deps = [
        "//:arduino_stream",
        "//:roo_io",
        "@roo_threads",
    ],
)
