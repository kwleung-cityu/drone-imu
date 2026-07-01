//% color="#2E7D32" weight=100 icon="\uf2db" block="Drone IMU"
namespace droneIMU {
    let initialized = false;

    //% block="initialize IMU"
    export function init(): void {
        initialized = true;
    }

    //% blockHidden=true
    export function shimSanityCheck(): boolean {
        return initialized;
    }
}
