extern unsigned int scanValue;

unsigned int readCoilMB(unsigned int reg) {
  scanValue = mb.Coil(reg) ? 1 : 0;
  return(scanValue);
}

unsigned int readCoilNotMB(unsigned int reg) {
  scanValue = mb.Coil(reg) ? 1 : 0;
  return(scanValue);
}

unsigned int writeCoilMB(unsigned int reg) {
	mb.Coil(reg, scanValue ? 1 : 0);
	return(scanValue);
}

unsigned int writeIstsMB(unsigned int reg) {
  mb.Ists(reg, scanValue ? 1 : 0);
  return(scanValue);
}

unsigned int writeNotMB(unsigned int reg) {
  mb.Ists(reg, scanValue ? 0 : 1);
  return(scanValue);
}

unsigned int writeIregMB(unsigned int reg) {
	mb.Ireg(reg, scanValue);
	return(scanValue);
}