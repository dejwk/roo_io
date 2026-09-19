load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_library(
    name = "idf_fake",
    testonly = True,
    srcs = ["idf_fake.cpp"],
    hdrs = ["idf_fake.h"],
    includes = ["."],
    visibility = ["//visibility:public"],
    target_compatible_with = select({
        "@roo_testing//roo_testing/platforms:is_idf": [],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    deps = ["@googletest//:gtest", "@roo_testing//roo_testing/frameworks/esp-idf"],
)

cc_test(
    name = "idf_i2c_test",
    size = "small",
    linkstatic = True,
    srcs = ["idf_i2c_test.cpp"],
    deps = [":idf_fake", "//:i2c", "@googletest//:gtest_main"],
)

cc_test(
    name = "idf_emulator_test",
    size = "small",
    srcs = ["idf_emulator_test.cpp"],
    linkstatic = True,
    target_compatible_with = select({
        "@roo_testing//roo_testing/platforms:is_idf": [],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    deps = [
        "//:i2c",
        "@roo_testing//:esp_idf_gtest_main",
        "@roo_testing//roo_testing/devices/clock/ds3231",
        "@roo_testing//roo_testing/microcontrollers/esp32:core",
    ],
)
