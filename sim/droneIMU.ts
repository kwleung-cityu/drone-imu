namespace pxsim.droneIMU {
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
}
