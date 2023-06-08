/*
ADCDRP.cpp

D.R.Patterson

  1)  For the analysis of uint8_t data array representing voltages
  2)  Will produce a simple serial graph from the data
		This version displays a horizontal axis line with the
		data item included in the axis - (20 data line interval)
  3)  Pad function to produce strings from unsigned integer with
  fixed width leading character padded format

 8/6/2023
 
Following a call to analyse-
class-identifier.analyse(Measuring frequency in Khz, reference voltage, buffer size, buffer)
  Provides the following ADCDRP class variables:

  Amax    Maximum voltage as 8 bit value
  Amin    Minimum voltage as 8 bit value
  Aaverage  Average voltage value in volts
  Avrange Range between Amax and Amin in volts 
  Avrms   RMS value in Volts based on mid point of Avrange
  Afreq   Estimate of observed frequency in Hertz by DSP
  Afreqm    Estimate of observed frequency in Hertz by transition
        at mid voltage
  Asdev   Standard deviation
Following a call to graph-
class-identifier.graph(buffer size, buffer) or class-identifier.graph(buffer size, buffer, voltage reference)
  Serial output graph :- 
          if the voltage reference is specified the X-scale will show voltage,
          otherwise the X-scale will show the raw data range.
          The left hand Y-axis will show the data value.
          The right hand Y-axis shows the data number next to the horizontal axis.
          In voltage mode, the right hand axis also shows the data point voltage.
          
Following a call to DRPpad-
class-identifier.(unsigned long value, number of digits, pad character as string)
  Padded string value returned with leading pad characters

*/

#include "ADCDRP.h"
#if defined (__SAM3X8E__)
#include <avr/dtostrf.h>	// Due does not have this function by default
#endif

// <<constructor>> setup the default variables
// to indicate no measurement so far

ADCDRP::ADCDRP(){
Avrange = -1;
Afreq = -1;
Afreqm = -1;
Aaverage = -1;
Avrms = -1;
Asdev = -1;
}

// Analyse a buffer to estimate the frequency based on mid voltage transitions

// Analyse a buffer
// mfreq in Khz, refv in volts, Abufsize size of the data array,
// Adata the array pointer

void ADCDRP::analyse(float mfreq, float refv, int Abufsize, uint8_t* Adata){

Afreq = -1;
Afreqm = -1;

int i;
float convert = refv / 255;
float bufsize = float(Abufsize);

// find max, min and average voltages
Amax = 0;
Amin = 255;
Aaverage = 0;
  for (i=0; i < Abufsize; i ++) {
    if (Adata[i] < Amin) Amin = Adata[i];
    if (Adata[i] > Amax) Amax =Adata[i];
  Aaverage += Adata[i];
  }
Avrange = float(Amax-Amin) * convert;
Aaverage = Aaverage / bufsize;
float fmidpoint = float(Amin + Amax) / 2;

/*
calculate rms and standard deviation
For the rms:
  Without user input, estimating the voltage level
  that the wave is oscillating around is a guess.
  The average value could be used as the midpoint voltage:
  float midestimate = Aaverage;
*/

float midestimate = float(Amax + Amin)/2;
Avrms = 0;
Asdev = 0;
float item;
  for (i = 0; i < Abufsize; i ++) {
  item = float(Adata[i]);
  Avrms +=  (item-midestimate) * (item-midestimate);
  Asdev += (Aaverage - item) * (Aaverage - item);
  }
Asdev= sqrt(Asdev / bufsize);
Avrms = sqrt(Avrms / bufsize);

// convert value to voltages
Avrms = Avrms * convert;
Aaverage = Aaverage * convert;

// estimate the observed frequency- independent of wave shape
// 8/6/2023 sacrifice speed in manipulationg long values for accuracy
float sample_freq = mfreq*1000;
float sum_old;		// was long: increased accuracy if float
float sum = 0;		// was long: increased accuracy if float
long thresh = 0;
int lperiod = 0;
byte pd_state = 0;


  for(i=0; i < Abufsize; i++)
  {
    sum_old = sum;
    sum = 0;
    // divide by 256.0 not 256 for float accuracy
    for(int k=0; k < Abufsize-i; k++) sum += (Adata[k]-128)*(Adata[k+i]-128)/256.0;
  
    // Peak Detect State Machine
    if ( (pd_state == 2) && ((sum-sum_old) <= 0) ) 
    {
      lperiod = i;
      pd_state = 3;
    }
    if ( (pd_state == 1) && (sum > thresh) && ((sum-sum_old) > 0) ) pd_state = 2;
    if (i==0) {
      thresh = sum * 0.5;
      pd_state = 1;
    }
  }
// Frequency identified in Hz, default -1
  if(lperiod>0) Afreq = sample_freq/lperiod;

// Calculate estimate "Afreqm" of frequency based on transitions around the midpoint, default -1

int fwaveno = 0;
int ffirstval, flastval;
boolean ffirstone = false;
float faddupperiod = 0;

	for (i = 0; i < (Abufsize - 9); i++) {
		if (Adata[i] < fmidpoint) {
			if (Adata[i + 1] >= fmidpoint) {
			float tot = 0;
				for (int fj = i + 2; fj < (i + 8); fj++) {
          			tot += Adata[fj];
				}
				if (tot/6.0 > fmidpoint) {		// look for average rise 
          				if (ffirstone == false){
          				ffirstval = i + 1;
          				ffirstone = true;        
          				} else {
          				fwaveno = fwaveno + 1;
          				flastval = i + 1;
          				faddupperiod = faddupperiod + flastval - ffirstval;
          				ffirstval = flastval;  
          				}
        			i += 7;
        			}
			}
    		}
	}

  
	if (fwaveno > 0){
  	// mfreq in  Khz - fmyinterval in seconds
  	float fmyinterval = 1 / (mfreq * 1000);
  	Afreqm = fwaveno / (fmyinterval * faddupperiod);   
  	} 
}

