
cc_library(
    name = "fakefs",
    srcs = [
        "fakefs.h",
        "fakefs.cpp",
        "fakefs_reference.h",
        "fakefs_reference.cpp",
        "fs_p.h",
        "fs_mount_p.h",
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
    srcs = [
        "fakefs_test.cpp",
    ],
    # includes = glob(["**/*.h"]),
    linkstatic = 1,
    deps = [
        "//:testing",
        ":fakefs"
    ],
)

cc_test(
    name = "reference_fs_test",
    srcs = [
        "fakefs_reference_test.cpp",
    ],
    # includes = glob(["**/*.h"]),
    linkstatic = 1,
    deps = [
        "//:testing",
        ":fakefs"
    ],
)
