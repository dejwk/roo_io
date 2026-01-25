load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "roo_io",
    srcs = glob(
        [
            "src/**/*.cpp",
            "src/**/*.c",
            "src/**/*.h",
        ],
        exclude = ["test/**"],
    ),
    defines = ["ARDUINO=10000"],
    includes = [
        "src",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@roo_backport",
        "@roo_logging",
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/FS",
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/SD",
        "@roo_testing//roo_testing/frameworks/esp-idf-v4.4.1",
        "@roo_threads",
    ],
)

cc_library(
    name = "testing",
    srcs = glob(
        [
            "src/**/*.cpp",
            "src/**/*.h",
        ],
        exclude = ["test/**"],
    ),
    defines = ["ROO_IO_TESTING"],
    includes = [
        "src",
    ],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        ":roo_io",
        "@roo_testing//:arduino_gtest_main",
    ],
)
