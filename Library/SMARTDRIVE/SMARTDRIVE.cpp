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
 IMPORTANT : This library is created for the Arduino 1.0 Software IDE
********************************************************/

/***The Arduino LED next to pin13 is configured by the SmartDRIVE library to show correct microSD card mounting,
    if the LED is constantly ON: drive is successfuly mounted. If the LED is constantly OFF: drive is unmounted
    and ready to receive commands. If the LED blinks: no microSD card is inserted or an ERROR ocurred while mounting
    the microSD card, will loop forever here***/

#include <avr/interrupt.h>
#include <Arduino.h> 

#include "SMARTDRIVE.h"

/****************************************************************/
//Arduino exclusive Functions
/****************************************************************/
SMARTDRIVE::SMARTDRIVE(){
	init();
}

void SMARTDRIVE::init(){      //configure the arduino board for SMARTDRIVE board
	analogReference(EXTERNAL); 
	Serial.begin(9600);	
	pinMode(TX,OUTPUT);
	pinMode(RX,INPUT);
	pinMode(RESET, OUTPUT);
	digitalWrite(RESET, HIGH);       //Set the RESET pin to 5v to avoid reset 	
	//This pin will be configured in order to visually let us know if the SmartDRIVE successfully mounted the microSD card	
	pinMode(ARDUINOUNOLED,OUTPUT);   //Configure the arduino uno LED connected to pin 13
	digitalWrite(ARDUINOUNOLED,LOW); //turn OFF LED		
}
 
void SMARTDRIVE::reset(){          //Reset the SMARTDRIVE board
	digitalWrite(RESET, LOW);    // set the pin to GND to reset 
	delay(500);
	digitalWrite(RESET, HIGH);   // set the pin to 5v to end reset
	delay(500);	
}


/****************************************************************/
//Master Functions - Those functions are available all the time
/****************************************************************/
//This next function visual assist the user:
//If LED is constantly ON : SmartDRIVE is successfully mounted and ready
//If LED is constantly OFF: SmartDRIVE in successfully unmounted
//If LED is blinking      : An error ocurred in the Disk while mounting/unmounting - will loop forever
unsigned char SMARTDRIVE::mountUnmountDrive(){       //mounts or unmounts  the SMARTDRIVE disk
  static unsigned char mountState=0;
  unsigned char aux=0;
    
  Serial.write('U');  
  while(Serial.available() < 2);  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'

  /**************************/  
  //Check reply
  if(aux!=OK){ //if mountUnmountDrive function returns any error (!=OK) then toggle LED forever
    while(1){  //toggle the pin forever showing ERROR
      digitalWrite(ARDUINOUNOLED,LOW); 
      delay(300); 
      digitalWrite(ARDUINOUNOLED,HIGH); 
      delay(300); 	 
    }  
  }
  /**************************/
  //else - No ERROR ocurred
  mountState=!mountState;                 //Toggle mount State
  digitalWrite(ARDUINOUNOLED,mountState); //Set LED the new State
  return aux;                             //Return SD file execution status  
}

unsigned char SMARTDRIVE::sleepInOut(){       //Send SMARTDRIVE to sleep in/Out mode
  unsigned char aux=0;
  
  Serial.write('Z');   
  while(Serial.available() < 2);
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status  
}


/****************************************************************/
//General Functions - Those functions are ONLY available when the SMARTDRIVE disk has been mounted
/****************************************************************/
// SMARTDRIVE DEFAULT/INITIAL MASTER USART BAUD RATE: 9600bps
unsigned char SMARTDRIVE::masterBaudChange(unsigned long val){       //Change baud rate of arduino and SMARTDRIVE board
  unsigned char aux;
 
  switch(val){
	case 9600:
	  aux=0;
	break;
	case 19200:
	  aux=1;
	break;
	case 57600:
	  aux=2;
	break;
	case 115200:
	  aux=3;
	break;
	case 256000:
	  aux=4;
	break;	
	case 500000:
	  aux=5;
	break;
	case 1000000:
	  aux=6;
	break;
	case 2000000:
	  aux=7;
	break;
	default:
	  return 'F';
	break;
  } 
  Serial.write('X');
  Serial.write('M');             //Master USART
  Serial.write(aux);
  while(Serial.available() == 0);  
  aux=Serial.read();
  if(aux=='O'){
	delay(150);
	Serial.begin(val);
	delay(200);
	//up from here the USART is at the new baudrate
	while(Serial.available() < 2);
	aux= Serial.read();          //Get SD file execution status
	Serial.read();               //Discard 'O' or 'F'
	return aux;                  //Return SD file execution status  	
  }else{
	Serial.read();
	return aux;
  }
}

