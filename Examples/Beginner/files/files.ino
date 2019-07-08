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
 1.- try open the "test file.txt" file in the microSD root path in write only mode 
 2.- if doesn't exist, create the file - if exist overwritte the file
 3.- open again the "test file.txt" file in the microSD root path in write only mode 
 4.- write "Data Written by the SmartDRIVE Processor" to the .txt file
 5.- save contents and close file
 6.- open again the file in read only mode
 7.- read from file to a buffer
 8.- verify data read to be equal to message
 9.- if different erase the created file else keep the file
 10- umount drive
 11.- end
 
 --HOW TO:
  -load the sketch on arduino board.
  -connect the SmartDRIVE with the proper wires and insert microSD card.
  -power up arduino/reset.
  -Arduino LED next to pin 13 will be ON during all the process.  
  -wait until TX and RX leds on arduino completely stop blinking.
  -Arduino LED next to pin 13 will shut OFF when process ends, wait until then. 
  -remove microSD card and search for the file on a PC with the written contents.
  
***The Arduino LED next to pin13 is configured by the SmartDRIVE library to show correct microSD card mounting,
    if the LED is constantly ON: drive is successfuly mounted. If the LED is constantly OFF: drive is unmounted
    and ready to receive commands. If the LED blinks: no microSD card is inserted or an ERROR ocurred while mounting
    the microSD card, will loop forever here***  
********************************************************/

#include <SMARTDRIVE.h>     //include the SMARTDRIVE library!

SMARTDRIVE sd;              //create our object called sd

unsigned char res = OK;     //create the variable that will store all SMARTDRIVE commands responses

char message[41]="Data Written by the SmartDRIVE Processor";

//function that loops forever on error
void die(unsigned char response){ //if the response is different then OK loop forever
  if(response!=OK) while(1);  
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
    char buffer[50]={0};
    unsigned int writtenBytes=0, readbytes=0, i=0;        
    
    //First try to open the file
    res=sd.openFile("test file.txt", WRITEONLY, WORKSPACE0);  //Try to open a file called "testFile.txt" in write only mode in the workspace block 0    
    if(res!=OK){                       //If the file doesn't Open is because it doesn't exist      
      res=sd.newFile("test file.txt");  //Try to create the file 
      res|=sd.openFile("test file.txt", WRITEONLY, WORKSPACE0);  //Try to open the created file      
      die(res);                        //If any error loop forever
    }
    
    //Up to here the file exist and is open
    res=sd.writeFile(message, sizeof(message), &writtenBytes, WORKSPACE0); //write to the open file in WORKSPACE0 size of message in bytes and store the successfully written Bytes on writtenBytes variable
    die(res);                         //If any error loop forever    
    sd.saveFile(WORKSPACE0);          //Save changes in the file contained in WORKSPACE0
    sd.closeFile(WORKSPACE0);         //Close the file --------------------
    
    //Now lets verify contents
    //open again the file in read only mode
    res=sd.openFile("test file.txt", READONLY, WORKSPACE0);  //Try to open again the file read only mode in the workspace block 0
    die(res);                         //If any error loop forever
    //read the file    
    res=sd.readFile(buffer, sizeof(message), &readbytes, WORKSPACE0); //read size of message in bytes from the open file in WORKSPACE0 and store the successfully read Bytes on readbytes variable
    die(res);                         //If any error loop forever        
    sd.closeFile(WORKSPACE0);         //Close the file
    
    //check contents
    for(i=0;i<sizeof(message);i++){      
      if(message[i]!=buffer[i]){ //if contents are different
        sd.eraseDirFile("test file.txt"); //erase the File
        break;
      }
    }
    //umount the drive
    sd.mountUnmountDrive();  //Arduino LED next to pin 13 will shut OFF here.
    while(1);
}
