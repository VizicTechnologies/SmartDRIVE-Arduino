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
 SMART DRIVE Hyperterminal monitor
 - remember to configure the Hyperterminal to 115200bps baudrate 
 
 --HOW TO:
  -load the sketch on arduino board.
  -load any files and folders to the microSD card root path with PC.  
  -connect the SmartDRIVE with the proper wires(masterTX, masterRX, reset, 3.3V, GND) and insert microSD card with any contents.
  -configure the Hyperterminal to 115200bps baudrate, connected to the secondary USART of SmartDRIVE(SRX, STX) and open session.
  -power up arduino/reset.
  -Arduino LED next to pin 13 will be ON during all the process.  
  -follow hyperterminal messages.
  -unmount drive to END.
  -Arduino LED next to pin 13 will shut OFF when process ends, wait until then.  
  
***The Arduino LED next to pin13 is configured by the SmartDRIVE library to show correct microSD card mounting,
    if the LED is constantly ON: drive is successfuly mounted. If the LED is constantly OFF: drive is unmounted
    and ready to receive commands. If the LED blinks: no microSD card is inserted or an ERROR ocurred while mounting
    the microSD card, will loop forever here***  
********************************************************/

#include <SMARTDRIVE.h>     //include the SMARTDRIVE library!

SMARTDRIVE sd;              //create our object called sd

unsigned char res = OK;     //create the variable that will store all SMARTDRIVE commands responses

char buffer[100]={0};       //working buffer-very limited due to low arduino ram(atmega328 ram)
char fileName[80]={0};      //file name buffer-very limited due to low arduino ram(atmega328 ram)

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
  char number[5];
  if(response!=OK){
     printTerminal("Error!!! Number:");
     itoa(response,number,10);
     printTerminal(number);      
     printTerminal("\r\n");     
  } 
} 

//Shows the main menu
/*******************************************/
void showControlMenu(){
    printTerminal("\r\n**********press key to perform action************\r\n");
    printTerminal("<p> Get Current Dir Path\r\n");
    printTerminal("<l> List Contents on current Directory Path\r\n");    
    printTerminal("<d> Open Dir Name\r\n");
    printTerminal("<f> Open file Name\r\n");      
    printTerminal("<s> File Size\r\n");    
    printTerminal("<e> Erase Dir/file\r\n");    
    printTerminal("<r> File rename\r\n");    
    printTerminal("<c> Copy File\r\n");       
    printTerminal("<u> Unmount drive\r\n");    
    printTerminal("*************************************************\r\n\r\n");        
}

//Get a char from the secondary USART or Hyper terminal
/*******************************************/
char getCharTerminal(){
    unsigned int i=0;
    char buff[2]={0};
    
    while(i==0){ //while we don't get valid characters ( > 0 ) from secondary USART
      sd.getBytesSecUSART(1, &i, buff);
    }   
    return buff[0];
}

//Scan name from secondary usart terminal
/*******************************************/
void scanName(char *name){ 
  unsigned int i=0;
  while(1){
    name[i]=getCharTerminal();
    sd.sendBytesSecUSART(1,name+i);
    if(name[i]==0x0D || name[i]==0x0A){ //if we get an ENTER
      printTerminal("\r\n");
      break;  
    }
    i++;
  }
}
       
