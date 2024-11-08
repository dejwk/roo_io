cc_library(
    name = "roo_io",
    srcs = glob(
        [
            "src/**/*.cpp",
            "src/**/*.c",
            "src/**/*.h",
            "src/**/*.inl",
        ],
        exclude = ["test/**"],
    ),
    includes = [
        "src",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//lib/roo_logging",
        "//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/FS",
        "//roo_testing/frameworks/esp-idf-v4.4.1/components/fatfs",
        "//roo_testing/frameworks/esp-idf-v4.4.1/components/driver:sdspi",
    ],
)

cc_library(
    name = "testing",
    srcs = glob(
        [
            "src/**/*.cpp",
            "src/**/*.c",
            "src/**/*.h",
            "src/**/*.inl",
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
        "//roo_testing:arduino_gtest_main",
        "//lib/roo_logging",
    ],
)

cc_test(
    name = "byte_order_test",
    srcs = [
        "test/byte_order_test.cpp",
    ],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = 1,
    deps = [
        "//lib/roo_io:testing",
    ],
)

cc_test(
    name = "io_memory_test",
    srcs = [
        "test/io_memory_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        "//lib/roo_io:testing",
    ],
)

cc_test(
    name = "memfill_test",
    srcs = [
        "test/memfill_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        "//lib/roo_io:testing",
    ],
)
