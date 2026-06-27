declare namespace droneIMU {
    //% shim=droneIMU::initMPU6050
    function initMPU6050(): void;

    //% shim=droneIMU::getSensorData
    function getSensorData(): Buffer;

    //% shim=droneIMU::pidConfigure
    function pidConfigure(kp: number, ki: number, kd: number, outMin: number, outMax: number): void;

    //% shim=droneIMU::pidReset
    function pidReset(): void;

    //% shim=droneIMU::pidUpdate
    function pidUpdate(setpoint: number, measurement: number, dtMs: number): number;
}
