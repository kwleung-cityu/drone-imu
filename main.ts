/**
 * Drone IMU troubleshooting baseline blocks.
 */
//% weight=100 color=#2E7D32 icon="\uf2db" block="Drone IMU"
namespace droneIMU {
    let initialized = false;

    //% blockId=droneimu_init block="initialize IMU"
    //% group="Basics"
    //% weight=100
    export function init(): void {
        initialized = true;
    }

    //% blockId=droneimu_ready block="IMU initialized?"
    //% group="Basics"
    //% weight=90
    export function isReady(): boolean {
        return initialized;
    }

    //% blockHidden=true
    export function shimSanityCheck(): boolean {
        return initialized;
    }
}
