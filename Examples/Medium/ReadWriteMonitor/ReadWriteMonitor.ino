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
 10.- umount drive
 11.- end
 
 --HOW TO:
  -load the sketch on arduino board.
  -connect the SmartDRIVE with the proper wires(masterTX, masterRX, reset, 3.3V, GND) and insert microSD card.
  -configure the Hyperterminal to 115200bps baudrate, connected to the secondary USART of SmartDRIVE(SRX, STX) and open session.
  -power up arduino/reset.
  -Arduino LED next to pin 13 will be ON during all the process.  
  -wait until hyperterminal messages show the END word.
  -Arduino LED next to pin 13 will shut OFF when process ends, wait until then.  

  *remove microSD card and search for the file on a PC with the written contents.
  
***The Arduino LED next to pin13 is configured by the SmartDRIVE library to show correct microSD card mounting,
    if the LED is constantly ON: drive is successfuly mounted. If the LED is constantly OFF: drive is unmounted
    and ready to receive commands. If the LED blinks: no microSD card is inserted or an ERROR ocurred while mounting
    the microSD card, will loop forever here***  
********************************************************/

#include <SMARTDRIVE.h>     //include the SMARTDRIVE library!

SMARTDRIVE sd;              //create our object called sd

unsigned char res = OK;     //create the variable that will store all SMARTDRIVE commands responses

char buffer[50]={0}; //working buffer

//Those next are the name of the file that will be created on microSD card root path, and the message to write to that same file.
char fileName[]="test file.txt"; //file name to create and write/read
char message[]="Data Written by the SmartDRIVE Processor"; //message to write on the file created

//function that prints messages in the secondary USART / hyperterminal
/*******************************************/
void printTerminal(char *messages){
  unsigned int i=0;
  while(1){
   if(messages[i]==0x00) break;
   i++;    
  }
  if(i>255) i=255; //Secondary USART can transmit a max of 255 bytes per call
  sd.sendBytesSecUSART(i,messages); //send message to hyper terminal through secondary USART
}

//function that loops forever on any error and loops forever
/*******************************************/
void die(unsigned char response){ //if the response is different then OK loop forever
  char number[5];
  if(response!=OK){
     printTerminal("Error!!! Number:");
     itoa(response,number,10);
     printTerminal(number);      
     printTerminal("\r\n");     
  } 
}

/*********************************************/
/*********************************************/
void setup() { //initial setup  
  //Those three functions must always be called for SMARTDRIVE support
  sd.init();              //configure the serial and pinout of arduino board for SMARTDRIVE support
  sd.reset();             //reset the SMARTDRIVE processor
  sd.mountUnmountDrive(); //mount the SMARTDRIVE microSD card - Arduino LED must be constantly ON after this if the microSD card was successfully mounted.
}

/*******************************************/
/*******************************************/
/*******************************************/
/*******************************************/
void loop() { //main loop
    char auxNum[10]={0};
    unsigned int writtenBytes=0, readbytes=0, i=0;        

    //change master USART baudrate to 115 200bps
    res=sd.masterBaudChange(115200); die(res);
    //change secondary USART baudrate to 115 200bps
    res=sd.secondaryBaudChange(115200); die(res);

    //First try to open the file
    printTerminal("****************************************\r\n");    
    printTerminal("***SmartDRIVE - Arduino test program!***\r\n"); 
    printTerminal("Trying to open File: <");
    printTerminal(fileName);
    printTerminal("> in WRITE ONLY mode.\r\n"); 
    
    res=sd.openFile(fileName, WRITEONLY, WORKSPACE0);  //Try to open a file in write only mode in the workspace block 0    
    if(res!=OK){                       //If the file doesn't Open is because it doesn't exist
      printTerminal("File doesn't exist.\r\n");    
      printTerminal("Creating File...\r\n");    
      res=sd.newFile(fileName);  //Try to create the file 
      res|=sd.openFile(fileName, WRITEONLY, WORKSPACE0);  //Try to open the created file      
      die(res);                        //If any error loop forever
      printTerminal("File successfully created!\r\n");       
    }
    
    //Up to here the file exist and is open  
    printTerminal("File open.\r\n\r\n");
    printTerminal("Writting to File: <");
    printTerminal(message); printTerminal("> \r\n");    
    
    res=sd.writeFile(message, sizeof(message), &writtenBytes, WORKSPACE0); //write to the open file in WORKSPACE0 size of message in bytes and store the successfully written Bytes on writtenBytes variable
    die(res);                         //If any error loop forever
    printTerminal("Successfully written bytes: ");
    ltoa(writtenBytes,auxNum,10);
    printTerminal(auxNum);    
    printTerminal("\r\nsave changes and close file.\r\n\r\n");
    sd.saveFile(WORKSPACE0);          //Save changes in the file contained in WORKSPACE0
    sd.closeFile(WORKSPACE0);         //Close the file --------------------
    
    //Now lets verify contents
    //open again the file in read only mode
    printTerminal("Trying to open File: <");
    printTerminal(fileName);
    printTerminal("> in READ ONLY mode.\r\n");        
    res=sd.openFile(fileName, READONLY, WORKSPACE0);  //Try to open again the file read only mode in the workspace block 0
    die(res);                         //If any error loop forever
    //read the file    
    printTerminal("reading from the file: ");
    ltoa(sizeof(message),auxNum,10);
    printTerminal(auxNum);        
    printTerminal(" bytes.\r\n");
    printTerminal("contents are:\r\n");
    printTerminal("--------------------\r\n");
    res=sd.readFile(buffer, sizeof(message), &readbytes, WORKSPACE0); //read size of message in bytes from the open file in WORKSPACE0 and store the successfully read Bytes on readbytes variable
    die(res);                         //If any error loop forever    
    printTerminal(buffer);  //print contents
    printTerminal("\r\n--------------------\r\n");    
    printTerminal("close file.\r\n\r\n");
    sd.closeFile(WORKSPACE0);         //Close the file
    
    //check contents
    printTerminal("check contents...\r\n");
    for(i=0;i<sizeof(message);i++){      
      if(message[i]!=buffer[i]){ //if contents are different
        printTerminal("FAIL, contents differ, Erasing file.\r\n");
        printTerminal("*******************END******************\r\n");
        sd.eraseDirFile("test file.txt"); //erase the File
        //umount the drive
        sd.mountUnmountDrive();        
        while(1);
      }
    }
    printTerminal("PASS, contents are equal.\r\n");
    printTerminal("*******************END******************\r\n");           
    //umount the drive
    sd.mountUnmountDrive();    //Arduino LED next to pin13 will shut OFF here
    while(1);
}
