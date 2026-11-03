load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "encoding_c",
    srcs = glob(["src/roo_io/**/*.c"]),
    hdrs = glob(["src/roo_io/third_party/**/*.h"]),
    includes = ["src"],
)

# Portable code has no filesystem adapter or blocking-pipe dependency.
cc_library(
    name = "core",
    srcs = glob(
        [
            "src/roo_io/**/*.cpp",
        ],
        exclude = [
            "src/roo_io/fs/**",
            "src/roo_io/ringpipe/**",
            "src/roo_io/i2c/**",
            "src/roo_io/spi/**",
            "src/roo_io/uart/**",
            "src/roo_io/stream/**",
        ],
    ),
    hdrs = glob(
        ["src/roo_io/**/*.h"],
        exclude = [
            "src/roo_io/fs/**",
            "src/roo_io/ringpipe/**",
            "src/roo_io/i2c/**",
            "src/roo_io/spi/**",
            "src/roo_io/uart/**",
            "src/roo_io/stream/**",
        ],
    ),
    copts = [
        "-fno-exceptions",
        "-fno-rtti",
    ],
    includes = ["src"],
    visibility = ["//visibility:public"],
    deps = [
        ":encoding_c",
        "@roo_backport",
        "@roo_logging",
    ],
)

cc_library(
    name = "filesystem",
    srcs = glob(["src/roo_io/fs/*.cpp"]),
    hdrs = glob(["src/roo_io/fs/*.h"]) + ["src/roo_io.h"],
    includes = ["src"],
    visibility = ["//visibility:public"],
    deps = [":core"],
)

cc_library(
    name = "posix_fs",
    srcs = glob(["src/roo_io/fs/posix/*.cpp"]),
    hdrs = glob(["src/roo_io/fs/posix/*.h"]),
    visibility = ["//visibility:public"],
    deps = [":filesystem"],
)

cc_library(
    name = "ringbuffer",
    hdrs = ["src/roo_io/ringpipe/ringbuffer.h"],
    includes = ["src"],
    visibility = ["//visibility:public"],
    deps = [":core"],
)

