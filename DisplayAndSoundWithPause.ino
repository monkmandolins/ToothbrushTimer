1-3-2021 Arduino Toothbrush Timer with MP3 Audio

#include <Event.h>
#include <Timer.h>
#include <TimeLib.h>
#include "SoftwareSerial.h"

Timer t; // create a timer object
long number = 0; //declear the variables
int first_digit = 0;
int second_digit = 0;
int third_digit = 0;
int fourth_digit = 0;
int CA_1 = 12;
int CA_2 = 11;
int CA_3 = 10;
int CA_4 = 9;
int clk = 6;
int latch = 5;
int data = 4;
int count = 0;
int digits[4] ;
int CAS[4] = {12, 11, 10, 9};
byte numbers[10] {B11111100, B01100000, B11011010, B11110010, B01100110, B10110110, B10111110, B11100000, B11111110, B11110110};       //byte combinations for each number 0-9

//Push Button Stuff
const int buttonPin = 2;
int buttonState = 0; 

//Counter Stuff
int timer_event = 0;
boolean is_counting = false;
long started;  //no need to use "long int", just a "long" means the same thing
int current_number = 0;
int last_number_shown;

//MP3Player Stuff
SoftwareSerial mySerial(8, 7);
# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]
# define ACTIVATED LOW


void setup() {
  pinMode(CA_1, OUTPUT);
  pinMode(CA_2, OUTPUT);
  pinMode(CA_3, OUTPUT);
  pinMode(CA_4, OUTPUT);
  pinMode(clk, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(data, OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(CA_1, HIGH);
  digitalWrite(CA_2, HIGH);
  digitalWrite(CA_3, HIGH);
  digitalWrite(CA_4, HIGH);
  mySerial.begin (9600);   //for the MP3 player
}

void(* resetFunc) (void) = 0;   // Reset to be used after the routine is done

void loop() {
  buttonState = digitalRead(buttonPin);
  t.update(); //timer update
  
  if (buttonState && !is_counting) {    // this only triggers if the button is pressed and there's not currently a countdown underway
    is_counting = true;                 // change state of variable is_counting to true
    started = now();                    // assign the value of now() to the variable started.  This number doesn't change throughout the countdown
    last_number_shown = -1;             //
    setVolume(30);                      // set MP3 player volume to 30 (maximum)
    playNext();                         // play the songe
  }
  
  // stops the counter, resets the device
  buttonState = digitalRead(buttonPin);  // check if the pushbutton is pressed
  if (buttonState && is_counting) {      // this triggers if the button is pressed and there is currently a countdown running
    //is_counting = false;                 // change state of variable is_counting to false
    //current_number = 0;                  // stop the counter
    playPause();
    resetFunc();
    }

  if (is_counting == true) {             // at this point in the code, if the variable is_counting has been changed to true by button being pressed when the counting routine is not already running
    long time_now = now();               // assign the value of now() to the variable time_now
    current_number = 120 - (time_now - started);    // assign a new value to current_number based on 120 minus how long it has been since the value of started was assigned
    if (current_number != last_number_shown) {      // if the current number does not equal last_number_shown
      
      t.stop(timer_event); //stop timer if anything to read
      cathode_high(); // blank the screen
      break_number(current_number);  //split the current_number into a 4-digit array
      timer_event = t.every(1, display_number);   // start timer again- THIS IS THE MAGIC FOR DISPLAYING THE NUMBERS
      last_number_shown = current_number;          // assign the value of current_number to last_number_shown
    }
      
 // need to catch the end.  Without this, the counter just continues to go below zero
  if (current_number == 0) {            // at the point where current number is zero
    is_counting = false;                // turn off the counter
    resetFunc();
    }  
  }  
}

//Function to break the current number into a 4 digit array
void break_number(long num) { // seperate the input number into 4 single digits
  first_digit = num / 1000;
  digits[0] = first_digit;
  int first_left = num - (first_digit * 1000);
  second_digit = first_left / 100;
  digits[1] = second_digit;
  int second_left = first_left - (second_digit * 100);
  third_digit = second_left / 10;
  digits[2] = third_digit;
  fourth_digit = second_left - (third_digit * 10);
  digits[3] = fourth_digit;
}

//Function to actually display the number
void display_number() { //scanning
  cathode_high(); //black screen
  digitalWrite(latch, LOW); //put the shift register to read
  shiftOut(data, clk, LSBFIRST, numbers[digits[count]]); //send the data
  digitalWrite(CAS[count], LOW); //turn on the relevent digit
  digitalWrite(latch, HIGH); //put the shift register to write mode
  count++; //count up the digit
  if (count == 4) { // keep the count between 0-3
    count = 0;
  }
}

//Function to turn off all 4 digits
void cathode_high() { 
  digitalWrite(CA_1, HIGH);
  digitalWrite(CA_2, HIGH);
  digitalWrite(CA_3, HIGH);
  digitalWrite(CA_4, HIGH);
}

//Function to set the volume, where setVolume(INPUT THE VOLUME FROM 0-30)
void setVolume(int volume)
{
execute_CMD(0x06, 0, volume); // Set the volume (0x00~0x30)
}

//Function to play a song
void playNext()
{
  execute_CMD(0x01,0,1);
}

//Function to pause player
void playPause()
{
  execute_CMD(0x0E,0,1);
}

//Part of the library for command execution for the MP3 player
void execute_CMD(byte CMD, byte Par1, byte Par2)      // Excecute the command and parameters
{
  word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);     // Calculate the checksum (2 bytes)
  byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge, Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte};   // Build the command line
  for (byte k=0; k<10; k++)     
{
    mySerial.write( Command_line[k]);    //Send the command line to the module
}
}