void ADCDRP::graph(int Abufsize, uint8_t* Adata){
graphit(Abufsize, Adata, 0);
}

void ADCDRP::graph(int Abufsize, uint8_t* Adata, float refv){
graphit(Abufsize, Adata, refv);
}

void graphit(int Abufsize, uint8_t* Adata, float refv){
String mystring, vline;
char buf[20];
  if(refv>0){
  String tick="++++++++++++";
  String vl=dtostrf(refv/4, 4, 2,buf);
  vline="0+++" + tick + vl + tick;
  vl=dtostrf(refv/2, 4, 2,buf);
  vline=vline + vl + tick;
  vl=dtostrf(refv*0.75, 4, 2,buf);
  vline=vline + vl + tick;
  vl=dtostrf(refv, 4, 2,buf);
  vline=vline + vl + "V ";
  }
 
Serial.println(F("\nData display:\n"));
  for (int i = 0; i < Abufsize; i++){
  mystring = String(Adata[i]);
  int k = mystring.length();
    for(int j=0; j<(3-k); j++) {mystring=" "+mystring;}
  mystring = mystring+" ";   
    if (i%20==0){
      if (refv==0){
      mystring = mystring + "0+++++++++++++++64++++++++++++++128+++++++++++++192+++++++++++++255";
      }else{
      mystring = mystring + vline;
      }
    mystring = mystring + "(" + String(i) + ")";
    unsigned int P = 4;
      for(int j=0; j < Adata[i]; j += 4){P++;}
    mystring.setCharAt(P,42);
    }else{
    	for(int j=0; j < Adata[i]; j += 4){mystring=mystring+" ";}
    mystring = mystring + "*";
    }
    if (refv>0){
      for(int j=mystring.length(); j < 80; j++){mystring=mystring+" ";}
    //String vl=dtostrf(Adata[i]*refv/256, 4, 2,buf);
    String vl=dtostrf(Adata[i]*refv/255, 4, 2,buf);
    mystring=mystring+vl;
    }
  Serial.println(mystring);
  }
  
  if ((Abufsize-1)%20>0){
    if (refv==0){ 
    Serial.print(F("    0+++++++++++++++64++++++++++++++128+++++++++++++192+++++++++++++255 "));
    }else{
    Serial.print("    " + vline);
    }
  Serial.print(F("("));Serial.print(Abufsize-1);Serial.println(F(")"));
  }
Serial.println();
}

String ADCDRP::DRPpad(unsigned long myval,int digit,String p){
// pad a number to digit characters with string p
String mystring = String(myval);
int k = mystring.length();
  for(int j = 0; j < (digit-k); j++){
  mystring = p + mystring;
  }
return mystring;
}
