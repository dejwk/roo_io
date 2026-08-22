load("@roo_testing//roo_testing/emulation:esp_idf.bzl", "roo_esp_idf_example")

roo_esp_idf_example(
    name = "SD",
    srcs = ["main.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//:esp32_idf_sdspi",
        "@roo_testing//roo_testing/frameworks/esp-idf",
        "@roo_testing//roo_testing/microcontrollers/esp32",
        "@roo_threads",
    ],
)
