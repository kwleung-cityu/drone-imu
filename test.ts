// Local tester for this package when opened as the top-level project.
// Not compiled when this package is consumed as an extension.

droneIMU.init()
basic.pause(50)

let samples = 0
let statStart = input.runningTime()
let lastLog = 0

basic.forever(function () {
	const loopStart = input.runningTime()
	const telemetry = droneIMU.readProcessedData()

	// [ax, ay, az, gx, gy, gz]
	const gx = telemetry[3]
	const gy = telemetry[4]

	samples += 1

	// Decimate serial output to reduce logging overhead while sampling at 100Hz.
	const now = input.runningTime()
	if (now - lastLog >= 20) {
		serial.writeValue("RollRate", gx)
		serial.writeValue("PitchRate", gy)
		lastLog = now
	}

	// Report effective read rate every second.
	if (now - statStart >= 1000) {
		serial.writeValue("SampleHz", samples)
		samples = 0
		statStart = now
	}

	// Target ~100Hz loop period.
	const loopDuration = input.runningTime() - loopStart
	if (loopDuration < 10) {
		basic.pause(10 - loopDuration)
	}
})