// SMARTDRIVE DEFAULT/INITIAL SECONDARY USART BAUD RATE: 9600bps
unsigned char SMARTDRIVE::secondaryBaudChange(unsigned long val){       //Change baud rate of arduino and SMARTDRIVE board
  unsigned char aux;
 
  switch(val){
	case 9600:
	  aux=0;
	break;
	case 19200:
	  aux=1;
	break;
	case 57600:
	  aux=2;
	break;
	case 115200:
	  aux=3;
	break;
	case 256000:
	  aux=4;
	break;	
	case 500000:
	  aux=5;
	break;
	case 1000000:
	  aux=6;
	break;
	case 2000000:
	  aux=7;
	break;
	default:
	  return 'F';
	break;
  } 
  Serial.write('X');
  Serial.write('S');             //Secondary USART
  Serial.write(aux);
  while(Serial.available() < 2);
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status   
}

//Obtain received bytes from the Secondary USART, MAX 255 per call
//this function is recommended to be called at a maximum master USART baud rate of 1000000bps, as arduino lost bytes on bigger speeds
unsigned char SMARTDRIVE::getBytesSecUSART(unsigned int BTO, unsigned int *SOB, char buffer[]){       //Obtain/Read received bytes from Secondary USART
  unsigned int sr=0;
  unsigned aux=0;
  
  if(BTO>255){
	*SOB=0;             //store Succesfully Obtained Bytes - Zero
	return INVALID_PARAMETER;
  }
  
  Serial.write('B');    
  Serial.write(BTO>>8); //Send Bytes To Obtain parameter
  Serial.write(BTO);  
  
  Serial.readBytes(buffer,BTO);

  while(Serial.available() < 4);  
  ((unsigned char*) &sr)[1]=Serial.read();
  ((unsigned char*) &sr)[0]=Serial.read();    

  *SOB = sr;                   //store Succesfully Obtained Bytes - Valid data
  //When SOB<BTO, means that the Secondary Usart Buffer only contained SOB bytes, the remaining BTO-SOB bytes are received as 0x00(Zeros)
  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status     
}

//Send bytes to the Secondary USART, MAX 255 per call
unsigned char SMARTDRIVE::sendBytesSecUSART(unsigned int BTS, char buffer[]){       //Transmit/Send bytes to Secondary USART at its current baud rate speed
  unsigned aux=0;
  unsigned int i=0;
  
  if(BTS>255) return INVALID_PARAMETER;
  
  Serial.write('K');    
  Serial.write(BTS>>8); //Send Bytes To Transmit parameter
  Serial.write(BTS);  
  for(i=0;i<BTS;i++){   //Send the Bytes
	Serial.write(buffer[i]);
  }
 
  while(Serial.available() < 2);
  aux= Serial.read();          //Get SD file execution status - if different than OK, no Bytes were transmitted
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status  
}

unsigned char SMARTDRIVE::getADC(unsigned char channel, unsigned int *value){       //Gets ADC current value of received channel 0-1023
  unsigned int val=0;
  unsigned aux=0;
  
  if(channel>3) return INVALID_PARAMETER;  //If invalid channel number
  
  Serial.write('A');           
  Serial.write('A');  //ADC
  Serial.write(channel);
  
  while(Serial.available() < 4);  
  ((unsigned char*) &val)[1]=Serial.read();
  ((unsigned char*) &val)[0]=Serial.read();    

  *value = val;                //Store Obtained ADC lecture 0-1024
  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status    
}

