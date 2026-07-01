//% color="#2E7D32" weight=100 icon="\uf142" block="Drone IMU"
namespace droneIMU {

    let nativeAvailable = true;
    let simKp = 0;
    let simKi = 0;
    let simKd = 0;
    let simOutMin = -1024;
    let simOutMax = 1024;
    let simIntegral = 0;
    let simPrevError = 0;
    let simHasPrev = false;

    //% block="initialize MPU6050 sensor"
    export function init(): void {
        if (!nativeAvailable) return;

        try {
            initMPU6050();
        } catch (e) {
            nativeAvailable = false;
        }
    }

    //% blockHidden=true
    export function shimSanityCheck(): boolean {
        if (!nativeAvailable) return false;

        try {
            initMPU6050();
            let buf = getSensorData();
            return !!buf && buf.length === 14;
        } catch (e) {
            nativeAvailable = false;
            return false;
        }
    }


    // Internal helper used by scalar-safe public APIs.
    function readProcessedDataCore(): number[] {
        if (!nativeAvailable) return [0, 0, 0, 0, 0, 0];

        let buf = pins.createBuffer(0);
        try {
            buf = getSensorData();
        } catch (e) {
            nativeAvailable = false;
            return [0, 0, 0, 0, 0, 0];
        }
        if (!buf || buf.length < 14) return [0, 0, 0, 0, 0, 0];

        // Helper function to combine 8-bit registers into signed 16-bit integers
        function parse16(highIdx: number): number {
            let val = (buf[highIdx] << 8) | buf[highIdx + 1];
            return val & 0x8000 ? val - 65536 : val;
        }

        // Convert raw integer chunks into scaled telemetry units
        let ax = parse16(0) / 8192.0;  // FS_SEL = 1 (+/- 4g) -> 8192 LSB/g
        let ay = parse16(2) / 8192.0;
        let az = parse16(4) / 8192.0;

        let gx = parse16(8) / 65.5;    // FS_SEL = 1 (+/- 500 deg/s) -> 65.5 LSB/(deg/s)
        let gy = parse16(10) / 65.5;
        let gz = parse16(12) / 65.5;

        return [ax, ay, az, gx, gy, gz];
    }

    //% blockHidden=true
    export function readRollRate(): number {
        return readProcessedDataCore()[3];
    }


    //% blockHidden=true
    export function readPitchRate(): number {
        return readProcessedDataCore()[4];
    }


    //% blockHidden=true
    export function readYawRate(): number {
        return readProcessedDataCore()[5];
    }

    //% blockHidden=true
    export function readAccelX(): number {
        return readProcessedDataCore()[0];
    }

    //% blockHidden=true
    export function readAccelY(): number {
        return readProcessedDataCore()[1];
    }

    //% blockHidden=true
    export function readAccelZ(): number {
        return readProcessedDataCore()[2];
    }


    //% blockHidden=true
    export function configurePID(kp: number, ki: number, kd: number, outMin: number, outMax: number): void {
        if (!nativeAvailable) {
            simKp = kp;
            simKi = ki;
            simKd = kd;
            if (outMin <= outMax) {
                simOutMin = outMin;
                simOutMax = outMax;
            } else {
                simOutMin = outMax;
                simOutMax = outMin;
            }
            return;
        }

        try {
            pidConfigure(kp, ki, kd, outMin, outMax);
        } catch (e) {
            nativeAvailable = false;
            configurePID(kp, ki, kd, outMin, outMax);
        }
    }


    //% blockHidden=true
    export function resetPID(): void {
        if (!nativeAvailable) {
            simIntegral = 0;
            simPrevError = 0;
            simHasPrev = false;
            return;
        }

        try {
            pidReset();
        } catch (e) {
            nativeAvailable = false;
            resetPID();
        }
    }


    //% blockHidden=true
    export function updatePID(setpoint: number, measurement: number, dtMs: number): number {
        if (!nativeAvailable) {
            let dt = dtMs / 1000;
            if (dt <= 0.000001) dt = 0.01;

            let err = setpoint - measurement;
            simIntegral += err * dt;
            if (simIntegral < simOutMin) simIntegral = simOutMin;
            if (simIntegral > simOutMax) simIntegral = simOutMax;

            let d = 0;
            if (simHasPrev) d = (err - simPrevError) / dt;

            let out = simKp * err + simKi * simIntegral + simKd * d;
            if (out < simOutMin) out = simOutMin;
            if (out > simOutMax) out = simOutMax;

            simPrevError = err;
            simHasPrev = true;
            return out;
        }

        try {
            return pidUpdate(setpoint, measurement, dtMs);
        } catch (e) {
            nativeAvailable = false;
            return updatePID(setpoint, measurement, dtMs);
        }
    }

}
