load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "sdmmc",
    srcs = ["main.cpp"],
    target_compatible_with = select({
        "@roo_testing//roo_testing/platforms:is_idf": [],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    deps = [
        "//:esp32_idf_sdmmc",
        "@roo_threads",
    ],
)
