/* Programmable Logic Controller Library for the Arduino and Compatibles

Controllino Maxi PLC - Use of default pin names and numbers
Product information: http://controllino.cc

Connections:
Inputs connected to pins A0 - A9, plus interrupts IN0 and IN1
Digital outputs connected to pins D0 to D11
Relay outputs connected to pins R0 to R9

Software and Documentation:
http://www.electronics-micros.com/software-hardware/plclib-arduino/

*/

// Pins A0 - A9 are configured automatically

// Interrupt pins
const int IN0 = 18;
const int IN1 = 19;

const int D0 = 2;
const int D1 = 3;
const int D2 = 4;
const int D3 = 5;
const int D4 = 6;
const int D5 = 7;
const int D6 = 8;
const int D7 = 9;
const int D8 = 10;
const int D9 = 11;
const int D10 = 12;
const int D11 = 13;

const int R0 = 22;
const int R1 = 23;
const int R2 = 24;
const int R3 = 25;
const int R4 = 26;
const int R5 = 27;
const int R6 = 28;
const int R7 = 29;
const int R8 = 30;
const int R9 = 31; 

void customIO() {
  // Input pin directions
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);
  pinMode(A8, INPUT);
  pinMode(A9, INPUT);
  
  // Output pin directions
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  pinMode(D9, OUTPUT);
  pinMode(D10, OUTPUT);
  pinMode(D11, OUTPUT);
  
  // Relay pin directions
  pinMode(R0, OUTPUT);
  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(R3, OUTPUT);
  pinMode(R4, OUTPUT);
  pinMode(R5, OUTPUT);
  pinMode(R6, OUTPUT);
  pinMode(R7, OUTPUT);
  pinMode(R8, OUTPUT);
  pinMode(R9, OUTPUT);
  
}

