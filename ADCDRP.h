// see ADCDRP.cpp for usage information
//
// D.R.Patterson 23/10/15

#ifndef ADCDRP_H
#define ADCDRP_H

#include "Arduino.h"

class ADCDRP {
public:
ADCDRP();
float Avrange;
float Afreq;
float Afreqm;
float Aaverage;
float Avrms;
float Asdev;
byte Amax;
byte Amin;
void analyse(float mfreq, float refv, int Abufsize, uint8_t* Adata);
void graph(int Abufsize, uint8_t* Adata);
void graph(int Abufsize, uint8_t* Adata, float refv); // use this call if voltage reference known
String DRPpad(unsigned long myval,int digit,String p);
};

void graphit(int Abufsize, uint8_t* Adata, float refv);

#endif

