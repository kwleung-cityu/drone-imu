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

	//% shim=droneIMUV3::imuRun100HzToggleTest
	function imuRun100HzToggleTest(addr: number, pinId: number, cycles: number, includeRead: boolean): number;
}
