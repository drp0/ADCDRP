/*

adcdrp.ino

Padded string function
8 bit data analysis
Serial monitor graph.

D.R.Patterson

15/10/2015

*/

boolean graphwithvoltage = true; // set to false to get without voltage

// drp Voltage analysis library
#include <ADCDRP.h>
ADCDRP mylib; // setup data type to converse with ADCDRP

const float incdeg = 10;
const float angle =  (incdeg * 71) / 4068; // angle increment in radians 

#define BUF_SIZE 320
uint8_t buf[BUF_SIZE];

char junk=' ';

void setup(){
Serial.begin(115200); // start serial for output
  while (Serial.available() > 0) junk = Serial.read();
// setup the array with a sine wave
float rad = 0;
  for (int i=0; i<BUF_SIZE ; i++){
  buf[i] = char(sin(rad)*127 + 127.5);
  rad += angle;
  }
  
Serial.println("Raw data:\n"); 
  for(int i=0; i<BUF_SIZE; i++){
  Serial.print(mylib.DRPpad(i,3," "));
  Serial.print(" , ");
  Serial.println(buf[i]);
  }
  
float vref = 5; // the measuring reference voltage
float vconvert = vref / 255;

Serial.println();
Serial.print(BUF_SIZE); Serial.println(" data point graph:");  

  if(graphwithvoltage){
  mylib.graph(BUF_SIZE, buf, vref);
  }else{
  mylib.graph(BUF_SIZE, buf);
  }
// The frequency is normally determined by the measuring conditions:
float frequency = 270; // This should be in KHZ

mylib.analyse(frequency, vref, BUF_SIZE, buf);
Serial.println("\nBuffer Analysis using ADCDRP\n");
Serial.print("Maximum Value "); Serial.print(mylib.Amax);
Serial.print(", "); Serial.print(mylib.Amax * vconvert); Serial.println(" V");
Serial.print("Minimum Value "); Serial.print(mylib.Amin);
Serial.print(", "); Serial.print(mylib.Amin * vconvert); Serial.println(" V");
Serial.print("Average Voltage "); Serial.print(mylib.Aaverage); Serial.println(" V");
Serial.print("Voltage Range "); Serial.print(mylib.Avrange); Serial.println(" V");
Serial.print("RMS Voltage "); Serial.print(mylib.Avrms); Serial.println(" V");
Serial.print("Frequency Estimate(dsp) "); Serial.print(mylib.Afreq); Serial.println(" Hz");
Serial.print("Frequency Estimate(midpoint) "); Serial.print(mylib.Afreqm); Serial.println(" Hz");
Serial.print("Standard Deviation "); Serial.println(mylib.Asdev);
Serial.flush();
}

void loop(){
}
