declare namespace droneIMUV3 {
	//% shim=droneIMUV3::nativeConst123
	function nativeConst123(): number;

	//% shim=droneIMUV3::imuReadReg
	function imuReadReg(addr: number, reg: number): number;

	//% shim=droneIMUV3::imuWriteReg
	function imuWriteReg(addr: number, reg: number, value: number): void;

	//% shim=droneIMUV3::imuReadWord
	function imuReadWord(addr: number, regHigh: number): number;

	//% shim=droneIMUV3::imuReadSensorPacket14
	function imuReadSensorPacket14(addr: number): Buffer;

	//% shim=droneIMUV3::imuRun100HzBurstReadToggleTest
	function imuRun100HzBurstReadToggleTest(addr: number, pinId: number, cycles: number): number;

	//% shim=droneIMUV3::imuRun100HzToggleTest
	function imuRun100HzToggleTest(addr: number, pinId: number, cycles: number, includeRead: boolean): number;
}

declare namespace KalmanFilter {
    //% shim=KalmanFilter::getAngle
    function getAngle(newAngle: number, newRate: number, dt: number): number;

    //% shim=KalmanFilter::setAngle
    function setAngle(angle: number): void;

    //% shim=KalmanFilter::getRate
    function getRate(): number;

    //% shim=KalmanFilter::setQangle
    function setQangle(Q_angle: number): void;
    
    //% shim=KalmanFilter::setQbias
    function setQbias(Q_bias: number): void;

    //% shim=KalmanFilter::setRmeasure
    function setRmeasure(R_measure: number): void;
    
    //% shim=KalmanFilter::getQangle
    function getQangle(): number;

    //% shim=KalmanFilter::getQbias
    function getQbias(): number;
    
    //% shim=KalmanFilter::getRmeasure
    function getRmeasure(): number;
}

declare namespace PidController {
    //% shim=PidController::update
    function update(setpoint: number, measured_value: number, dt: number): number;
    
    //% shim=PidController::setTunings
    function setTunings(Kp: number, Ki: number, Kd: number): void;

    //% shim=PidController::setOutputLimits
    function setOutputLimits(min: number, max: number): void;
    
    //% shim=PidController::reset
    function reset(): void;
}

