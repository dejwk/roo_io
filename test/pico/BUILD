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
