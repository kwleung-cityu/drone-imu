// Baseline test harness for import troubleshooting.
droneIMU.init()
basic.showString(droneIMU.shimSanityCheck() ? "OK" : "ERR")
