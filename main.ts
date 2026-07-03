/**
 * Drone IMU V3 minimal diagnostic baseline.
 */
//% weight=100 color=#2E7D32 icon="\uf2db" block="Drone IMU V3 MIN 125"
namespace droneIMUV3 {
    const BUILD_SIGNATURE = "V3-MIN-SIG-20260703-K"
    const BUILD_SIGNATURE_CODE = 41025
    const MPU_ADDR_68 = 0x68
    const MPU_ADDR_69 = 0x69
    const REG_PWR_MGMT_1 = 0x6B
    const REG_GYRO_CONFIG = 0x1B
    const REG_ACCEL_XOUT_H = 0x3B
    const REG_WHO_AM_I = 0x75
    const REG_GYRO_XOUT_H = 0x43
    const REG_GYRO_YOUT_H = 0x45
    const REG_GYRO_ZOUT_H = 0x47
    let activeAddr = MPU_ADDR_68
    let initialized = false
    let lastPacket = pins.createBuffer(14)
    let hasLastPacket = false
    let gyroBiasX = 0
    let gyroBiasY = 0
    let gyroBiasZ = 0

    function i16be(buf: Buffer, idx: number): number {
        let v = (buf[idx] << 8) | buf[idx + 1]
        if (v & 0x8000) v = v - 65536
        return v
    }

    function readReg(addr: number, reg: number): number {
        pins.i2cWriteNumber(addr, reg, NumberFormat.UInt8LE, true)
        return pins.i2cReadNumber(addr, NumberFormat.UInt8LE, false)
    }

    function readWord(addr: number, regHigh: number): number {
        const hi = readReg(addr, regHigh)
        const lo = readReg(addr, regHigh + 1)
        let v = (hi << 8) | lo
        if (v & 0x8000) v = v - 65536
        return v
    }

    function readSensorPacketAt(addr: number): Buffer {
        pins.i2cWriteNumber(addr, REG_ACCEL_XOUT_H, NumberFormat.UInt8LE, true)
        return pins.i2cReadBuffer(addr, 14, false)
    }

    function writeReg(addr: number, reg: number, value: number): void {
        const b = pins.createBuffer(2)
        b[0] = reg
        b[1] = value
        pins.i2cWriteBuffer(addr, b, false)
    }

    function configureAt(addr: number): void {
        writeReg(addr, REG_PWR_MGMT_1, 0x01)
        basic.pause(5)
        writeReg(addr, REG_GYRO_CONFIG, 0x08)
        basic.pause(5)
    }

    function packetLooksAlive(p: Buffer): boolean {
        let allZero = true
        let allFF = true
        for (let i = 0; i < p.length; i++) {
            const b = p[i]
            if (b != 0) allZero = false
            if (b != 255) allFF = false
            if (!allZero && !allFF) return true
        }
        return false
    }

    function whoAmIWithRetry(addr: number): number {
        let id = 0
        for (let i = 0; i < 5; i++) {
            id = readReg(addr, REG_WHO_AM_I)
            if (id != 0 && id != 255) return id
            basic.pause(20)
        }
        return id
    }

    function selectActiveAddress(): number {
        // Prefer known-good default address; do not auto-switch on ambiguous WHO_AM_I.
        activeAddr = MPU_ADDR_68
        return activeAddr
    }

    function updateSnapshot(): boolean {
        const p = readSensorPacketAt(activeAddr)
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
        selectActiveAddress()

        configureAt(activeAddr)
        updateSnapshot()

        basic.pause(30)
        initialized = true
        hasLastPacket = false
        gyroBiasX = 0
        gyroBiasY = 0
        gyroBiasZ = 0
    }

    //% blockId=droneimuv3_initsimple block="initialize IMU (simple)"
    //% weight=95
    export function initSimple(): void {
        initialized = true
        hasLastPacket = false
        gyroBiasX = 0
        gyroBiasY = 0
        gyroBiasZ = 0
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
        return whoAmIWithRetry(activeAddr)
    }

    //% blockId=droneimuv3_whoami68 block="WHO_AM_I at 0x68"
    //% weight=89
    export function whoAmIAt68(): number {
        return whoAmIWithRetry(MPU_ADDR_68)
    }

    //% blockId=droneimuv3_whoami69 block="WHO_AM_I at 0x69"
    //% weight=89
    export function whoAmIAt69(): number {
        return whoAmIWithRetry(MPU_ADDR_69)
    }

    //% blockId=droneimuv3_pwr68 block="PWR_MGMT_1 at 0x68"
    //% weight=89
    export function pwrMgmtAt68(): number {
        return readReg(MPU_ADDR_68, REG_PWR_MGMT_1)
    }

    //% blockId=droneimuv3_pwr69 block="PWR_MGMT_1 at 0x69"
    //% weight=89
    export function pwrMgmtAt69(): number {
        return readReg(MPU_ADDR_69, REG_PWR_MGMT_1)
    }

    //% blockId=droneimuv3_gyrocfg68 block="GYRO_CONFIG at 0x68"
    //% weight=89
    export function gyroConfigAt68(): number {
        return readReg(MPU_ADDR_68, REG_GYRO_CONFIG)
    }