unsigned char SMARTDRIVE::setPWMFreq(unsigned long freq){       //Set the PWM frequency of all channels
  unsigned aux=0;
  
  if(freq<30 || freq>1000000) return INVALID_PARAMETER;  //If invalid frequency value
  
  Serial.write('A');           
  Serial.write('P');  //PWM
  Serial.write('F');  //Frequency  

  freq = (2000000/freq); //scale value and convert to int
  Serial.write(freq>>8); //Send Scaled Value
  Serial.write(freq);  
  
  while(Serial.available() < 2);
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status  
}

unsigned char SMARTDRIVE::setPWMDuty(unsigned char channel, float duty, unsigned long freq){       //Set a PWM channel Duty Cycle from a received frequency
  unsigned aux=0;
  float val=2000000;
  
  if(duty>100 || channel>2) return INVALID_PARAMETER;  //If invalid duty value or If invalid channel number
  
  Serial.write('A');           
  Serial.write('P');     //PWM
  Serial.write('D');     //Duty Cycle
  
  val/=freq;
  duty=(val*duty)/100;   //scale value get corresponding duty cycle with the given frequency parameter
  Serial.write(((unsigned int)duty)>>8); //Send Scaled Value
  Serial.write(((unsigned int)duty));  
  Serial.write(channel);
  
  while(Serial.available() < 2);
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status  
}

/****************************************************************/
//SD FAT management Functions - Those functions are ONLY available when the SMARTDRIVE disk has been mounted
//Those next SDF - SD Functions return file execution status instead of ACK 'O' or NAK 'F'
/****************************************************************/
unsigned char SMARTDRIVE::getList(unsigned int *numOfDirs, unsigned int *numOfFiles){ //get number of dirs and files
  unsigned char  aux=0;
  unsigned int dirs=0, files=0;

  Serial.write('L');           //List/count dirs and files
  while(Serial.available() < 6);
  ((unsigned char *) &dirs)[1]= Serial.read();       //Get Upper part
  ((unsigned char *) &dirs)[0]= Serial.read();       //Get Lower part
  ((unsigned char *) &files)[1]= Serial.read();      //Get Upper part
  ((unsigned char *) &files)[0]= Serial.read();      //Get Lower part  

  *numOfDirs=dirs;  
  *numOfFiles=files;       
  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status
}

//this function is recommended to be called at a maximum master USART baud rate of 1000000bps, as arduino lost bytes on bigger speeds
unsigned char SMARTDRIVE::getDirName(unsigned int itemNumber, char name[]){ //searches for the "itemNumber" on the SD current folder and updates the buffer with the Dir name ended with NULL character
  unsigned char  aux=0;

  Serial.write('G');           //Get name of given item file number
  Serial.write('D');           //Directory
  Serial.write(itemNumber>>8); //Send Upper part of itemNumber
  Serial.write(itemNumber);    //Send Lower part of itemNumber
  
  aux=Serial.readBytesUntil(0x00,name,255); //if we find NULL character, means end of name
  name[aux]=0x00;                           //Add the null character
    
  while(Serial.available() < 2);  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status
}

//this function is recommended to be called at a maximum master USART baud rate of 1000000bps, as arduino lost bytes on bigger speeds
unsigned char SMARTDRIVE::getFileName(unsigned int itemNumber, char name[]){ //searches for the "itemNumber" on the SD current folder and updates the buffer with the File name ended with NULL character
  unsigned char  aux=0;

  Serial.write('G');           //Get name of given item file number
  Serial.write('F');           //File  
  Serial.write(itemNumber>>8); //Send Upper part of itemNumber
  Serial.write(itemNumber);    //Send Lower part of itemNumber

  aux=Serial.readBytesUntil(0x00,name,255); //if we find NULL character, means end of name  
  name[aux]=0x00;                           //Add the null character
    
  while(Serial.available() < 2);  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status
 }
 
//this function is recommended to be called at a maximum master USART baud rate of 1000000bps, as arduino lost bytes on bigger speeds
unsigned char SMARTDRIVE::getDirPath(char path[]){ //obtains current dir path and stores on path[] buffer
  unsigned char  aux=0;

  Serial.write('H');           //Get current Dir Path
  
  aux=Serial.readBytesUntil(0x00,path,255); //if we find NULL character, means end of path
  path[aux]=0x00;                           //Add the null character
    
  while(Serial.available() < 2);  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status
}

