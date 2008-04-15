#include "WProgram.h"
int setup_motor();
int turn(int duty_cycle, int period, int on_time, int turn_direction);
int course_correction(int course_delta);
int setup_compass();
int get_heading();
#include "SoftwareSerial.h"
#include "Wire.h"
#include "smartpilot.h"
void setup (){
  Serial.begin(9600); 
  youreOK = 1;			//initialize sate as all ok
  setup_motor();
  setup_compass();
  for( int i; i<100; i++){
    //grab some heading data and trow away the first 10 since they often are erroneous
    desired_heading = get_heading();
  }
  delay(1000);
  desired_heading = get_heading();  //TODO:  eventually this should average the heading over a number of samples.
}

void loop (){
    while (youreOK){
    current_heading = get_heading();
    course_delta = desired_heading - current_heading;
  
    Serial.print("Current heading: "); 
    Serial.print(int (current_heading));
    Serial.print("                   course_delta: ");     
    Serial.print(int (course_delta));
    Serial.print("                   desired_heading: ");
    Serial.println(int (desired_heading));
    
    course_correction(course_delta);
    //delay(1000);
    // if heading off by tollerable_error do nothing
    // else if off by small_error turn_small
    // elsif off by medium_error turn_medium
    // elsif off by large_error big_fix and alram
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//begin definitiion of Motor functions

int  motor_power_2 = 2;
int  motor_power_3 = 3;

int setup_motor(){
 //set initial state of motor to off
 pinMode(motor_power_2, OUTPUT);
 pinMode(motor_power_3, OUTPUT);
 digitalWrite(motor_power_2, HIGH); 
 digitalWrite(motor_power_3, LOW); 
 pinMode(4, OUTPUT);
 pinMode(5, OUTPUT);
 digitalWrite(4, HIGH); 
 digitalWrite(5, LOW); 
}

int turn(int duty_cycle, int period, int on_time, int turn_direction){
  //if turn_direction is negative turn port, else starboard
  int on_pulse_width = period*duty_cycle/100;
  int pulse_count = on_time/period;
  int top_pin;
  int bottom_pin;
  
  Serial.print("  -- Turning for ");
  Serial.println( int(on_time));
  
  if (turn_direction<0){
   //Serial.print("Current heading: "); 
   top_pin = 2;   //when top pin is high and bottom is low, state is off
   bottom_pin = 3; 
  }
   else if (turn_direction>0){
   top_pin = 4;   //when top pin is high and bottom is low, state is off
   bottom_pin = 5; 
  }
  else{
    return 0;
  }
  
  for (int i=0; i<pulse_count; i++){
   //turn on motor
     digitalWrite(top_pin,LOW); 
     digitalWrite(bottom_pin,HIGH); 
     delay(on_pulse_width);
    //turn off 
     digitalWrite(top_pin,HIGH); 
     digitalWrite(bottom_pin,LOW); 
     delay(period-on_pulse_width);  //now the motor is off
  }
     digitalWrite(top_pin,HIGH); 
     digitalWrite(bottom_pin,LOW); 
}
//end motor section

//////////////////////////////////////////////////////////////////////////////////////////

//course correction variables and functions
int course_correction(int course_delta){  //define function to deal with course corrections
  int tollerable_error = 50;
  int small_error = 100; //this is 10 degrees
  int medium_error = 200;
  int small_turn = 2000;  //milliseconds to run the motor
  int medium_turn = 3000;  //milliseconds to run the motor
  int big_turn = 4000;   
  int course_delta_magnitude;
  course_delta_magnitude = abs(course_delta);
  if (course_delta_magnitude < tollerable_error)
    return 0;
  else if (abs(course_delta) <= small_error){
    if (course_delta < 0){
      turn(50, 100, small_turn, -1);
    }
    else{
      turn(50, 100, small_turn, +1);
    }
  }
  else if (abs(course_delta) <= medium_error ){
    if (course_delta < 0){
      turn(50, 100, medium_turn, -1);
    }
    else{
      turn(50, 100, medium_turn, +1);
    }
  }
  else{  //we are way off course
    //alarm();
    if (course_delta < 0){
      turn(50, 100, 5000, -1);
     }
     else{
       turn(50, 100, 5000, +1);
     }
  }
 }
//end course correction section 
 
 
//heading variables and funcitoins 
int HMC6352Address = 0x42; 
int slaveAddress;             // This is calculated in the setup() function 
int ledPin = 13; 
boolean ledState = false; 
byte headingData[2]; 
int i, headingValue; 

int setup_compass(){
  //begin setup of compass
  slaveAddress = HMC6352Address >> 1;   // This results in 0x21 as the address to pass to TWI 
  pinMode(ledPin, OUTPUT);      // Set the LED pin as output 
  Wire.begin(); 
  //end setup compass
}

int get_heading(){  //dummy definition of getheading function 
  // Flash the LED on pin 13 just to show that something is happening 
  // Also serves as an indication that we're not "stuck" waiting for TWI data 
  ledState = !ledState; 
  if (ledState) { 
    digitalWrite(ledPin,HIGH); 
  } 
  else { 
    digitalWrite(ledPin,LOW); 
  } 
  // Send a "A" command to the HMC6352 
  // This requests the current heading data 
  Wire.beginTransmission(slaveAddress); 
  Wire.send("A");              // The "Get Data" command 
  Wire.endTransmission(); 
  delay(10);                   // The HMC6352 needs at least a 70us (microsecond) delay 
                               // after this command.  Using 10ms just makes it safe 
  // Read the 2 heading bytes, MSB first 
  // The resulting 16bit word is the compass heading in 10th's of a degree 
  // For example: a heading of 1345 would be 134.5 degrees 
  Wire.requestFrom(slaveAddress, 2);        // Request the 2 byte heading (MSB comes first) 
  i = 0; 
  while(Wire.available() && i < 2) {  
    headingData[i] = Wire.receive(); 
    i++; 
  } 
  headingValue = headingData[0]*256 + headingData[1];  // Put the MSB and LSB together 
//  Serial.print("Current heading: "); 
//  Serial.print(int (headingValue / 10));     // The whole number part of the heading 
//  Serial.print("."); 
//  Serial.print(int (headingValue % 10));     // The fractional part of the heading 
//  Serial.println(" degrees"); 
  return headingValue;
  //delay(200); 
} 
//end heading sectioin

int main(void)
{
	init();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

