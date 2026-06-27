//% color="#2E7D32" weight=100 icon="\uf142" block="Drone IMU"
namespace droneIMU {

    let nativeAvailable = true;

    //% block="initialize MPU6050 sensor"
    export function init(): void {
        if (!nativeAvailable) return;

        try {
            initMPU6050();
        } catch (e) {
            nativeAvailable = false;
        }
    }

    //% block="IMU shim sanity check"
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

    /**
     * Reads all active orientation rates from the IMU.
     * Returns an array: [AccelX(G), AccelY(G), AccelZ(G), GyroX(deg/s), GyroY(deg/s), GyroZ(deg/s)]
     */
    //% block="read processed IMU values"
    export function readProcessedData(): number[] {
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
}
