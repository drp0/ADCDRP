# ADCDRP
An Arduino library for 8 bit ADC data analysis.  
The library is primarily intended for the analysis of an eight bit data array created by recording ADC values.  

It includes two Serial monitor graph options:  
  - Raw data is graphed ( 0 - 255).  
  - If the analogue voltage reference is supplied, the data is graphed as voltages ( 0 - voltage reference).  

Data analysis requires the data sampling frequency, voltage reference, array size and the name of the array.  
The following data analysis values are available:  
  - Voltage range (Avrange)  
  - Maximum data value (Amax)  
  - Minimum data value (Amin)  
  - Average voltage (Aaverage)  
  - RMS voltage (Avrms)  
  - Standard deviation (Asdev)  
  - Signal Frequency (Afreqm)  Uses the signal mid point crossing point to determine the frequency  
  - Signal frequency (Afreq)   Uses digital signal processing techniques   

A useful integer padding function is also available ( mylib.DRPpad(I, n, " ") )  
  - This returns a string using integer I, with a width of n characters, left padded with the character within the quotes.  

Example ADCdrp produces an eight bit sin wave with 320 data values.  

Example ADCmeasure reads 320 values from ADC0 at a frequency of ~144 Khz (Mega).
  - A square wave PWM signal of ~3921.16 Hz can be enabled on port 3  
  - If port 3 is connected to ADC0, the square wave will be analysed  
