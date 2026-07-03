# Drone IMU Extension

This repository is used for MakeCode extension rebuild and cache-safe release testing.

Current build signature: `V3-MIN-SIG-20260703-C`
Current build signature code: `41017`

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
    "drone-imu-v3-min": "github:kwleung-cityu/drone-imu#v1.0.17"
}
```

3. Save `pxt.json` and let MakeCode reload packages.

This method is confirmed to fetch the correct tagged version without creating a new repo or project.

## Verify Loaded Version

Use this Python probe after updating dependency tag:

```python
serial.write_value("probe", droneIMUV3.releaseProbe117())
serial.write_value("sig", droneIMUV3.buildSignatureCode())
```

Expected for `v1.0.17`:

1. `probe:117`
2. `sig:41017`

If values do not match, MakeCode is still using stale package content.

## Python Quick Check

```python
droneIMUV3.init()
basic.show_string("T")
serial.write_value("probe", droneIMUV3.releaseProbe117())
serial.write_value("sig", droneIMUV3.buildSignatureCode())
basic.show_string("Y")
```

For hardware verification, `droneIMUV3.hardwareWhoAmI()` (or `droneIMUV3.whoAmI()`) should return `104` when AD0 is tied to GND.

`droneIMUV3.readSensorPacketValid()` should return `true` when a 14-byte packet is received.

Use `droneIMUV3.refreshSensorSnapshot()` before reading roll/pitch/yaw if you want all three values from the same burst packet.

`droneIMUV3.readRollRate()`, `droneIMUV3.readPitchRate()`, and `droneIMUV3.readYawRate()` provide gyro rates in deg/s.

For native verification, `droneIMUV3.nativeConstant()` should return `123` on device and `-1` in simulator.

If this works, your dependency pin is healthy and you can proceed with feature development.
