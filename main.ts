/**
 * Drone IMU V3 minimal diagnostic baseline.
 */
//% weight=100 color=#2E7D32 icon="\uf2db" block="Drone IMU V3 MIN 111"
namespace droneIMUV3 {
    const BUILD_SIGNATURE = "V3-MIN-SIG-20260702-I"
    const BUILD_SIGNATURE_CODE = 41011
    let initialized = false

    //% blockId=droneimuv3_init block="initialize IMU"
    //% weight=100
    export function init(): void {
        initialized = true
    }

    //% blockId=droneimuv3_initsimple block="initialize IMU (simple)"
    //% weight=95
    export function initSimple(): void {
        initialized = true
    }

    //% blockHidden=true
    export function initHardware(): void {
        initMPU6050()
    }

    //% blockId=droneimuv3_buildsig block="build signature"
    //% weight=90
    export function buildSignature(): string {
        return BUILD_SIGNATURE
    }

    //% blockId=droneimuv3_buildsigcode block="build signature code"
    //% weight=89
    export function buildSignatureCode(): number {
        return BUILD_SIGNATURE_CODE
    }

    //% blockId=droneimuv3_ping block="ping"
    //% weight=88
    export function ping(): boolean {
        return initialized
    }

    //% blockId=droneimuv3_releaseprobe104 block="release probe 104"
    //% weight=87
    export function releaseProbe104(): number {
        return 104
    }

    //% blockId=droneimuv3_releaseprobe105 block="release probe 105"
    //% weight=86
    export function releaseProbe105(): number {
        return 105
    }

    //% blockId=droneimuv3_releaseprobe106 block="release probe 106"
    //% weight=85
    export function releaseProbe106(): number {
        return 106
    }

    //% blockId=droneimuv3_releaseprobe111 block="release probe 111"
    //% weight=84
    export function releaseProbe111(): number {
        return 111
    }
}