unsigned char SMARTDRIVE::newDir(char name[]){ //create a new Directory, fails if already exist
  unsigned char aux = 0;
  unsigned int counter=0;
  
  Serial.write('N');           //New
  Serial.write('D');           //Directory/Folder  
  while(1){
  	Serial.write(name[counter]);
  	if(name[counter]==0x00){
		break;
  	}	
  	counter++;
  }
  
  while(Serial.available() < 2);  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status
}

unsigned char SMARTDRIVE::newFile(char name[]){ //create a new File, fails if already exist
  unsigned char aux = 0;
  unsigned int counter=0;
  
  Serial.write('N');           //New
  Serial.write('F');           //File 
  while(1){
  	Serial.write(name[counter]);
  	if(name[counter]==0x00){
		break;
  	}	
  	counter++;
  }
  
  while(Serial.available() < 2);  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status
}

unsigned char SMARTDRIVE::openDir(char name[]){ //opens an existing Dir
  unsigned char aux = 0;
  unsigned int counter=0;

  Serial.write('D');           //Open Dir
  while(1){
	Serial.write(name[counter]);
	if(name[counter]==0x00){
		break;
	}	
	counter++;
  }
	
  while(Serial.available() < 2);  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status  
}

unsigned char SMARTDRIVE::openFile(char name[], unsigned char mode, unsigned char objectWorkspaceNo){ //opens an existing file in READONLY, WRITEONLY or READWRITE mode on the received object # workspace
  unsigned char aux = 0;
  unsigned int counter=0;
  
  Serial.write('O');               //Open file
  Serial.write(objectWorkspaceNo); //object workspace number to allocate open file 0-4
  Serial.write(mode);              //Mode - READONLY,WRITEONLY,READWRITE
  while(1){
	Serial.write(name[counter]);
	if(name[counter]==0x00){
		break;
	}	
	counter++;
  }
	
  while(Serial.available() < 2);  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status  
}

//save changes to a file and close it.
unsigned char SMARTDRIVE::closeFile(unsigned char objectWorkspaceNo){ //close and save file object # workspace
  unsigned char aux = 0;

  Serial.write('C');           //Close File
  Serial.write(objectWorkspaceNo); //object workspace number to close 0-4  
  
  while(Serial.available() < 2);
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status
}

//Use this function to save changes/contents to a file when writting, without closing it.
unsigned char SMARTDRIVE::saveFile(unsigned char objectWorkspaceNo){ //sync/save file object # workspace
  unsigned char aux = 0;
  
  Serial.write('S');           //Save/Sync file - Save changes on file
  Serial.write(objectWorkspaceNo); //object workspace number to save changes 0-4  
  
  while(Serial.available() < 2);
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status 
}

unsigned char SMARTDRIVE::setFilePointer(unsigned long pointerPosition, unsigned char objectWorkspaceNo){ // set/move file pointer of file object # workspace
  unsigned char aux = 0;
  
  Serial.write('P');           //Pointer position
  Serial.write(objectWorkspaceNo); //object workspace number to move pointer 0-4  
  Serial.write('S');           //Set  
  Serial.write(pointerPosition>>24);
  Serial.write(pointerPosition>>16);
  Serial.write(pointerPosition>>8);
  Serial.write(pointerPosition);
   
  while(Serial.available() < 2);
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status 
}	

unsigned char SMARTDRIVE::getFilePointer(unsigned long *pointerPosition, unsigned char objectWorkspaceNo){ // get file pointer of file object # workspace
  unsigned char aux = 0;
  unsigned long pos = 0;

  Serial.write('P');           //Pointer position
  Serial.write(objectWorkspaceNo); //object workspace number to get pointer 0-4  
  Serial.write('G');           //Get
 
  while(Serial.available() < 6);  
  ((unsigned char *) &pos)[3]=Serial.read();
  ((unsigned char *) &pos)[2]=Serial.read();
  ((unsigned char *) &pos)[1]=Serial.read();  
  ((unsigned char *) &pos)[0]=Serial.read(); 

  *pointerPosition =pos;
  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status 
}
	
