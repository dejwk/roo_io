load("@rules_cc//cc:cc_test.bzl", "cc_test")

[
    cc_test(
        name = "arduino_transport_" + mode + "_test",
        size = "small",
        timeout = "short",
        srcs = ["arduino_transport_test.cpp"],
        copts = [
            "-fno-rtti",
            "-fno-exceptions",
        ],
        linkstatic = True,
        target_compatible_with = [
            "@roo_testing//roo_testing/platforms:arduino",
            "@roo_testing//roo_testing/platforms:rp2040",
        ],
        deps = [
            "//:arduino_stream",
            "//:i2c",
            "//:spi",
            "@roo_testing//:arduino_gtest_main",
            "@roo_testing//roo_testing/buses/i2c",
            "@roo_testing//roo_testing/buses/spi",
            "@roo_testing//roo_testing/buses/uart",
            "@roo_testing//roo_testing/microcontrollers/rp2040:pico",
        ] + (["@roo_testing//roo_testing/system:manual_time_mode"] if mode == "manual" else []),
    )
    for mode in [
        "auto",
        "manual",
    ]
]

cc_test(
    name = "stream_contract_test",
    size = "small",
    srcs = ["stream_contract_test.cpp"],
    copts = [
        "-fno-rtti",
        "-fno-exceptions",
    ],
    target_compatible_with = ["@roo_testing//roo_testing/platforms:arduino"],
    deps = [
        "//:arduino_stream",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "sdk_transport_test",
    size = "small",
    timeout = "short",
    srcs = ["sdk_transport_test.cpp"],
    copts = [
        "-fno-rtti",
        "-fno-exceptions",
    ],
    target_compatible_with = ["@roo_testing//roo_testing/platforms:rp2040"],
    deps = [
        "//:pico_i2c",
        "//:pico_spi",
        "//:pico_uart",
        "@roo_testing//roo_testing/buses/i2c",
        "@roo_testing//roo_testing/buses/spi",
        "@roo_testing//roo_testing/buses/uart",
        "@roo_testing//roo_testing/microcontrollers/rp2040:pico",
    ] + select({
        "@roo_testing//roo_testing/platforms:is_arduino": ["@roo_testing//:arduino_gtest_main"],
        "//conditions:default": ["@roo_testing//:pico_gtest_main"],
    }),
)

cc_test(
    name = "storage_test",
    size = "small",
    srcs = ["storage_test.cpp"],
    copts = [
        "-fno-rtti",
        "-fno-exceptions",
    ],
    target_compatible_with = [
        "@roo_testing//roo_testing/platforms:arduino",
        "@roo_testing//roo_testing/platforms:rp2040",
    ],
    deps = [
        "//:arduino_fs",
        "@roo_testing//:arduino_gtest_main",
        "@roo_testing//roo_testing/devices/storage/sd_spi",
        "@roo_testing//roo_testing/frameworks/arduino_pico_support:filesystem",
        "@roo_testing//roo_testing/microcontrollers/rp2040:pico",
    ],
)

cc_test(
    name = "sd_detection_test",
    size = "small",
    timeout = "short",
    srcs = ["sd_detection_test.cpp"],
    copts = [
        "-fno-rtti",
        "-fno-exceptions",
    ],
    target_compatible_with = [
        "@roo_testing//roo_testing/platforms:arduino",
        "@roo_testing//roo_testing/platforms:rp2040",
    ],
    deps = [
        "//:arduino_fs",
        "@roo_testing//:arduino_gtest_main",
        "@roo_testing//roo_testing/devices/storage/sd_spi",
        "@roo_testing//roo_testing/frameworks/arduino_pico_support:filesystem",
        "@roo_testing//roo_testing/microcontrollers/rp2040:pico",
    ],
)