    //% blockId=droneimuv3_gyrocfg69 block="GYRO_CONFIG at 0x69"
    //% weight=89
    export function gyroConfigAt69(): number {
        return readReg(MPU_ADDR_69, REG_GYRO_CONFIG)
    }

    //% blockId=droneimuv3_activeaddr block="active I2C address"
    //% weight=89
    export function activeI2cAddress(): number {
        return activeAddr
    }

    //% blockId=droneimuv3_setaddr block="set I2C address %addr"
    //% addr.min=104 addr.max=105 addr.defl=104
    //% weight=89
    export function setI2cAddress(addr: number): number {
        if (addr == 105) activeAddr = MPU_ADDR_69
        else activeAddr = MPU_ADDR_68
        return activeAddr
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
        return (readRawGyroX() - gyroBiasX) / 65.5
    }

    //% blockId=droneimuv3_read_pitch block="read pitch rate deg/s"
    //% weight=86
    export function readPitchRate(): number {
        return (readRawGyroY() - gyroBiasY) / 65.5
    }

    //% blockId=droneimuv3_read_yaw block="read yaw rate deg/s"
    //% weight=85
    export function readYawRate(): number {
        return (readRawGyroZ() - gyroBiasZ) / 65.5
    }

    //% blockId=droneimuv3_raw_gx block="read raw gyro X"
    //% weight=85
    export function readRawGyroX(): number {
        return readWord(activeAddr, REG_GYRO_XOUT_H)
    }

    //% blockId=droneimuv3_raw_gy block="read raw gyro Y"
    //% weight=85
    export function readRawGyroY(): number {
        return readWord(activeAddr, REG_GYRO_YOUT_H)
    }

    //% blockId=droneimuv3_raw_gz block="read raw gyro Z"
    //% weight=85
    export function readRawGyroZ(): number {
        return readWord(activeAddr, REG_GYRO_ZOUT_H)
    }

    //% blockId=droneimuv3_calib_gyro_bias block="calibrate gyro bias samples %samples"
    //% samples.min=16 samples.max=256 samples.defl=64
    //% weight=84
    export function calibrateGyroBias(samples: number): boolean {
        if (samples < 1) samples = 64

        let sx = 0
        let sy = 0
        let sz = 0
        let n = 0

        for (let i = 0; i < samples; i++) {
            if (updateSnapshot()) {
                sx += i16be(lastPacket, 8)
                sy += i16be(lastPacket, 10)
                sz += i16be(lastPacket, 12)
                n += 1
            }
            basic.pause(2)
        }

        if (n < 1) return false

        gyroBiasX = Math.round(sx / n)
        gyroBiasY = Math.round(sy / n)
        gyroBiasZ = Math.round(sz / n)
        return true
    }

    //% blockId=droneimuv3_reset_gyro_bias block="reset gyro bias"
    //% weight=84
    export function resetGyroBias(): void {
        gyroBiasX = 0
        gyroBiasY = 0
        gyroBiasZ = 0
    }

    //% blockId=droneimuv3_nativeconst block="native constant"
    //% weight=88
    export function nativeConstant(): number {
        // Keep runtime stable in simulator and device while native path is optional.
        return 123
    }

    //% blockId=droneimuv3_ping block="ping"
    //% weight=87
    export function ping(): boolean {
        return initialized
    }

    // Python-friendly aliases to avoid naming ambiguity across targets.
    export function native_constant(): number {
        return nativeConstant()
    }

    export function hardware_who_am_i(): number {
        return hardwareWhoAmI()
    }

    export function who_am_i(): number {
        return whoAmI()
    }

    export function who_am_i_at_68(): number {
        return whoAmIAt68()
    }

    export function who_am_i_at_69(): number {
        return whoAmIAt69()
    }

    export function active_i2c_address(): number {
        return activeI2cAddress()
    }

    export function set_i2c_address(addr: number): number {
        return setI2cAddress(addr)
    }

    export function pwr_mgmt_at_68(): number {
        return pwrMgmtAt68()
    }

    export function pwr_mgmt_at_69(): number {
        return pwrMgmtAt69()
    }

    export function gyro_config_at_68(): number {
        return gyroConfigAt68()
    }

    export function gyro_config_at_69(): number {
        return gyroConfigAt69()
    }

    export function refresh_sensor_snapshot(): boolean {
        return refreshSensorSnapshot()
    }

    export function read_roll_rate(): number {
        return readRollRate()
    }

    export function read_pitch_rate(): number {
        return readPitchRate()
    }

    export function read_yaw_rate(): number {
        return readYawRate()
    }

    export function read_raw_gyro_x(): number {
        return readRawGyroX()
    }

    export function read_raw_gyro_y(): number {
        return readRawGyroY()
    }

    export function read_raw_gyro_z(): number {
        return readRawGyroZ()
    }

    export function calibrate_gyro_bias(samples: number): boolean {
        return calibrateGyroBias(samples)
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

    //% blockId=droneimuv3_releaseprobe125 block="release probe 125"
    //% weight=83
    export function releaseProbe125(): number {
        return 125
    }
}
