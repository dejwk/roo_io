
cc_library(
    name = "testing",
    srcs = [
        "input_iterator_p.h",
        "multipass_input_iterator_p.h",
    ],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    includes = ["."],
    deps = [
        "//lib/roo_io:testing",
    ],
)

cc_test(
    name = "unsafe_memory_input_iterator_test",
    srcs = [
        "unsafe_memory_input_iterator_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":testing",
    ],
)

cc_test(
    name = "memory_input_iterator_test",
    srcs = [
        "memory_input_iterator_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":testing",
    ],
)

cc_test(
    name = "buffered_input_stream_iterator_test",
    srcs = [
        "buffered_input_stream_iterator_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":testing",
    ],
)

cc_test(
    name = "buffered_multipass_input_stream_iterator_test",
    srcs = [
        "buffered_multipass_input_stream_iterator_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":testing",
    ],
)

cc_test(
    name = "multipass_memory_input_iterator_test",
    srcs = [
        "multipass_memory_input_iterator_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":testing",
    ],
)

cc_test(
    name = "memory_output_iterator_test",
    srcs = [
        "memory_output_iterator_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":testing",
    ],
)
