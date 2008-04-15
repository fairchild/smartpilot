#include "WProgram.h"
int setup_motor();
int turn(int duty_cycle, int period, int on_time, int turn_direction);
int stop_turn();
int course_correction(int local_course_delta);
int setup_compass();
int get_heading();
int update_heading_stack();
int get_current_heading();
#include "SoftwareSerial.h"
#include "Wire.h"
#include "smartpilot.h"
int headings[10];
unsigned int counter = 0;
int motor_command[4]; //duty_cycle, period, on_time, direction

void setup (){
  int count;
  Serial.begin(9600); 
  youreOK = 1;			//initialize sate as all ok
  setup_motor();
  setup_compass();
  delay(1000);  //wait for the first bit of spurious data to pass
  int sum=0;
  for( int i=0; i<10; i++){
    headings[i]= get_current_heading();
    sum += headings[i];
  }
  desired_heading = sum/10;
}

int average_heading;
int count_down=0;
void loop (){
    while (youreOK){    
      if ( (counter % 100) ==  0 ){  //update timeaveraged heading every 300 cycles
        update_heading_stack();
        // Serial.print(int(counter));
      }
      if ( (counter % 50000) ==  0 ){  //take course correcting action on a much less frequent basis
        average_heading = get_heading();
        course_delta = desired_heading - average_heading;
        Serial.print(int (counter));
        Serial.print(":  Current heading: ");
        Serial.print(int (average_heading));
        Serial.print("\tcourse_delta: ");
        Serial.print(int (course_delta));
        Serial.print("\tdesired_heading: ");
        Serial.println(int (desired_heading));
        course_correction(course_delta);
          turn(motor_command[0], motor_command[1], motor_command[2], motor_command[3]);
          // Serial.print(int (course_delta));
          if (motor_command[2]>0)
            Serial.println(" correcting course");
        counter = 0;
      }
      counter++;
   } 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//begin definitiion of Motor functions

int setup_motor(){
 //set initial state of motor to off
 pinMode(2, OUTPUT);
 pinMode(3, OUTPUT);
 digitalWrite(2, HIGH); 
 digitalWrite(3, LOW); 
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
  
  for (int i=0; i < pulse_count; i++){
   //turn on motor
     digitalWrite(top_pin,LOW); 
     digitalWrite(bottom_pin,HIGH); 
     delay(on_pulse_width);
     digitalWrite(top_pin,HIGH); 
     digitalWrite(bottom_pin,LOW);
     delay(period-on_pulse_width);
  }
  return 0;
}
  
int stop_turn(){
  //turn everything off
  digitalWrite(2, HIGH); 
  digitalWrite(3, LOW); 
  digitalWrite(4, HIGH); 
  digitalWrite(5, LOW);
}
//end motor section

//////////////////////////////////////////////////////////////////////////////////////////

//course correction variables and functions
int course_correction(int local_course_delta){  //define function to deal with course corrections
  int tollerable_error = 50;
  int small_error = 100; //this is 10 degrees
  int medium_error = 200;
  int small_turn = 2000;  //milliseconds to run the motor
  int medium_turn = 3000;  //milliseconds to run the motor
  int big_turn = 4000;
  int base_duty_cycle = 10;
  int motor_period = 100;
  int course_delta_magnitude;
  // int motor_commands_array[4];
  int port = -1;
  int starboard = +1;
  course_delta_magnitude = abs(local_course_delta);
  motor_command[0] = base_duty_cycle;
  motor_command[1] = 100; //motor period
  
  if (course_delta_magnitude < tollerable_error){
    motor_command[2]=0;
    return 0;
  }
  if (course_delta < 0){
    motor_command[3] = port;
  }
  else{
    motor_command[3] = starboard;
  }
  if (abs(course_delta) <= small_error){
    motor_command[2] = small_turn;
    return small_turn;
  }
  else if (abs(course_delta) <= medium_error ){
    motor_command[2] = medium_turn;
    return medium_turn;
  }
  else{  //we are way off course
    //alarm();
    motor_command[2] = big_turn;
    return big_turn;
  }
} 
//end course correction section 
 
////////////////////////////////////////////////////////////////////////////////////////////////////
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

int get_heading(){
  int sum=0;
  for (int i=0; i<=9; i++){
    sum += headings[i];
  }
  return sum/10;
}

int update_heading_stack(){
  for (int i=9; i>=0; i--){
    headings[i]=headings[i-1];
  }
  headings[0] = get_current_heading();
}

int get_current_heading(){  //dummy definition of getheading function 
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

