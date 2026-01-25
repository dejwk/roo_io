load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_test(
    name = "arduino_fs_test",
    srcs = [
        "arduino_fs_test.cpp",
    ],
    # includes = glob(["**/*.h"]),
    linkstatic = 1,
    deps = [
        ":fakefs_arduino",
        "@roo_io//:testing",
    ],
)

cc_test(
    name = "arduino_file_input_iterator_test",
    srcs = [
        "arduino_file_input_iterator_test.cpp",
    ],
    # includes = glob(["**/*.h"]),
    linkstatic = 1,
    deps = [
        ":fakefs_arduino",
        "@roo_io//test:testing",
    ],
)

cc_test(
    name = "arduino_file_output_iterator_test",
    srcs = [
        "arduino_file_output_iterator_test.cpp",
    ],
    # includes = glob(["**/*.h"]),
    linkstatic = 1,
    deps = [
        ":fakefs_arduino",
        "@roo_io//test:testing",
    ],
)

cc_library(
    name = "fakefs_arduino",
    srcs = [
        "fakefs_arduino.cpp",
        "fakefs_arduino.h",
    ],
    # includes = glob(["**/*.h"]),
    linkstatic = 1,
    deps = [
        "@roo_io//:testing",
        "@roo_io//test/fs:fakefs",
    ],
)
