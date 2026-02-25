load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_test(
    name = "string_printf_test",
    size = "small",
    srcs = [
        "string_printf_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        "//:testing",
    ],
)

cc_test(
    name = "unicode_test",
    size = "small",
    srcs = [
        "unicode_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        "//:testing",
    ],
)
