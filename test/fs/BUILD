load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_library(
    name = "fakefs",
    srcs = [
        "fakefs.cpp",
        "fakefs.h",
        "fakefs_reference.cpp",
        "fakefs_reference.h",
        "fs_mount_p.h",
        "fs_p.h",
    ],
    includes = ["."],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "//:testing",
    ],
)

cc_test(
    name = "fakefs_test",
    size = "small",
    srcs = [
        "fakefs_test.cpp",
    ],
    # includes = glob(["**/*.h"]),
    linkstatic = 1,
    deps = [
        ":fakefs",
        "//:testing",
    ],
)

cc_test(
    name = "reference_fs_test",
    size = "small",
    srcs = [
        "fakefs_reference_test.cpp",
    ],
    # includes = glob(["**/*.h"]),
    linkstatic = 1,
    deps = [
        ":fakefs",
        "//:testing",
    ],
)
