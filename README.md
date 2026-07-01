# Drone IMU Extension

This repository is used for MakeCode extension rebuild and cache-safe release testing.

Current build signature: `V3-MIN-SIG-20260701-D`
Current build signature code: `41005`

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
    "drone-imu-v3-min": "github:kwleung-cityu/drone-imu#v1.0.5"
}
```

3. Save `pxt.json` and let MakeCode reload packages.

This method is confirmed to fetch the correct tagged version without creating a new repo or project.

## Verify Loaded Version

Use this Python probe after updating dependency tag:

```python
serial.write_value("probe", droneIMUV3.releaseProbe105())
serial.write_value("sig", droneIMUV3.buildSignatureCode())
```

Expected for `v1.0.5`:

1. `probe:105`
2. `sig:41005`

If values do not match, MakeCode is still using stale package content.

## Python Quick Check

```python
droneIMUV3.init()
basic.show_string("T")
serial.write_value("probe", droneIMUV3.releaseProbe105())
serial.write_value("sig", droneIMUV3.buildSignatureCode())
basic.show_string("Y")
```

If this works, your dependency pin is healthy and you can proceed with feature development.
