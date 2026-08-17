load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "roo_io",
    srcs = glob(
        [
            "src/**/*.cpp",
            "src/**/*.c",
            "src/**/*.h",
        ],
        exclude = [
            "src/roo_io/fs/arduino/**/*.cpp",
            "src/roo_io/fs/esp32/arduino/**/*.cpp",
            "src/roo_io/fs/esp32/esp-idf/**/*.cpp",
            "src/roo_io/fs/esp32/internal/**/*.cpp",
            "src/roo_io/fs/esp32/littlefs.cpp",
            "src/roo_io/fs/esp32/spiffs.cpp",
            "src/roo_io/stream/arduino/**/*.cpp",
            "src/roo_io/uart/esp32/**/*.cpp",
            "test/**",
        ],
    ),
    includes = [
        "src",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@roo_backport",
        "@roo_logging",
        "@roo_threads",
    ] + select({
        "@roo_testing//roo_testing/platforms:is_arduino": [
            "@roo_testing//:arduino",
        ],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "arduino_stream",
    srcs = glob(["src/roo_io/stream/arduino/**"]),
    includes = ["src"],
    target_compatible_with = [
        "@roo_testing//roo_testing/platforms:arduino",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":roo_io",
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/FS",
    ],
)

cc_library(
    name = "esp32_uart",
    srcs = glob(["src/roo_io/uart/esp32/**"]),
    includes = ["src"],
    visibility = ["//visibility:public"],
    deps = [
        ":roo_io",
        "@roo_testing//roo_testing/frameworks/esp-idf",
    ],
)

cc_library(
    name = "arduino_fs",
    srcs = glob(["src/roo_io/fs/arduino/**"]),
    includes = ["src"],
    target_compatible_with = [
        "@roo_testing//roo_testing/platforms:arduino",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":roo_io",
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/FS",
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/SD",
        "@roo_testing//roo_testing/frameworks/esp-idf",
    ],
)

cc_library(
    name = "esp32_arduino_sdspi",
    srcs = [
        "src/roo_io/fs/esp32/arduino/internal/sd_spi_probe.cpp",
        "src/roo_io/fs/esp32/arduino/sdspi.cpp",
    ],
    includes = ["src"],
    target_compatible_with = [
        "@roo_testing//roo_testing/platforms:arduino",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":roo_io",
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/SD",
    ],
)

cc_library(
    name = "esp32_arduino_sdmmc",
    srcs = [
        "src/roo_io/fs/esp32/arduino/sdmmc.cpp",
        "src/roo_io/fs/esp32/internal/sd_mmc_probe.cpp",
    ],
    includes = ["src"],
    target_compatible_with = [
        "@roo_testing//roo_testing/platforms:arduino",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":roo_io",
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/SD_MMC",
    ],
)

cc_library(
    name = "esp32_idf_sdspi",
    srcs = [
        "src/roo_io/fs/esp32/esp-idf/internal/sd_spi_probe.cpp",
        "src/roo_io/fs/esp32/esp-idf/sdspi.cpp",
    ],
    includes = ["src"],
    visibility = ["//visibility:public"],
    deps = [
        ":roo_io",
        "@roo_testing//roo_testing/frameworks/esp-idf",
    ],
)

cc_library(
    name = "esp32_idf_sdmmc",
    srcs = [
        "src/roo_io/fs/esp32/esp-idf/sdmmc.cpp",
        "src/roo_io/fs/esp32/internal/sd_mmc_probe.cpp",
    ],
    includes = ["src"],
    visibility = ["//visibility:public"],
    deps = [
        ":roo_io",
        "@roo_testing//roo_testing/frameworks/esp-idf",
    ],
)

cc_library(
    name = "esp32_littlefs",
    srcs = ["src/roo_io/fs/esp32/littlefs.cpp"],
    includes = ["src"],
    visibility = ["//visibility:public"],
    deps = [
        ":roo_io",
        "@roo_testing//roo_testing/frameworks/esp-idf",
    ],
)

cc_library(
    name = "esp32_spiffs",
    srcs = ["src/roo_io/fs/esp32/spiffs.cpp"],
    includes = ["src"],
    visibility = ["//visibility:public"],
    deps = [
        ":roo_io",
        "@roo_testing//roo_testing/frameworks/esp-idf",
    ],
)

cc_library(
    name = "testing",
    srcs = glob(
        [
            "src/**/*.cpp",
            "src/**/*.h",
        ],
        exclude = ["test/**"],
    ),
    includes = [
        "src",
    ],
    linkstatic = 1,
    target_compatible_with = [
        "@roo_testing//roo_testing/platforms:arduino",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":arduino_fs",
        ":roo_io",
        "@roo_testing//:arduino_gtest_main",
    ],
)
