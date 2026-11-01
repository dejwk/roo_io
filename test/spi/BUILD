load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_test(
    name = "arduino_spi_test",
    size = "small",
    srcs = [
        "arduino_spi_test.cpp",
        "//:arduino_spi_sources",
    ] + glob(["stubs/*.h"]),
    includes = ["stubs"],
    linkstatic = True,
    target_compatible_with = ["@roo_testing//roo_testing/platforms:arduino"],
    deps = [
        "//:spi_headers",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "idf_spi_test",
    size = "small",
    srcs = ["idf_spi_test.cpp"],
    linkstatic = True,
    target_compatible_with = select({
        "@roo_testing//roo_testing/platforms:is_idf": [],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    deps = [
        "//:spi",
        "@googletest//:gtest_main",
    ],
)
