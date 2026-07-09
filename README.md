# Drone IMU Extension

This repository is used for MakeCode extension rebuild and cache-safe release testing.

## Current Status

Validated as of `v1.0.48`:

1. MakeCode dependency pinning by tag is the reliable update path.
2. WHO_AM_I at address `0x68` returns `104` on the verified hardware.
3. Gyroscope raw values and deg/s conversion are working.
4. Accelerometer raw values and `g` conversion are working.
5. Low-level hardware I2C access has been migrated into C++ shims in `droneIMU.cpp`.
6. A C++ DSO timing harness is available for 100 Hz loop validation.

Most relevant timing observations from today's DSO checks:

1. Simple C++ toggle loop with optional single gyro read measured about `97.4 Hz`.
2. Full 14-byte burst-read timing test measured about `88 Hz` with:
    - period about `11.3 ms`
    - high pulse about `1.6 ms`
    - low pulse about `9.7 ms`
3. I2C transfer was observed during the high pulse in the burst-read test.
4. I2C clock on the bus measured about `100 kHz`.

Interpretation:

1. The burst read itself is not prohibitively expensive.
2. The current burst-read harness sleeps a fixed `10 ms` after each read, so total cycle time is `read time + 10 ms`.
3. This strongly suggests a true scheduled 100 Hz C++ loop is feasible if the sleep is reduced to the remaining slot time instead of a fixed 10 ms.

## Files

1. `pxt.json`
2. `main.ts`
3. `README.md`

## IMPORTANT

Do **NOT** rely on standard MakeCode **Extensions** Git URL import for this repository.
It may load stale package metadata and old APIs.

Use this workflow instead:

1. Open your project `pxt.json` in MakeCode.
2. Pin dependency directly to a release tag:

```json
"dependencies": {
    "core": "*",
    "drone-imu-v3-min": "github:kwleung-cityu/drone-imu#v1.0.45"
}
```

3. Save `pxt.json` and let MakeCode reload packages.

This method is confirmed to fetch the correct tagged version without creating a new repo or project.

## Verify Loaded Version

Use this Python probe after updating dependency tag:

```python
serial.write_value("probe", droneIMUV3.releaseProbe129())
serial.write_value("sig", droneIMUV3.buildSignatureCode())
```

Expected for `v1.0.29`:

1. `probe:129`
2. `sig:41029`

If values do not match, MakeCode is still using stale package content.

## Python Quick Check

```python
droneIMUV3.init()
basic.show_string("T")
serial.write_value("probe", droneIMUV3.releaseProbe129())
serial.write_value("sig", droneIMUV3.buildSignatureCode())
basic.show_string("Y")
```

For hardware verification, `droneIMUV3.hardwareWhoAmI()` (or `droneIMUV3.whoAmI()`) should return `104` when AD0 is tied to GND.

For diagnostics, also read `droneIMUV3.whoAmIAt68()`, `droneIMUV3.whoAmIAt69()`, and `droneIMUV3.activeI2cAddress()`.

`droneIMUV3.readSensorPacketValid()` should return `true` when a 14-byte packet is received.

Use `droneIMUV3.refreshSensorSnapshot()` before reading roll/pitch/yaw if you want all three values from the same burst packet.

If stationary bias is present (for example roll around `-12 deg/s` at rest), call `droneIMUV3.calibrateGyroBias(64)` while the sensor is fixed on a table.

`droneIMUV3.readRollRate()`, `droneIMUV3.readPitchRate()`, and `droneIMUV3.readYawRate()` provide gyro rates in deg/s.

For low-level diagnostics, use `droneIMUV3.readRawGyroX()`, `droneIMUV3.readRawGyroY()`, and `droneIMUV3.readRawGyroZ()`.

`v1.0.26` adds accelerometer readings: `readRawAccelX/Y/Z()` and `readAccelXg/Yg/Zg()`.

`v1.0.27` migrates low-level hardware I2C operations into C++ shims (`droneIMU.cpp`) while keeping user-facing APIs in TypeScript.

For DSO timing checks, use `run100HzToggleTest(pin, cycles, includeRead)` or Python alias `run_100hz_toggle_test(pin, cycles, includeRead)`.

For a full MPU6050 14-byte read inside each 100 Hz cycle, use `run100HzBurstReadToggleTest(pin, cycles)` or Python alias `run_100hz_burst_read_toggle_test(pin, cycles)`.

`v1.0.20` also includes Python-friendly underscore aliases for WHO_AM_I/address diagnostics (for example `who_am_i_at_68()`, `who_am_i_at_69()`, `active_i2c_address()`).

For runtime verification, `droneIMUV3.nativeConstant()` should return `123` in both simulator and device (stability mode).

If this works, your dependency pin is healthy and you can proceed with feature development.
