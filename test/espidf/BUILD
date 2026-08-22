load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_test(
    name = "posix_directory_test",
    size = "small",
    srcs = ["posix_directory_test.cpp"],
    target_compatible_with = select({
        "@roo_testing//roo_testing/platforms:is_idf": [],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    deps = [
        "//:roo_io",
        "@roo_testing//:esp_idf_gtest_main",
    ],
)

cc_test(
    name = "uart_stream_test",
    srcs = ["uart_stream_test.cpp"],
    target_compatible_with = select({
        "@roo_testing//roo_testing/platforms:is_idf": [],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    deps = [
        "//:esp32_uart",
        "@roo_testing//:esp_idf_gtest_main",
    ],
)