//this function is recommended to be called at a maximum master USART baud rate of 1000000bps, as arduino lost bytes on bigger speeds
unsigned char SMARTDRIVE::readFile(char buffer[], unsigned int BTR, unsigned int *SRB, unsigned char objectWorkspaceNo){ //Bytes to Read, Succesfully Read Bytes, file object # to read bytes from
  unsigned char aux = 0;
  unsigned int sr=0;

  Serial.write('R');           //Read file
  Serial.write(objectWorkspaceNo); //object workspace number to read 0-4    
  Serial.write(BTR>>8);
  Serial.write(BTR);  
  
  Serial.readBytes(buffer,BTR); //Get BTR bytes
  
  while(Serial.available() < 4);  
  ((unsigned char*) &sr)[1]=Serial.read();
  ((unsigned char*) &sr)[0]=Serial.read();    

  *SRB = sr;                   //store succesfully read bytes
  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status 
}	

unsigned char SMARTDRIVE::writeFile(char buffer[], unsigned int BTW, unsigned int *SWB,  unsigned char objectWorkspaceNo){ //Bytes to Write, Succesfully Written Bytes, file object # to write bytes
  unsigned char aux = 0;
  unsigned int x=0, sw=0;

  if(BTW>512){
	*SWB=0;                   //Return 0 successfully written bytes
	return INVALID_PARAMETER; //512 bytes is the max to write in a single call
  }
   
  Serial.write('W');           //Write file
  Serial.write(objectWorkspaceNo); //object workspace number to write bytes 0-4    
  Serial.write(BTW>>8);
  Serial.write(BTW);    
  for(x=0;x<BTW;x++){
	Serial.write(buffer[x]);
  }

  while(Serial.available() < 4);  
  ((unsigned char*) &sw)[1]=Serial.read();
  ((unsigned char*) &sw)[0]=Serial.read();  

  *SWB = sw;                   //store succesfully written bytes   
  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status 
}

unsigned char SMARTDRIVE::testFileError(unsigned char objectWorkspaceNo){  //test for an error on file # workspace
  unsigned char aux=0;
  
  Serial.write('Q');           //Test 
  Serial.write(objectWorkspaceNo); //object workspace number to write bytes 0-4    
  Serial.write('R');           //Test Error  

  while(Serial.available() < 2);
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status  
}

unsigned char SMARTDRIVE::testFileEnd(unsigned char objectWorkspaceNo){  //test for an error on file # workspace
  unsigned char aux=0;
  
  Serial.write('Q');           //Test 
  Serial.write(objectWorkspaceNo); //object workspace number to write bytes 0-4    
  Serial.write('E');           //Test End of File

  while(Serial.available() < 2);
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status  
}

unsigned char SMARTDRIVE::truncateFile(unsigned char objectWorkspaceNo){  //truncates the file size to the current file read/write pointer of the file # workspace
  unsigned char aux=0;
  
  Serial.write('V');           //Truncate
  Serial.write(objectWorkspaceNo); //object workspace number 0-4 to truncate on current pointerPosition

  while(Serial.available() < 2);
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status  
}

unsigned char SMARTDRIVE::eraseDirFile(char name[]){ //Erases an existing Dir or File
  unsigned char aux = 0;
  unsigned int counter=0;
  
  Serial.write('E');           //Erase Dir File
  Serial.write('O');           //Unlock Erase Protection
  while(1){
	Serial.write(name[counter]);
	if(name[counter]==0x00){
		break;
	}	
	counter++;
  }

  while(Serial.available() < 2);  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status  
}

unsigned char SMARTDRIVE::setFileTimeDate(char time[], char date[], char name[]){ //Set Time and Date to an existing File
  unsigned char aux = 0;
  unsigned int counter=0;
  
  Serial.write('T');           //Time/Date
  Serial.write('S');           //Set
  Serial.write(time[0]);	     //hour
  Serial.write(time[1]);	     //min
  Serial.write(time[2]);		 //sec
  Serial.write(date[0]);	     //day
  Serial.write(date[1]);	     //month
  Serial.write(date[2]);		 //year upper
  Serial.write(date[3]);		 //year lower	
  while(1){
	Serial.write(name[counter]);
	if(name[counter]==0x00){
		break;
	}	
	counter++;
  }

  while(Serial.available() < 2);  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status  
}