cc_library(
    name = "ringpipe",
    srcs = ["src/roo_io/ringpipe/ringpipe.cpp"],
    hdrs = glob(
        ["src/roo_io/ringpipe/*.h"],
        exclude = ["src/roo_io/ringpipe/ringbuffer.h"],
    ),
    target_compatible_with = select({
        "@roo_testing//roo_testing/platforms:is_bare_metal": ["@platforms//:incompatible"],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
    deps = [
        ":ringbuffer",
        "@roo_threads",
    ],
)

# Preserve the existing aggregate on threaded runtimes. Bare metal exports
# portable facilities and requires explicit use of supported nonblocking APIs.
cc_library(
    name = "roo_io",
    visibility = ["//visibility:public"],
    deps = [
        ":core",
        ":filesystem",
        ":posix_fs",
        ":ringbuffer",
    ] + select({
        "@roo_testing//roo_testing/platforms:is_bare_metal": [],
        "//conditions:default": [":ringpipe"],
    }),
)

cc_library(
    name = "arduino_stream",
    srcs = glob(["src/roo_io/stream/arduino/**"]),
    hdrs = glob(["src/roo_io/uart/arduino/*.h"]),
    includes = ["src"],
    target_compatible_with = [
        "@roo_testing//roo_testing/platforms:arduino",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":roo_io",
        "@roo_testing//:arduino",
    ],
)

cc_library(
    name = "esp32_uart",
    srcs = glob(["src/roo_io/uart/esp32/**"]),
    includes = ["src"],
    target_compatible_with = ["@roo_testing//roo_testing/platforms:esp_idf"],
    visibility = ["//visibility:public"],
    deps = [
        ":roo_io",
        "@roo_testing//roo_testing/frameworks/esp-idf",
    ],
)

cc_library(
    name = "arduino_fs",
    srcs = glob(
        ["src/roo_io/fs/arduino/**"],
        exclude = ["src/roo_io/fs/arduino/internal/sd_spi_probe.cpp"],
    ),
    copts = [
        "-fno-rtti",
        "-fno-exceptions",
    ],
    includes = ["src"],
    target_compatible_with = [
        "@roo_testing//roo_testing/platforms:arduino",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":arduino_sd_probe_internal",
        ":esp32_fs_headers",
        ":filesystem",
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/FS",
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/SD",
        "@roo_testing//roo_testing/frameworks/esp-idf",
    ],
)

cc_library(
    name = "arduino_sd_probe_internal",
    srcs = ["src/roo_io/fs/arduino/internal/sd_spi_probe.cpp"],
    hdrs = ["src/roo_io/fs/arduino/internal/sd_spi_probe.h"],
    includes = ["src"],
    target_compatible_with = ["@roo_testing//roo_testing/platforms:arduino"],
    visibility = ["//:__pkg__"],
    deps = select({
        "@roo_testing//roo_testing/platforms:is_arduino_rp2040": ["@roo_testing//:arduino"],
        "//conditions:default": ["@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/SPI"],
    }),
)

cc_library(
    name = "esp32_arduino_sdspi",
    srcs = [
        "src/roo_io/fs/esp32/arduino/internal/sd_spi_probe.cpp",
        "src/roo_io/fs/esp32/arduino/sdspi.cpp",
    ],
    includes = ["src"],
    target_compatible_with = [
        "@roo_testing//roo_testing/platforms:esp_idf",
        "@roo_testing//roo_testing/platforms:arduino",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":arduino_sd_probe_internal",
        ":esp32_fs_headers",
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
        "@roo_testing//roo_testing/platforms:esp_idf",
        "@roo_testing//roo_testing/platforms:arduino",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":esp32_fs_headers",
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
    target_compatible_with = ["@roo_testing//roo_testing/platforms:esp_idf"],
    visibility = ["//visibility:public"],
    deps = [
        ":esp32_fs_headers",
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
    target_compatible_with = ["@roo_testing//roo_testing/platforms:esp_idf"],
    visibility = ["//visibility:public"],
    deps = [
        ":esp32_fs_headers",
        ":roo_io",
        "@roo_testing//roo_testing/frameworks/esp-idf",
    ],
)

cc_library(
    name = "esp32_littlefs",
    srcs = ["src/roo_io/fs/esp32/littlefs.cpp"],
    includes = ["src"],
    target_compatible_with = ["@roo_testing//roo_testing/platforms:esp_idf"],
    visibility = ["//visibility:public"],
    deps = [
        ":esp32_fs_headers",
        ":roo_io",
        "@roo_testing//roo_testing/frameworks/esp-idf",
        "@roo_testing//roo_testing/frameworks/esp_littlefs",
    ],
)

cc_library(
    name = "esp32_spiffs",
    srcs = ["src/roo_io/fs/esp32/spiffs.cpp"],
    includes = ["src"],
    target_compatible_with = ["@roo_testing//roo_testing/platforms:esp_idf"],
    visibility = ["//visibility:public"],
    deps = [
        ":esp32_fs_headers",
        ":roo_io",
        "@roo_testing//roo_testing/frameworks/esp-idf",
    ],
)

# Test fixtures use the matching runner without pulling Arduino filesystems.
cc_library(
    name = "testing",
    testonly = True,
    visibility = ["//visibility:public"],
    deps = [":roo_io"] + select({
        "@roo_testing//roo_testing/platforms:is_arduino": ["@roo_testing//:arduino_gtest_main"],
        "@roo_testing//roo_testing/platforms:is_idf": ["@roo_testing//:esp_idf_gtest_main"],
        "@roo_testing//roo_testing/platforms:is_pico": ["@roo_testing//:pico_gtest_main"],
        "//conditions:default": ["@googletest//:gtest_main"],
    }),
)

# Headers and sources separately exposed for scripted transport-boundary tests.
cc_library(
    name = "i2c_headers",
    hdrs = glob(["src/roo_io/i2c/**/*.h"]),
    includes = ["src"],
    visibility = ["//visibility:public"],
    deps = ["@roo_backport"],
)

filegroup(
    name = "arduino_i2c_sources",
    srcs = ["src/roo_io/i2c/arduino/i2c.cpp"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "i2c",
    srcs = select({
        "@roo_testing//roo_testing/platforms:is_arduino": [":arduino_i2c_sources"],
        "@roo_testing//roo_testing/platforms:is_idf": ["src/roo_io/i2c/esp32/i2c.cpp"],
        "@roo_testing//roo_testing/platforms:is_pico": [],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
    deps = [":i2c_headers"] + select({
        "@roo_testing//roo_testing/platforms:is_arduino_rp2040": ["@roo_testing//:arduino"],
        "@roo_testing//roo_testing/platforms:is_arduino": [
            "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/Wire",
        ],
        "@roo_testing//roo_testing/platforms:is_idf": [
            "@roo_testing//roo_testing/frameworks/esp-idf",
        ],
        "@roo_testing//roo_testing/platforms:is_pico": [":pico_i2c"],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "spi_headers",
    hdrs = glob(["src/roo_io/spi/**/*.h"]),
    includes = ["src"],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "arduino_spi_sources",
    srcs = ["src/roo_io/spi/arduino/spi.cpp"],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "idf_spi_sources",
    srcs = ["src/roo_io/spi/esp32/spi.cpp"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "spi",
    srcs = select({
        "@roo_testing//roo_testing/platforms:is_arduino": [":arduino_spi_sources"],
        "@roo_testing//roo_testing/platforms:is_idf": [":idf_spi_sources"],
        "@roo_testing//roo_testing/platforms:is_pico": [],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
    deps = [":spi_headers"] + select({
        "@roo_testing//roo_testing/platforms:is_arduino_rp2040": [
            "@roo_testing//:arduino",
        ],
        "@roo_testing//roo_testing/platforms:is_arduino": [
            "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/SPI",
        ],
        "@roo_testing//roo_testing/platforms:is_idf": [
            "@roo_testing//roo_testing/frameworks/esp-idf",
        ],
        "@roo_testing//roo_testing/platforms:is_pico": [":pico_spi"],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "pico_uart",
    hdrs = ["src/roo_io/uart/pico/uart.h"],
    includes = ["src"],
    target_compatible_with = ["@roo_testing//roo_testing/platforms:rp2040"],
    visibility = ["//visibility:public"],
    deps = [
        ":core",
        "@roo_testing//:pico_sdk",
    ],
)

cc_library(
    name = "pico_i2c",
    srcs = ["src/roo_io/i2c/pico/i2c.cpp"],
    hdrs = ["src/roo_io/i2c/pico/i2c.h"],
    includes = ["src"],
    target_compatible_with = ["@roo_testing//roo_testing/platforms:rp2040"],
    visibility = ["//visibility:public"],
    deps = [
        ":i2c_headers",
        "@roo_testing//:pico_sdk",
        "@roo_threads",
    ],
)

cc_library(
    name = "pico_spi",
    srcs = ["src/roo_io/spi/pico/spi.cpp"],
    hdrs = ["src/roo_io/spi/pico/spi.h"],
    includes = ["src"],
    target_compatible_with = ["@roo_testing//roo_testing/platforms:rp2040"],
    visibility = ["//visibility:public"],
    deps = [
        ":spi_headers",
        "@roo_testing//:pico_sdk",
        "@roo_threads",
    ],
)

cc_library(
    name = "esp32_fs_headers",
    hdrs = glob(["src/roo_io/fs/esp32/**/*.h"]),
    includes = ["src"],
    target_compatible_with = ["@roo_testing//roo_testing/platforms:esp_idf"],
    deps = [
        ":filesystem",
        ":posix_fs",
        "@roo_testing//roo_testing/frameworks/esp-idf",
    ],
)
