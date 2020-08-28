// ADCmeasure.ino

// Takes 320 high speed 8 bit data samples from ADC 0
// Connect pin 3 to ADC 0
// Configure float vref to match Arduino board voltage
// Open the Serial monitor at 115200 baud

// pin 3 is configured for pwm square wave output at ~ 3921.16 Hz
// http://arduinoinfo.mywikis.net/wiki/Arduino-PWM-Frequency
// Configure fastpwm as required for arduino board type
 
// The data is saved in bufa
// The ADCDRP library plots and analyses the data

// Measurement frequency can be altered with prescalar

#include <ADCDRP.h>                         // drp Voltage analysis library
ADCDRP mylib;                               // setup data type to converse with ADCDRP

// Defines for clearing register bits
#ifndef mycbi
#define mycbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
// Defines for setting register bits
#ifndef mysbi
#define mysbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// Square wave on port 3 3921.16 Hz
#ifndef fastpwm                             // comment out as required
#define fastpwm (TCCR3B = TCCR3B & B11111000 | B00000010)    // Mega
//#define fastpwm (TCCR2B = TCCR2B & B11111000 | B00000010)    // Uno
#endif

const byte testpin = 3;                     // pwm pin

float vref = 5.0;                           // set to processor voltage
                                            // measure if accuracy is required!
#define BUF_SIZE 320
volatile uint8_t bufa[BUF_SIZE];            // buffer for 8 bit ADC data

float vconvert = vref / 255.0;
const byte sensor = 0;

unsigned long t1;
volatile unsigned long t2;
volatile boolean isready;
volatile uint16_t bufcount;

byte prescalar = 8;                         // 4, 8, 16

void setup() {
Serial.begin(115200); delay(250);
pinMode(testpin,OUTPUT);
analogWrite(testpin, 0);
fastpwm;
setCapture();
}

void setCapture(){
bufcount = 0;
analogWrite(testpin, 127);                  // pwm on
isready = false;
delay(10);                                  // ensures pwm is active

// Setup continuous reading of the adc port 'sensor' using an interrupt
cli();                                      // disable interrupts
ADCSRA = 0;                                 // clear ADCSRA and ADCSRB registers
ADCSRB = 0;
ADMUX  = sensor;                            // set up continuous sampling of analog pin sensor 
mysbi(ADMUX, REFS0);                        // set reference voltage
mysbi(ADMUX, ADLAR);                        // left align the ADC value- so we can read highest 8 bits from ADCH register

  if (prescalar ==  4) ADCSRA |= (1 << ADPS1);
  if (prescalar ==  8) ADCSRA |= (1 << ADPS1) | (1 << ADPS0);
  if (prescalar == 16) ADCSRA |= (1 << ADPS2); 
                                            // prescalar 4: ~230 KHz (Mega)
                                            // prescalar 8: ~130 KHz, prescalar 16: ~70 KHz
mysbi(ADCSRA, ADATE);                       // enable auto trigger
mysbi(ADCSRA, ADIE);                        // Activate ADC Conversion Complete Interrupt
mysbi(ADCSRA, ADEN);                        // enable ADC. Disable with mycbi(ADCSRA, ADEN);
mysbi(ADCSRA, ADSC);                        // start ADC measurements on interrupt
sei();
t1 = micros();                                      // enable interrupts
}

void adcRepeat(){
bufcount = 0;
analogWrite(testpin, 127);                  // pwm on
isready = false;
delay(10);
cli();
mysbi(ADCSRA, ADEN);                        // enable ADC. Disable with mycbi(ADCSRA, ADEN);
mysbi(ADCSRA, ADSC);                        // start ADC measurements on interrupt
sei();
t1 = micros();                              // enable interrupts
}

void loop() {
  if (isready) {
  analogWrite(testpin, 0);                  // pwm off
  mylib.graph(BUF_SIZE, bufa, vref);        // graph with voltages
  float myperiod = t2 - t1;                 // measurement period uS
  float myfrequency = float(BUF_SIZE + 1) * 1000.0 / myperiod;  // measurement frequency in KHz

  Serial.print("\nPrescalar "); Serial.print(prescalar);
  Serial.print(", Sample Period "); Serial.print(myperiod, 0); Serial.print(" uS");
  Serial.print(" Sample Frequency "); Serial.print(myfrequency, 2); Serial.println(" KHz,");
  mylib.analyse(myfrequency, vref, BUF_SIZE, bufa);
  Serial.println("\nBuffer Analysis using ADCDRP\n");
  Serial.print("Maximum Value "); Serial.print(mylib.Amax);
  Serial.print(", "); Serial.print(mylib.Amax * vconvert); Serial.println(" V");
  Serial.print("Minimum Value "); Serial.print(mylib.Amin);
  Serial.print(", "); Serial.print(mylib.Amin * vconvert); Serial.println(" V");
  Serial.print("Average Voltage "); Serial.print(mylib.Aaverage); Serial.println(" V");
  Serial.print("Voltage Range "); Serial.print(mylib.Avrange); Serial.println(" V");
  Serial.print("RMS Voltage "); Serial.print(mylib.Avrms); Serial.println(" V");
  Serial.print("Frequency Estimate(midpoint) "); Serial.print(mylib.Afreqm, 1); Serial.println(" Hz");
  Serial.print("Frequency Estimate(dsp) "); Serial.print(mylib.Afreq, 1); Serial.println(" Hz");
  Serial.print("Standard Deviation "); Serial.println(mylib.Asdev);
  Serial.println();
  Serial.flush();
  delay(10000);
  adcRepeat();
  }
}

ISR(ADC_vect) {                             // grab measurement
bufa[bufcount] = ADCH;
bufcount = bufcount + 1;
  if (bufcount == BUF_SIZE) {
  t2 = micros();
  mycbi(ADCSRA, ADEN);                      // stop the interrupt
  isready = true;
  }
}
