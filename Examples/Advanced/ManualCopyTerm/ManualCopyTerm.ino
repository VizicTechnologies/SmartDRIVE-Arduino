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
 1.- find all files on the microSD card root path
 2.- open one by one the files
 3.- read contents of the files
 4.- create new files(copies) and write contents in them
 5.- umount drive
 6.- end
 
 --HOW TO:
  -load the sketch on arduino board.
  -load the "microSD root contents" folder to the microSD Card root path with a PC.
  -connect the SmartDRIVE with the proper wires and insert the microSD card with the contents.
  -open hyperterminal-configured to 115200bps 1stop / no parity - and connect it to SmartDRIVE secondary USART(STX, SRX).
  -power up arduino/reset.
  -Arduino LED next to pin 13 will be ON during all the process.  
  -watch execution on hyperterminal.
  -Arduino LED next to pin 13 will shut OFF when process ends, wait until then.
  
  *remove microSD card and search for the files on a PC with the copied contents.
  
***The Arduino LED next to pin13 is configured by the SmartDRIVE library to show correct microSD card mounting,
    if the LED is constantly ON: drive is successfuly mounted. If the LED is constantly OFF: drive is unmounted
    and ready to receive commands. If the LED blinks: no microSD card is inserted or an ERROR ocurred while mounting
    the microSD card, will loop forever here***  
********************************************************/

#include <SMARTDRIVE.h>     //include the SMARTDRIVE library!

SMARTDRIVE sd;              //create our object called sd

unsigned char res = OK;     //create the variable that will store all SMARTDRIVE commands responses-can be removed if no validation is needed

//function that prints messages in the secondary USART / hyperterminal
/*******************************************/
void printTerminal(char *messages){
  unsigned int i=0;
  while(1){
   if(messages[i]==0x00) break;
   i++;    
  }
  if(i>255) i=255; //secondary Usart can transmit a max of 255 per call  
  sd.sendBytesSecUSART(i,messages); //send message to hyper terminal through secondary USART
}

//function that loops forever on any error and loops forever
/*******************************************/
void die(unsigned char response){ //if the response is different then OK loop forever
  char number[10];
  if(response!=OK){
     printTerminal("Error!!! Number: ");
     itoa(response,number,10);
     printTerminal(number);         
     printTerminal(" -loop forever @");     
     while(1);     
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
    char buffer[250]={0}; //working buffer
    char fileName[100]={0};//buffer to store the file names
    unsigned int writtenbytes=0,readbytes=0,foundFiles=0,filesCounter=0;        
    unsigned long bytesCopied=0;
    
    //change master USART baudrate to 500 000bps
    res=sd.masterBaudChange(500000); die(res);
    //change secondary USART baudrate to 115 200bps
    res=sd.secondaryBaudChange(115200); die(res);
    
    //MAIN MESSAGE
    printTerminal("\r\n*************************************************\r\n");   
    printTerminal("*SmartDRIVE - Arduino Manual File Copy Example!**\r\n");
    printTerminal("*************************************************\r\n");        
    
    sd.getList(0,&foundFiles);//get only the number of files on current path
    printTerminal("Number of Files Found: "); itoa(foundFiles,buffer,10); printTerminal(buffer); printTerminal("\r\n"); 
  
    while(foundFiles){
      //First get name of the file item
      res=sd.getFileName(filesCounter++,fileName); die(res);  //advance filesCounter pointer ++
      //this next instruction avoids files to be re-copied again     
      if(fileName[0]=='c' && fileName[1]=='o' && fileName[2]=='p' && fileName[3] == 'y') continue; //if fileName is a file that has been already copied
             
      //Try to open the file to copy in workspace0 - read only
      res=sd.openFile(fileName, READONLY, WORKSPACE0); die(res);      
      //create the new file name
      strcpy(buffer, "copy item - ");   //add the "copy item - "
      strcat(buffer, fileName);         //add the original file name	
      //create the new file
      res=sd.newFile(buffer);           //create the new file "copy item - xxxxx.xxx"      
      if(res==ALREADY_EXIST) continue;  //file already exists/already copied - jump to next file
      else die(res); 

      //print
      printTerminal("\r\n*********************\r\n");              
      printTerminal("Name of File to copy: ");      
      printTerminal(fileName); printTerminal("\r\n"); //print name + new line  
      
      //open the new created file in workspace1 - write only
      res=sd.openFile(buffer, WRITEONLY, WORKSPACE1); die(res);      
      printTerminal("Name of New File:     ");      
      printTerminal(buffer); printTerminal("\r\n"); //print name + new line               

      //perform the copy
      printTerminal("Copying File...   ");         
      bytesCopied=0;
      writtenbytes=0;
      while(1){
	bytesCopied += writtenbytes; //add bytes copied counter the successfully written bytes        
        res=sd.readFile(buffer, sizeof(buffer), &readbytes, WORKSPACE0); die(res); //read data from source file
	if(readbytes==0) break;  /*enf of file*/
        res=sd.writeFile(buffer, readbytes, &writtenbytes, WORKSPACE1); die(res); //write data to destination file
	if(writtenbytes < readbytes) break;   /*disk full*/
      }
      //print
      printTerminal(" ...Success!  ");          
      ltoa(bytesCopied,buffer,10);
      printTerminal(buffer); printTerminal(" bytes copied\r\n");

      //close the files
      sd.closeFile(WORKSPACE0);      //close source file
      sd.closeFile(WORKSPACE1);      //save and close destination file
      foundFiles--;                  //decrease number of files to copy variable
    }
    
    printTerminal("\r\n***Program END!***\r\n");     
    //umount the drive
    sd.mountUnmountDrive();          //Arduino LED next to pin13 will shut OFF here
    while(1);
}
