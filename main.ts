/**
 * Drone IMU V3 minimal diagnostic baseline.
 */
//% weight=100 color=#2E7D32 icon="\uf2db" block="Drone IMU V3 MIN 115"
namespace droneIMUV3 {
    const BUILD_SIGNATURE = "V3-MIN-SIG-20260703-A"
    const BUILD_SIGNATURE_CODE = 41015
    const MPU_ADDR = 0x68
    const REG_PWR_MGMT_1 = 0x6B
    const REG_ACCEL_XOUT_H = 0x3B
    let initialized = false

    function i16be(buf: Buffer, idx: number): number {
        let v = (buf[idx] << 8) | buf[idx + 1]
        if (v & 0x8000) v = v - 65536
        return v
    }

    function readSensorPacketInternal(): Buffer {
        pins.i2cWriteNumber(MPU_ADDR, REG_ACCEL_XOUT_H, NumberFormat.UInt8LE, true)
        return pins.i2cReadBuffer(MPU_ADDR, 14, false)
    }

    //% blockId=droneimuv3_init block="initialize IMU"
    //% weight=100
    export function init(): void {
        pins.i2cWriteNumber(MPU_ADDR, REG_PWR_MGMT_1, NumberFormat.UInt8LE)
        pins.i2cWriteNumber(MPU_ADDR, 0x01, NumberFormat.UInt8LE)
        initialized = true
    }

    //% blockId=droneimuv3_initsimple block="initialize IMU (simple)"
    //% weight=95
    export function initSimple(): void {
        initialized = true
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

    //% blockId=droneimuv3_hwwhoami block="hardware WHO_AM_I"
    //% weight=89
    export function hardwareWhoAmI(): number {
        pins.i2cWriteNumber(MPU_ADDR, 0x75, NumberFormat.UInt8LE, true)
        return pins.i2cReadNumber(MPU_ADDR, NumberFormat.UInt8LE, false)
    }

    //% blockId=droneimuv3_readsensorpacket block="read sensor packet valid"
    //% weight=88
    export function readSensorPacketValid(): boolean {
        const packet = readSensorPacketInternal()
        return !!packet && packet.length === 14
    }

    //% blockId=droneimuv3_read_roll block="read roll rate deg/s"
    //% weight=87
    export function readRollRate(): number {
        const p = readSensorPacketInternal()
        if (!p || p.length < 14) return 0
        return i16be(p, 8) / 65.5
    }

    //% blockId=droneimuv3_read_pitch block="read pitch rate deg/s"
    //% weight=86
    export function readPitchRate(): number {
        const p = readSensorPacketInternal()
        if (!p || p.length < 14) return 0
        return i16be(p, 10) / 65.5
    }

    //% blockId=droneimuv3_read_yaw block="read yaw rate deg/s"
    //% weight=85
    export function readYawRate(): number {
        const p = readSensorPacketInternal()
        if (!p || p.length < 14) return 0
        return i16be(p, 12) / 65.5
    }

    //% blockId=droneimuv3_nativeconst block="native constant"
    //% weight=88
    export function nativeConstant(): number {
        return nativeConst123()
    }

    //% blockId=droneimuv3_ping block="ping"
    //% weight=87
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

    //% blockId=droneimuv3_releaseprobe115 block="release probe 115"
    //% weight=83
    export function releaseProbe115(): number {
        return 115
    }
}
