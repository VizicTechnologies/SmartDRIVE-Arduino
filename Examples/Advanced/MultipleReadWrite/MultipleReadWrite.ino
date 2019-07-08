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
 1.- Create 5 files, "file0.txt", "file1.txt", etc.
 2.- Open the files in READWRITE mode.
 3.- Write numbers(ascii) to the files, "file0.txt" will write zeros, "file1.txt" will write ones, etc.
 4.- Close the files
 5.- umount drive
 6.- end
 
 --HOW TO:
  -load the sketch on arduino board.
  -connect the SmartDRIVE with the proper wires(masterTX, masterRX, reset, 3.3V, GND) and insert microSD card.
  -configure the Hyperterminal to 115200bps baudrate, connected to the secondary USART of SmartDRIVE(SRX, STX) and open session.
  -power up arduino/reset.
  -Arduino LED next to pin 13 will be ON during all the process.  
  -wait until hyperterminal messages show the END word.
  -Arduino LED next to pin 13 will shut OFF when process ends, wait until then.  

  *remove microSD card and search for the files on a PC with the written contents.
  *the microSD card will contain 5 files with written numbers on each
  
***The Arduino LED next to pin13 is configured by the SmartDRIVE library to show correct microSD card mounting,
    if the LED is constantly ON: drive is successfuly mounted. If the LED is constantly OFF: drive is unmounted
    and ready to receive commands. If the LED blinks: no microSD card is inserted or an ERROR ocurred while mounting
    the microSD card, will loop forever here***  
********************************************************/

#include <SMARTDRIVE.h>     //include the SMARTDRIVE library!

SMARTDRIVE sd;              //create our object called sd

unsigned char res = OK;     //create the variable that will store all SMARTDRIVE commands responses

char fileName[]="file0.txt";//name of the first file to create and write/read

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
    char number[2]={0}; 
    unsigned char workSpace=0;
    unsigned int bytesPerFileToWrite=100, writtenbytes=0, readbytes=0; //bytesPerFileToWrite or bytes to write to each file
    unsigned long filePointerPosition=0, sizeInBytes=0;

    //change master USART baudrate to 500 000bps
    res=sd.masterBaudChange(500000); die(res);
    //change secondary USART baudrate to 115 200bps
    res=sd.secondaryBaudChange(115200); die(res);

    //print
    printTerminal("\r\n************************************************************\r\n");    
    printTerminal("***SmartDRIVE - Arduino Multiple Read-Write test program!***\r\n"); 
    printTerminal("-Create and Open Files in Workspaces 0-4:\r\n");
        
    //Create and open files, will fail if any file already exist on the microSD card root
    res=sd.newFile(fileName);                         die(res); //Create the new file - "file0.txt"
    res=sd.openFile(fileName, READWRITE, WORKSPACE0); die(res); //Open file in READWRITE mode in WORKSPACE0
    fileName[4]='1';                                            //change the 0 for 1 - "file1.txt"
    res=sd.newFile(fileName);                         die(res); //Create the new file    
    res=sd.openFile(fileName, READWRITE, WORKSPACE1); die(res); //Open file in READWRITE mode in WORKSPACE1
    fileName[4]='2';                                            //change the 1 for 2 - "file2.txt"
    res=sd.newFile(fileName);                         die(res); //Create the new file    
    res=sd.openFile(fileName, READWRITE, WORKSPACE2); die(res); //Open file in READWRITE mode in WORKSPACE2
    fileName[4]='3';                                            //change the 2 for 3 - "file3.txt"
    res=sd.newFile(fileName);                         die(res); //Create the new file    
    res=sd.openFile(fileName, READWRITE, WORKSPACE3); die(res); //Open file in READWRITE mode in WORKSPACE3
    fileName[4]='4';                                            //change the 3 for 4 - "file4.txt"
    res=sd.newFile(fileName);                         die(res); //Create the new file    
    res=sd.openFile(fileName, READWRITE, WORKSPACE4); die(res); //Open file in READWRITE mode in WORKSPACE4
       
    //Up to here the files are created and are open  
    printTerminal("-Files are Open... Writting numbers to Files...\r\n");
    
    //write the numbers to the multiple open files: file0.txt - write zeros, file1.txt - write ones, file2.txt - write twos ... etc.
    while(bytesPerFileToWrite--){ //repeat all bytesPerFileToWrite times
      number[0]='0';              //reset number to zero
      
      //Write 1 byte to a file, read the byte and writte the read byte to next file and increase it, repeat process for all files    
      for(workSpace=0; workSpace<5; workSpace++){                         //jump to next file by increasing workSpace each cycle
        //write 1 number(1 byte) to the file in workSpace
        res=sd.writeFile(number, 1, &writtenbytes, workSpace);  die(res); //write to file contained in the workSpace variable
        //get file pointer position and set it/go back it 1 position
        res=sd.getFilePointer(&filePointerPosition,workSpace);  die(res); //get file pointer position
        res=sd.setFilePointer(filePointerPosition-1,workSpace); die(res); //set file pointer position -1 to read last written byte      
        //read 1 byte from the file, the previously written
        res=sd.readFile(number, 1, &readbytes, workSpace);      die(res); //read file - the previously written byte      
        //save changes on file
//        res=sd.saveFile(workSpace);                                       //Save changes to the file - optional, as closing the files save changes.

        //increase the previously read byte or number
        number[0]= number[0] + 1;                                         //increase value
      }
    }
    
    //print
    printTerminal("-Success!, closing Files...\r\n");    
    //Data was successfully written - now close the files
    for(workSpace=0; workSpace<5; workSpace++){
      res=sd.closeFile(workSpace);   die(res);                            //Close the files/work spaces
    }
    
    //print info
    printTerminal("-Successfully written bytes per file: ");
    //Get the size of last file("file4.txt") to know how many bytes were written
    res=sd.getFileSize(fileName,&sizeInBytes); die(res);   //Get the size
    ltoa(sizeInBytes,fileName,10);                         //convert value to ascii and store it in the same fileName buffer
    printTerminal(fileName);                               //print the value   
    
    //print
    printTerminal("\r\n*******************************END**************************\r\n");           
    //umount the drive
    sd.mountUnmountDrive();    //Arduino LED next to pin13 will shut OFF here
    while(1);       
}
