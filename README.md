# Drone IMU (micro:bit v2/v2.2 + MPU6050)

High-speed MPU6050 extension for MakeCode micro:bit with:

1. Native C++ I2C driver for IMU reads at 100 Hz target loop pacing.
2. Native C++ PID core for simple control experiments.
3. TypeScript and MakeCode Python-facing APIs.

## Practical Approach (Recommended)

Use a hardware-first workflow.

1. Edit extension source in VS Code only.
2. Commit and push from VS Code only.
3. In MakeCode, import extension and test on real hardware.
4. Avoid simulator for IMU validation (simulator is only for basic code-path checks).

Why: this avoids most C++ shim/simulator edge cases and keeps classroom flow simple.

## Hardware Setup

Target board: micro:bit v2 or v2.2

MPU6050 wiring:

1. VCC -> 3V
2. GND -> GND
3. SDA -> P20
4. SCL -> P19

Notes:

1. Default MPU6050 I2C address in this extension is 0x68 (AD0 tied low).
2. Connect micro:bit to PC by USB for flashing and serial graph viewing.

## Use as Extension in MakeCode

1. Open https://makecode.microbit.org/
2. Create a new project.
3. Open Extensions.
4. Import this GitHub repository (https://github.com/kwleung-cityu/drone-imu.git) URL.
5. Confirm a new Drone IMU category appears in toolbox.

If Drone IMU appears, extension import is successful.

## Quick Python Smoke Test

Paste in MakeCode Python view:

```python
droneIMU.init()
ok = droneIMU.shim_sanity_check()
if ok:
	basic.show_string("OK")
else:
	basic.show_string("ERR")
```

Expected:

1. On real hardware with sensor connected, should show OK.
2. If ERR, check wiring, power, and I2C lines.

## 100 Hz Test + Serial Graph

Paste in MakeCode Python view:

```python
droneIMU.init()

last_log = 0

while True:
	start_tick = input.running_time()
	telemetry = droneIMU.read_processed_data()

	gx = telemetry[3]
	gy = telemetry[4]

	now = input.running_time()
	if now - last_log >= 20:
		serial.write_value("RollRate", gx)
		serial.write_value("PitchRate", gy)
		last_log = now

	loop_duration = input.running_time() - start_tick
	if loop_duration < 10:
		basic.pause(10 - loop_duration)
```

Then:

1. Flash to micro:bit.
2. Open serial/data viewer in MakeCode.
3. Watch RollRate and PitchRate traces.

## PID API (Extension)

Available APIs:

1. init()
2. shimSanityCheck() -> boolean
3. readProcessedData() -> [ax, ay, az, gx, gy, gz]
4. configurePID(kp, ki, kd, outMin, outMax)
5. resetPID()
6. updatePID(setpoint, measurement, dtMs) -> number

Python aliases are also available and recommended in MakeCode Python when name-conversion bugs appear:

1. shim_sanity_check()
2. read_processed_data()
3. configure_pid(kp, ki, kd, outMin, outMax)
4. reset_pid()
5. update_pid(setpoint, measurement, dtMs)

## Sync Strategy That Avoids Conflicts

Single-writer rule:

1. VS Code is the only edit/commit place for extension source.
2. MakeCode is for importing/pulling and running tests.
3. Do not edit the same extension files in both places.

If MakeCode shows upload arrows without intentional edits:

1. Inspect diff in MakeCode GitHub panel.
2. Discard local web changes unless they are intentional.

## Repository Files (Core)

1. [droneIMU.cpp](droneIMU.cpp): native MPU6050 driver and native PID logic.
2. [main.ts](main.ts): public extension APIs and wrappers.
3. [shims.d.ts](shims.d.ts): native shim declarations.
4. [sim/droneIMU.ts](sim/droneIMU.ts): simulator shim backend.
5. [test.ts](test.ts): local top-level harness for extension development.

## Classroom Suggestion

For beginner classes:

1. Keep extension stable during class.
2. Students import extension in normal MakeCode projects.
3. Students code in Python only; no GitHub sync operations needed.
