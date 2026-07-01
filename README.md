# Drone IMU (Reset Baseline)

This repository is currently in a troubleshooting reset mode.

Goal:

1. Verify extension import stability first.
2. Add IMU and PID features back one layer at a time.

## Current Version Scope

Version `0.2.2` is an ultra-minimal TypeScript-only baseline with no native C++ shims.

Exported APIs:

1. `init()`
2. `shimSanityCheck()`

Behavior:

1. `init()` marks an internal initialized flag.
2. `shimSanityCheck()` returns `true` only after `init()` was called.

## Import Test (First Gate)

In a brand-new MakeCode project, import:

1. `https://github.com/kwleung-cityu/drone-imu#v0.2.2`

Then run only:

```python
basic.show_string("HELLO")
```

If this works, run:

```python
droneIMU.init()
ok = droneIMU.shimSanityCheck()
if ok:
    basic.show_string("OK")
else:
    basic.show_string("ERR")
```

## Next Rebuild Plan

After the baseline import is stable:

1. Add TS-only scalar read APIs.
2. Add simulated/static IMU data path.
3. Re-introduce native C++ shim declarations.
4. Re-introduce `droneIMU.cpp` features incrementally.
5. Add PID APIs last.