//List and print the contents of the current Directory path via Hyperterminal
/*******************************************/
void listContentsCurrentDirPath(){
  char number[10];
  unsigned int Dirs=0,Files=0,i=0;
  //get number of Dirs and Files
  res=sd.getList(&Dirs,&Files); die(res);
  printTerminal("Number of Dirs: "); itoa(Dirs,number,10); printTerminal(number);    
  printTerminal(" - Number of Files: "); itoa(Files,number,10); printTerminal(number); printTerminal("\r\n");      
      
  //list Dir name
  printTerminal("*******************Directories*******************\r\n");
  for(i=0;i<Dirs;i++){
   res=sd.getDirName(i,fileName); die(res); //get item names
   printTerminal(fileName); printTerminal("\r\n"); //print name + new line        
  }
      //list File name
  printTerminal("\r\n**********************Files**********************\r\n");
  for(i=0;i<Files;i++){
   res=sd.getFileName(i,fileName); die(res); //get item names
   printTerminal(fileName); printTerminal("\r\n"); //print name + new line        
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
  unsigned long bytesCopied=0;
  unsigned int writtenBytes=0, bytesRead=0;    
  char number[10];
  
  //change master USART baudrate to 500 000bps
  res=sd.masterBaudChange(500000); die(res);
  //change secondary USART baudrate to 115 200bps
  res=sd.secondaryBaudChange(115200); die(res);
 
  //MAIN MESSAGE
  printTerminal("\r\n*************************************************\r\n");   
  printTerminal("SmartDRIVE - Arduino Hyper terminal file manager!\r\n");
  printTerminal("*************************************************\r\n");    

  while(1){
    showControlMenu();
    
    switch(getCharTerminal()){
      case 'p':
       res=sd.getDirPath(buffer); die(res);       
       printTerminal("Current SmartDRIVE Directory Path:\r\n"); 
       printTerminal(buffer); printTerminal("\r\n");
       delay(1000); //A little delay to let user read the answer       
      break;  
      
      case 'l':
       listContentsCurrentDirPath();
       delay(2000); //A little delay to let user read the answer
      break;     
      
      case 'd':
       printTerminal("Enter Dir Name to open <dir name> + <ENTER>\r\n");      
       scanName(fileName);       
       res=sd.openDir(fileName); 
       if(res){//if any error
         printTerminal("Error opening the Dir...\r\n");
       }else{ 
         res=sd.getDirPath(buffer); die(res);       
         printTerminal("Current SmartDRIVE Directory Path:\r\n"); 
         printTerminal(buffer); printTerminal("\r\n");
       }
       delay(1000); //A little delay to let user read the answer       
      break; 
      
      case 'f':
       printTerminal("Enter File Name to open <file name.xxx> + <ENTER>\r\n");      
       scanName(fileName);
       res=sd.openFile(fileName,READONLY,WORKSPACE0);
       if(res){//if any error
         printTerminal("Error opening the File...\r\n");
       }else{ 
         printTerminal("File Open, reading contents...\r\n");
         //delay(500); //optional delay
         printTerminal("File contents:\r\n");
         printTerminal("-----------------------\r\n");                  
         bytesCopied=0;//variable to store the successfully read bytes
         do{
           sd.readFile(buffer,sizeof(buffer),&bytesRead,WORKSPACE0);
           printTerminal(buffer);
           bytesCopied+=bytesRead;
         }while(bytesRead!=0);
         ltoa(bytesCopied,buffer,10);         
         printTerminal("\r\n-----------------------\r\n");         
         printTerminal(buffer);
         printTerminal(" Successfully Read Bytes!\r\n");         
         sd.closeFile(WORKSPACE0);
       }    
       delay(1000); //A little delay to let user read the answer       
      break;
      
      case 's':
       printTerminal("Enter File Name to Get Size <file name.xxx> + <ENTER>\r\n");      
       scanName(fileName);
       res=sd.getFileSize(fileName, &bytesCopied); //Store size in bytesCopied variable instead of using another(save ram)
       if(res){//if any error
         printTerminal("Error...\r\n");
       }else{ 
         printTerminal("File Size: ");
         ltoa(bytesCopied,buffer,10);
         printTerminal(buffer); printTerminal(" Bytes\r\n");         
       }    
       delay(1000); //A little delay to let user read the answer       
      break;
      
      case 'e':
       printTerminal("Enter Dir/File Name to erase <file name.xxx> + <ENTER>\r\n");    
       scanName(fileName);
       res=sd.eraseDirFile(fileName);
       if(res){//if any error
         printTerminal("Error erasing the Dir/File...\r\n");
         printTerminal("If Directory, must be empty in order to erase it.\r\n");         
       }else{ 
         printTerminal("Dir/File Erased!\r\n");
       }    
       delay(1000); //A little delay to let user read the answer       
      break;

      case 'r':
       printTerminal("Enter File Name to rename <file name.xxx> + <ENTER>\r\n");     
       scanName(fileName);
       printTerminal("Enter New File Name <file name.xxx> + <ENTER>\r\n");   
       scanName(buffer);           
       //file rename
       res=sd.renameMoveDirFile(fileName,buffer);
       if(res){//if any error
         printTerminal("Error Renaming the File...\r\n");
       }else{ 
         printTerminal(buffer);                                         
         printTerminal("Successfully Renamed File!\r\n");
       }    
       delay(1000); //A little delay to let user read the answer       
      break;
      
      case 'c':
       printTerminal("Enter File Name to copy <file name.xxx> + <ENTER>\r\n");      
       scanName(fileName);
       printTerminal("Enter New File Name of the copy <file name.xxx> + <ENTER>\r\n"); 
       scanName(buffer);         
        //perform copy
       res=sd.copyFile(fileName,buffer,&bytesCopied);
       ltoa(bytesCopied,buffer,10);
       if(res){//if any error
         printTerminal("Error copying the File...\r\n");
       }else{ 
         printTerminal(buffer);                                         
         printTerminal(" Successfully Copied Bytes!\r\n");
       }    
       delay(1000); //A little delay to let user read the answer       
      break;
      
      case 'u':
        printTerminal("**Drive unmounted - END**\r\n");        
        sd.mountUnmountDrive(); //Arduino LED next to pin13 will shut OFF here
        while(1);
      break;
      
      default:
       printTerminal("invalid key!\r\n");
      break;             
    }
  }    
}