unsigned char SMARTDRIVE::getFileTimeDate(char time[], char date[], char name[]){ //Get Time and Date to an existing File
  unsigned char aux = 0;
  unsigned int counter=0;
  
  Serial.write('T');           //Time/Date
  Serial.write('G');           //Get
  while(1){
	Serial.write(name[counter]);
	if(name[counter]==0x00){
		break;
	}	
	counter++;
  }	

  while(Serial.available() < 9); 	
  time[0]= Serial.read();	     //hour
  time[1]= Serial.read();	     //min
  time[2]= Serial.read();		 //sec
  date[0]= Serial.read();	     //day
  date[1]= Serial.read();	     //month
  date[2]= Serial.read();		 //year upper
  date[3]= Serial.read();		 //year lower	
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status  
}

unsigned char SMARTDRIVE::getFileSize(char name[], unsigned long *fileSize){ //Get Size of an existing File
  unsigned char aux = 0;
  unsigned int counter=0;
  unsigned long size=0;
  
  Serial.write('I');           //Info
  Serial.write('S');           //Size
  while(1){
	Serial.write(name[counter]);
	if(name[counter]==0x00){
		break;
	}	
	counter++;
  }	

  while(Serial.available() < 6); 	
  ((unsigned char *) &size)[3]=Serial.read();
  ((unsigned char *) &size)[2]=Serial.read();
  ((unsigned char *) &size)[1]=Serial.read();  
  ((unsigned char *) &size)[0]=Serial.read(); 
	
  *fileSize=size;
  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status  
}

unsigned char SMARTDRIVE::renameMoveDirFile(char oldName[], char newName[]){ //renames or moves an existing Dir or File
  unsigned char aux = 0;
  unsigned int counter=0;
  
  Serial.write('M');           //Rename / Move
  while(1){
	Serial.write(oldName[counter]);
	if(oldName[counter]==0x00){
		break;
	}	
	counter++;
  }
  counter=0;
  while(1){
	Serial.write(newName[counter]);
	if(newName[counter]==0x00){
		break;
	}	
	counter++;
  }	

  while(Serial.available() < 2);
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status  
}

unsigned char SMARTDRIVE::copyFile(char currentFileName[], char newFileName[], unsigned long *bytesCopied){  //copies an existing File
  unsigned char aux = 0;
  unsigned int counter=0;
  unsigned long bytesC=0;
  
  Serial.write('Y');           //Copy File
  while(1){
	Serial.write(currentFileName[counter]);
	if(currentFileName[counter]==0x00){
		break;
	}	
	counter++;
  }
  counter=0;
  while(1){
	Serial.write(newFileName[counter]);
	if(newFileName[counter]==0x00){
		break;
	}	
	counter++;
  }	

  while(Serial.available() < 6);
  ((unsigned char *) &bytesC)[3]=Serial.read();
  ((unsigned char *) &bytesC)[2]=Serial.read();
  ((unsigned char *) &bytesC)[1]=Serial.read();  
  ((unsigned char *) &bytesC)[0]=Serial.read(); 
  
  *bytesCopied=bytesC;  
  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status  
}

unsigned char SMARTDRIVE::getFreeTotalSpace(unsigned long *freeSpace,unsigned long *totalSpace){ //Get free and total space in bytes of the microSD card
  unsigned char aux = 0;
  unsigned long fSpace=0, tSpace=0;
  
  Serial.write('F');           //Free/Total space

  while(Serial.available() < 10); 	
  ((unsigned char *) &fSpace)[3]=Serial.read();
  ((unsigned char *) &fSpace)[2]=Serial.read();
  ((unsigned char *) &fSpace)[1]=Serial.read();  
  ((unsigned char *) &fSpace)[0]=Serial.read(); 
  ((unsigned char *) &tSpace)[3]=Serial.read();
  ((unsigned char *) &tSpace)[2]=Serial.read();
  ((unsigned char *) &tSpace)[1]=Serial.read();  
  ((unsigned char *) &tSpace)[0]=Serial.read(); 
  
  *freeSpace=fSpace;
  *totalSpace=tSpace;  
  
  aux= Serial.read();          //Get SD file execution status
  Serial.read();               //Discard 'O' or 'F'
  return aux;                  //Return SD file execution status  
}
