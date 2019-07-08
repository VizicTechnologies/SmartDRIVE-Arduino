/*********************************************************
VIZIC TECHNOLOGIES. COPYRIGHT 2013.
THE DATASHEETS, SOFTWARE AND LIBRARIES ARE PROVIDED "AS IS." 
VIZIC EXPRESSLY DISCLAIM ANY WARRANTY OF ANY KIND, WHETHER 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
OR NONINFRINGEMENT. IN NO EVENT SHALL VIZIC BE LIABLE FOR 
ANY INCIDENTAL, SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES, 
LOST PROFITS OR LOST DATA, HARM TO YOUR EQUIPMENT, COST OF 
PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, 
ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO 
ANY DEFENCE THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION,
OR OTHER SIMILAR COSTS.
*********************************************************/

/********************************************************
 IMPORTANT : This Example is created for the Arduino 1.0 Software IDE
********************************************************/

/********************************************************
 This simple sketch does the next:
 1.- Read the ADC selected channel value
 2.- Scale ADC value through the setServoChannel() function 
 3.- Send scaled value to the PWM selected channel
 4.- Loop
 
 --HOW TO:
  -load the sketch on arduino board.
  -connect the SmartDRIVE with the proper wires
  -connect a pot to the ADC selected channel input of SmartDRIVE
  -connect a Servo to the PWM selected channel output of SmartDRIVE  
  -power up arduino/reset
  -Analyze PWM signals in a oscilloscope
  
***The Arduino LED next to pin13 is configured by the SmartDRIVE library to show correct microSD card mounting,
    if the LED is constantly ON: drive is successfuly mounted. If the LED is constantly OFF: drive is unmounted
    and ready to receive commands. If the LED blinks: no microSD card is inserted or an ERROR ocurred while mounting
    the microSD card, will loop forever here***
********************************************************/

#include <SMARTDRIVE.h>     //include the SMARTDRIVE library!

#define SMARTDRIVEPWMCHANNEL 0
#define SMARTDRIVEADCCHANNEL 0

SMARTDRIVE sd;              //create our object called sd

//function that converts ADC values 0-1023 to 50HZ PWM to drive servos
void setServoChannel(unsigned char channel, unsigned int value){
  float duty=0;  
  if(value>1023) value=1023; //if value overflows the max to set
  //Standard servos work at 50Hz
  duty=(value*8.5);
  duty/=1023;
  duty+=3;        
  sd.setPWMDuty(channel,duty,50);  //set duty at standard frequency of 50 Hz      
}

void setup() { //initial setup  
  //Those three functions must always be called for SMARTDRIVE support
  sd.init();              //configure the serial and pinout of arduino board for SMARTDRIVE support
  sd.reset();             //reset the SMARTDRIVE processor
  sd.mountUnmountDrive(); //mount the SMARTDRIVE microSD card - Arduino LED must be constantly ON after this if the microSD card was successfully mounted.
}

void loop() { //main loop
    unsigned int ADCValue=0;     
    
    sd.masterBaudChange(115200); //set master usart baudrate speed to 115200bps    
    sd.setPWMFreq(50); //set 50 Hz frequency to SmartDRIVE PWMs - ideal frequency to drive standard servos
    
    while(1){
      sd.getADC(SMARTDRIVEADCCHANNEL,&ADCValue); //get SmartDRIVE ADC channel X value (0-1023)
      setServoChannel(SMARTDRIVEPWMCHANNEL, ADCValue);      
      delay(100);
    }
}
