//% color="#2E7D32" weight=100 block="Drone IMU"
namespace droneIMU {
    // Minimal baseline for import troubleshooting.
    let initialized = false;

    //% block="initialize IMU (baseline)"
    export function init(): void {
        initialized = true;
    }

    //% blockHidden=true
    export function shimSanityCheck(): boolean {
        return initialized;
    }
}
