load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_test(
    name = "ringbuffer_test",
    size = "small",
    srcs = [
        "ringbuffer_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        "//test:testing",
    ],
)

cc_test(
    name = "ringpipe_test",
    size = "small",
    srcs = [
        "ringpipe_test.cpp",
    ],
    linkstatic = 1,
    target_compatible_with = select({
        "@roo_testing//roo_testing/platforms:is_bare_metal": ["@platforms//:incompatible"],
        "//conditions:default": [],
    }),
    deps = [
        "//test:testing",
    ],
)
