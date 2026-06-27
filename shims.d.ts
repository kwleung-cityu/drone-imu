declare namespace droneIMU {
    //% shim=droneIMU::initMPU6050
    function initMPU6050(): void;

    //% shim=droneIMU::getSensorData
    function getSensorData(): Buffer;
}
