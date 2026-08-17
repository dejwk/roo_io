load("@rules_cc//cc:cc_binary.bzl", "cc_binary")

cc_binary(
    name = "uart",
    srcs = ["main.cpp"],
    target_compatible_with = select({
        "@roo_testing//roo_testing/platforms:is_idf": [],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    deps = [
        "//:esp32_uart",
        "@roo_testing//:esp_idf_main",
        "@roo_testing//roo_testing/frameworks/esp-idf",
    ],
)
