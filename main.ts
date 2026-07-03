/**
 * Drone IMU V3 minimal diagnostic baseline.
 */
//% weight=100 color=#2E7D32 icon="\uf2db" block="Drone IMU V3 MIN 117"
namespace droneIMUV3 {
    const BUILD_SIGNATURE = "V3-MIN-SIG-20260703-C"
    const BUILD_SIGNATURE_CODE = 41017
    const MPU_ADDR = 0x68
    const REG_PWR_MGMT_1 = 0x6B
    const REG_GYRO_CONFIG = 0x1B
    const REG_ACCEL_XOUT_H = 0x3B
    let initialized = false
    let lastPacket = pins.createBuffer(14)
    let hasLastPacket = false

    function i16be(buf: Buffer, idx: number): number {
        let v = (buf[idx] << 8) | buf[idx + 1]
        if (v & 0x8000) v = v - 65536
        return v
    }

    function readSensorPacketInternal(): Buffer {
        pins.i2cWriteNumber(MPU_ADDR, REG_ACCEL_XOUT_H, NumberFormat.UInt8LE, true)
        return pins.i2cReadBuffer(MPU_ADDR, 14, false)
    }

    function writeReg(reg: number, value: number): void {
        const b = pins.createBuffer(2)
        b[0] = reg
        b[1] = value
        pins.i2cWriteBuffer(MPU_ADDR, b, false)
    }

    function updateSnapshot(): boolean {
        const p = readSensorPacketInternal()
        if (!p || p.length < 14) {
            hasLastPacket = false
            return false
        }

        lastPacket = p
        hasLastPacket = true
        return true
    }

    //% blockId=droneimuv3_init block="initialize IMU"
    //% weight=100
    export function init(): void {
        writeReg(REG_PWR_MGMT_1, 0x01)
        writeReg(REG_GYRO_CONFIG, 0x08)
        basic.pause(10)
        initialized = true
        hasLastPacket = false
    }

    //% blockId=droneimuv3_initsimple block="initialize IMU (simple)"
    //% weight=95
    export function initSimple(): void {
        initialized = true
        hasLastPacket = false
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

    //% blockId=droneimuv3_whoami block="WHO_AM_I"
    //% weight=89
    export function whoAmI(): number {
        return hardwareWhoAmI()
    }

    //% blockId=droneimuv3_readsensorpacket block="read sensor packet valid"
    //% weight=88
    export function readSensorPacketValid(): boolean {
        return updateSnapshot()
    }

    //% blockId=droneimuv3_refresh_snapshot block="refresh sensor snapshot"
    //% weight=88
    export function refreshSensorSnapshot(): boolean {
        return updateSnapshot()
    }

    //% blockId=droneimuv3_read_roll block="read roll rate deg/s"
    //% weight=87
    export function readRollRate(): number {
        if (!hasLastPacket && !updateSnapshot()) return 0
        return i16be(lastPacket, 8) / 65.5
    }

    //% blockId=droneimuv3_read_pitch block="read pitch rate deg/s"
    //% weight=86
    export function readPitchRate(): number {
        if (!hasLastPacket && !updateSnapshot()) return 0
        return i16be(lastPacket, 10) / 65.5
    }

    //% blockId=droneimuv3_read_yaw block="read yaw rate deg/s"
    //% weight=85
    export function readYawRate(): number {
        if (!hasLastPacket && !updateSnapshot()) return 0
        return i16be(lastPacket, 12) / 65.5
    }

    //% blockId=droneimuv3_nativeconst block="native constant"
    //% weight=88
    export function nativeConstant(): number {
        try {
            return nativeConst123()
        } catch (e) {
            // Simulator has no native shim; return sentinel instead of crashing runtime.
            return -1
        }
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

    //% blockId=droneimuv3_releaseprobe117 block="release probe 117"
    //% weight=83
    export function releaseProbe117(): number {
        return 117
    }
}
