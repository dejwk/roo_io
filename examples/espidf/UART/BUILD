load("@roo_testing//roo_testing/emulation:esp_idf.bzl", "roo_esp_idf_example")

roo_esp_idf_example(
    name = "UART",
    srcs = ["main.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//:esp32_uart",
        "@roo_testing//roo_testing/frameworks/esp-idf",
    ],
)
