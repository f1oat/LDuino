class sysinfo {
public:
	static void wipeRam(); // Wipe RAM between heap and stack with 0x55 pattern
	static int unusedRam();
};