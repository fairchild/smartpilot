
/*
*  smarto.c steers the boat.
* Digital Pin outs are: 
* 	Pin1: Alarm - Output
* 	Pin2: Motor1 - Output
*	Pin3: Motor2 - Output
*	Pin4: SetHeading - declared in whereto.h
*	Pin5: 
*	Pin6: courseDelta = -20
*	Pin7: courseDelta = -10
*	Pin8: courseDelta < deadBand
*	Pin9: courseDelta = +10
*	Pin10: courseDelat = +20
*	Pin11: 
*	Pin12:
*	Pin13: Output - ledPin - set in heading.c

AnalogPins

*	Pin1:
*	Pin2:
*	Pin3:
*	Pin4: 
*	Pin5:
*/
//int current_bearing;
int compasSays; 		//a bearing based on real time compass data
int smartCompasSays;	//conditioned compass data
//int thereTo;	 		//the bearing we'd like to hold
int  youreOK = 1;		//a flag accessible by several functions. If it ever goes low alarm should sound;
int alarmPin = 1;		//location of alarm pin	
int course_delta;          //amount that we are off course
int desired_heading;        //the bearing we'd like to hold
int current_heading;
int courseDeltan20pin = 6;//location of -20 pin		
int courseDeltan10pin = 7;//location of -10 pin
int courseDeltaDB = 8;	//location of pin for Within the dead band
int courseDeltap10 = 9;	//location of +10 pin
int courseDeltap20 = 10;	//location of +20 pin

int  motor_power_turn_port_high = 2;
int  motor_power_turn_port_low = 3;
int  motor_power_turn_starboard_high = 4;
int  motor_power_turn_starboard_low = 5;

