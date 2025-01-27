cc_library(
    name = "roo_io",
    srcs = glob(
        [
            "src/**/*.cpp",
            "src/**/*.h",
        ],
        exclude = ["test/**"],
    ),
    includes = [
        "src",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//lib/roo_backport",
        "//lib/roo_logging",
        "//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/FS",
        "//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/SD",
        "//roo_testing/frameworks/esp-idf-v4.4.1/components/fatfs",
        "//roo_testing/frameworks/esp-idf-v4.4.1/components/spiffs",
        "//roo_testing/frameworks/esp-idf-v4.4.1/components/driver:sdspi",
        "//roo_testing/libraries/SdFat",
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
    alwayslink = 1,
    includes = [
        "src",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":roo_io",
        "//roo_testing:arduino_gtest_main",
    ],
)
