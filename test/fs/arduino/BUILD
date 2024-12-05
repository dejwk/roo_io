
cc_library(
    name = "fakefs",
    srcs = [
        "fakefs_arduino.cpp",
        "fakefs_arduino.h",
        "fakefs.h",
        "fakefs.cpp"
    ],
    # includes = glob(["**/*.h"]),
    linkstatic = 1,
    deps = [
        "//lib/roo_io:testing",
    ],
)

cc_test(
    name = "fakefs_test",
    srcs = [
        "fakefs_test.cpp",
    ],
    # includes = glob(["**/*.h"]),
    linkstatic = 1,
    deps = [
        "//lib/roo_io:testing",
        ":fakefs"
    ],
)

cc_test(
    name = "arduino_fs_test",
    srcs = [
        "arduino_fs_test.cpp",
    ],
    # includes = glob(["**/*.h"]),
    linkstatic = 1,
    deps = [
        "//lib/roo_io:testing",
        ":fakefs"
    ],
)
