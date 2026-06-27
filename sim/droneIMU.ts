namespace pxsim.droneIMU {
    let kp = 0
    let ki = 0
    let kd = 0
    let outMin = -1024
    let outMax = 1024
    let integral = 0
    let prevError = 0
    let hasPrev = false

    export function initMPU6050(): void {
        // No hardware in simulator.
    }

    export function getSensorData(): Buffer {
        const data = control.createBuffer(14);

        // Return a valid 14-byte frame in simulator: all zeros.
        // [axH, axL, ayH, ayL, azH, azL, tempH, tempL, gxH, gxL, gyH, gyL, gzH, gzL]
        for (let i = 0; i < 14; i++) {
            data[i] = 0;
        }

        return data;
    }

    export function pidConfigure(newKp: number, newKi: number, newKd: number, newOutMin: number, newOutMax: number): void {
        kp = newKp
        ki = newKi
        kd = newKd
        if (newOutMin <= newOutMax) {
            outMin = newOutMin
            outMax = newOutMax
        } else {
            outMin = newOutMax
            outMax = newOutMin
        }
    }

    export function pidReset(): void {
        integral = 0
        prevError = 0
        hasPrev = false
    }

    export function pidUpdate(setpoint: number, measurement: number, dtMs: number): number {
        let dt = dtMs / 1000
        if (dt <= 0.000001) dt = 0.01

        const err = setpoint - measurement
        integral += err * dt
        if (integral < outMin) integral = outMin
        if (integral > outMax) integral = outMax

        let d = 0
        if (hasPrev) d = (err - prevError) / dt

        let output = kp * err + ki * integral + kd * d
        if (output < outMin) output = outMin
        if (output > outMax) output = outMax

        prevError = err
        hasPrev = true
        return output
    }
}
