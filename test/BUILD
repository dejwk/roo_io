load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "testing",
    srcs = [
        "input_iterator_p.h",
        "multipass_input_iterator_p.h",
        "output_iterator_p.h",
    ],
    includes = ["."],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "//:testing",
    ],
)
