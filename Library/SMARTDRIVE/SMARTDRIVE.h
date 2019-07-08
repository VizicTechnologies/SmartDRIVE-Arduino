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
	
#ifndef SMARTDRIVE_h
#define SMARTDRIVE_h

#include <inttypes.h>

//General PIN definitions for SmartDrive connections
#define RX             0
#define TX             1
#define RESET          2
#define ARDUINOUNOLED 13

//General definitions
#define OFF 0
#define ON  1
#define GND 0
#define VCC 1

//File access definitions
#define READONLY     0x01
#define WRITEONLY    0x02
#define READWRITE    0x03
#define BEGINNING    0
#define ALLCONTENTS  0

//SMARTDRIVE Command Execution responses definitions
#define OK                  0
#define DISK_ERROR          1
#define INT_ERROR           2
#define NOT_READY           3
#define NO_FILE             4
#define NO_PATH             5
#define INVALID_NAME        6
#define DENIED              7
#define ALREADY_EXIST       8
#define INVALID_OBJECT      9
#define WRITE_PROTECTED    10
#define INVALID_DRIVE      11
#define NOT_ENABLED        12
#define NO_FILE_SYSTEM     13
#define MAKE_FILE_ABORTED  14
#define TIMEOUT            15
#define LOCKED             16
#define NOT_ENOUGHT_CORE   17
#define TOO_MANY_FILES     18
#define INVALID_PARAMETER  19

//SMARTDRIVE WorkSpaces definitions
#define WORKSPACE0          0
#define WORKSPACE1          1
#define WORKSPACE2          2
#define WORKSPACE3          3
#define WORKSPACE4          4

//ADC and PWM definitions
#define CHANNEL0 0
#define CHANNEL1 1
#define CHANNEL2 2
#define CHANNEL3 3

class SMARTDRIVE{
	
public:
/****************************************************************/
//Arduino exclusive Functions
/****************************************************************/
    SMARTDRIVE();
	
	void init();

	void reset();

/****************************************************************/
//Master Functions - Those functions are available all the time
/****************************************************************/	
	unsigned char mountUnmountDrive(); 
	
	unsigned char sleepInOut();

/****************************************************************/
//General Functions - Those functions are ONLY available when the SMARTDRIVE disk has been mounted
/****************************************************************/
	unsigned char masterBaudChange(unsigned long);

	unsigned char secondaryBaudChange(unsigned long);
	
	unsigned char getBytesSecUSART(unsigned int, unsigned int*, char[]);

	unsigned char sendBytesSecUSART(unsigned int, char[]);
	
	unsigned char getADC(unsigned char, unsigned int*);

	unsigned char setPWMFreq(unsigned long);

	unsigned char setPWMDuty(unsigned char, float, unsigned long);	

	
/****************************************************************/
//SD FAT management Functions - Those functions are ONLY available when the SMARTDRIVE disk has been mounted
//Those next SDF - SD Functions return file execution status instead of ACK 'O' or NAK 'F'
/****************************************************************/
	unsigned char getList(unsigned int*, unsigned int*); //get number of dirs and files

	unsigned char getDirName(unsigned int, char[]); //searches for the "itemNumber" on the SD current folder and updates the buffer with the Dir name ended with NULL character

	unsigned char getFileName(unsigned int, char[]); //searches for the "itemNumber" on the SD current folder and updates the buffer with the File name ended with NULL character

	unsigned char getDirPath(char[]); //obtains current dir path and stores on path[] buffer

	unsigned char newDir(char[]); //create a new Directory, fails if already exist

	unsigned char newFile(char[]); //create a new File, fails if already exist

	unsigned char openDir(char[]); //opens an existing Dir

	unsigned char openFile(char[], unsigned char, unsigned char); //opens an existing file in READONLY, WRITEONLY or READWRITE mode on the received object # workspace

	unsigned char closeFile(unsigned char); //close and save file object # workspace
 
	unsigned char saveFile(unsigned char); //sync/save file object # workspace

	unsigned char setFilePointer(unsigned long, unsigned char); // set/move file pointer of file object # workspace

	unsigned char getFilePointer(unsigned long*, unsigned char); // get file pointer of file object # workspace

	unsigned char readFile(char[], unsigned int, unsigned int*, unsigned char); //Bytes to Read, Succesfully Read Bytes, file object # to read bytes from

	unsigned char writeFile(char[], unsigned int, unsigned int*,  unsigned char); //Bytes to Write, Succesfully Written Bytes, file object # to write bytes

	unsigned char testFileError(unsigned char);  //test for an error on file # workspace

	unsigned char testFileEnd(unsigned char);  //test for an error on file # workspace

	unsigned char truncateFile(unsigned char);  //truncates the file size to the current file read/write pointer of the file # workspace

	unsigned char eraseDirFile(char[]); //Erases an existing Dir or File

	unsigned char setFileTimeDate(char[], char[], char[]); //Set Time and Date to an existing File

	unsigned char getFileTimeDate(char[], char[], char[]); //Get Time and Date to an existing File

	unsigned char getFileSize(char[],unsigned long *); //Get Size of an existing File

	unsigned char renameMoveDirFile(char[], char[]); //renames or moves an existing Dir or File

	unsigned char copyFile(char[], char[], unsigned long *); //copies an existing File

	unsigned char getFreeTotalSpace(unsigned long *,unsigned long *); //Get free and total space in bytes of the microSD card
	
};

#endif
