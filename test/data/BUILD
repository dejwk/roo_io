load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_test(
    name = "byte_order_test",
    size = "small",
    srcs = [
        "byte_order_test.cpp",
    ],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = 1,
    deps = [
        "//:testing",
    ],
)

cc_test(
    name = "read_test",
    size = "small",
    srcs = [
        "read_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        "//:testing",
    ],
)

cc_test(
    name = "write_test",
    size = "small",
    srcs = [
        "write_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        "//:testing",
    ],
)

cc_test(
    name = "input_stream_reader_test",
    size = "small",
    srcs = [
        "input_stream_reader_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        "//:testing",
    ],
)

cc_test(
    name = "multipass_input_stream_reader_test",
    size = "small",
    srcs = [
        "multipass_input_stream_reader_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        "//:testing",
    ],
)

cc_test(
    name = "output_stream_writer_test",
    size = "small",
    srcs = [
        "output_stream_writer_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        "//:testing",
    ],
)
