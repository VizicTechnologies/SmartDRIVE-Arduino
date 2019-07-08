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
 1.- Read the "Servo Play Secuence.txt" File stored on the microSD card root path
 2.- Convert the 2 read bytes to single int variable
 3.- Scale read values 0-1024 through the setServoChannel() function 
 4.- Send scaled value to the PWM selected channel
 5.- Repeat - End when no more bytes can be read
 
 --HOW TO:
  -load the sketch on arduino board.
  -load "microSD root contents" folder to the microSD card root path with PC
  -connect the SmartDRIVE with the proper wires and insert the microSD card with contents
  -connect a Servo to the PWM selected channel output of SmartDRIVE  
  -power up arduino/reset
  -Arduino LED next to pin 13 will be ON during all the process.  
  -servo will move according to the read values from the microSD card file "servo play secuence.txt"
  -Arduino LED next to pin 13 will shut OFF when process ends, wait until then.  
  
  *Analyze PWM signals in a oscilloscope
  
***The Arduino LED next to pin13 is configured by the SmartDRIVE library to show correct microSD card mounting,
    if the LED is constantly ON: drive is successfuly mounted. If the LED is constantly OFF: drive is unmounted
    and ready to receive commands. If the LED blinks: no microSD card is inserted or an ERROR ocurred while mounting
    the microSD card, will loop forever here***  
********************************************************/

#include <SMARTDRIVE.h>     //include the SMARTDRIVE library!

#define SMARTDRIVEPWMCHANNEL 0

SMARTDRIVE sd;              //create our object called sd


//function that converts ADC values 0-1023 to 50HZ PWM to drive servos
/*********************************************/
void setServoChannel(unsigned char channel, unsigned int value){
  float duty=0;  
  if(value>1023) value=1023; //if value overflows the max to set
  //Standard servos work at 50Hz
  duty=(value*8.5);
  duty/=1023;
  duty+=3;        
  sd.setPWMDuty(channel,duty,50);  //set duty at standard frequency of 50 Hz      
}

/*********************************************/
/*********************************************/
void setup() { //initial setup  
  //Those three functions must always be called for SMARTDRIVE support
  sd.init();              //configure the serial and pinout of arduino board for SMARTDRIVE support
  sd.reset();             //reset the SMARTDRIVE processor
  sd.mountUnmountDrive(); //mount the SMARTDRIVE microSD card - Arduino LED must be constantly ON after this if the microSD card was successfully mounted.
}

/*********************************************/
/*********************************************/
/*********************************************/
/*********************************************/
void loop() { //main loop
    char buffer[2];
    unsigned int bytesRead=0, readValue=0;     
    
    //change master USART baudrate to 500 000bps
    sd.masterBaudChange(500000);
    
    sd.setPWMFreq(50); //set 50 Hz frequency to SmartDRIVE PWMs - ideal frequency to drive standard servos
    
    sd.openFile("Servo Play Secuence.txt",READONLY,WORKSPACE0); //open the file that has the servo movements secuence 
    
    while(1){
      sd.readFile(buffer,2,&bytesRead,WORKSPACE0); //read 2 bytes from the file
      if(bytesRead==0) break; //if no bytes were read

      //convert the 2 read bytes to int 
      ((unsigned char *)&readValue)[1]=buffer[0];
      ((unsigned char *)&readValue)[0]=buffer[1];
     
      //set the Value
      setServoChannel(SMARTDRIVEPWMCHANNEL, readValue);      
      delay(1000); //delay between movements
    }    
    sd.closeFile(WORKSPACE0);
    
    sd.mountUnmountDrive(); //Arduino LED next to pin 13 will shut OFF here
    while(1);
}
