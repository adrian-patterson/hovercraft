#include <Servo.h>
#define direction_pin 6                            // PIN DEFINITIONS
#define lift_pin 10
#define servo_pin 9
#define us1_pin 3
#define us2_pin 4
#define us3_pin 2

//define constants
Servo servo; 	// initializing servo angle and a servo object
volatile int servo_angle = 90; 
double turn_rate = 0.0; 
bool DCC_ACTIVE = false; 	//course correction flag 
bool TURN_ACTIVE = false; 	//panic mode flag 

//control panel
const double DCC_AGGRO = 2;	//course correct sensitivity
const double servo_offset = 0; 	//software servo offset: used for calibration 
const double servo_limit = 60; 	//servo choke angle: degrees away from mid
const double DCC_THRESHOLD = 300; 	//DCC algo activation distance 
const double DCC_THRESHOLD_STRAIGHT = 500; //panic mode activation distance 
const double PANIC_AGGRO = 2.75; 	//panic mode MULTIPLIER
const double THRUST_SPEED = 245; //thruster speed

//define sensors
long US1, US2, US3, US_reading_1, US_reading_2, US_reading_3;

void setup()
{
  pinMode(lift_pin,OUTPUT);
  pinMode(direction_pin,OUTPUT);              // pin modes
  pinMode(servo_pin,OUTPUT);
  pinMode(us1_pin,INPUT);
  pinMode(us2_pin,INPUT);
  pinMode(us3_pin, INPUT);
  
  servo.attach(servo_pin);      
  analogWrite(6,THRUST_SPEED); //thrust: analog 
  digitalWrite(10,255); 	//lift: digital only
  Serial.begin(9600);

  servo.write(servo_angle - servo_offset);
}

void loop() {
  read_sensor();
  print_range();
  DCC();

  delay(5);
}

/*--------------------------------------------FUNCTIONS-----------------------------------------*/

void read_sensor(){
  //US sensor 1
  US1 = pulseIn(us1_pin, HIGH);
  US_reading_1 = US1 * 1.81378476420798 / 10;
  //US sensor 2
  US2 = pulseIn(us2_pin, HIGH);
  US_reading_2 = US2 * 1.81378476420798 / 10;
  //US sensor 3
  US3 = pulseIn(us3_pin, HIGH);
  US_reading_3 = US3 * 1.81378476420798 / 10;

  US_reading_1 -= 45;
  US_reading_2 -= 45;
}

void DCC(){ //DCC = Dynamic Course Correction
  DCC_ACTIVE = false;
  turn_rate = 0;
  servo_angle = 90;
  TURN_ACTIVE = false;
  double aggro_temp = DCC_AGGRO;
  
  if (US_reading_1 <= DCC_THRESHOLD || US_reading_2 <= DCC_THRESHOLD || US_reading_3 <= DCC_THRESHOLD_STRAIGHT){
    DCC_ACTIVE = true;

    if(US_reading_3 <= DCC_THRESHOLD_STRAIGHT){//determine whether or not to activate panic mode 
      TURN_ACTIVE = true;
      aggro_temp *= PANIC_AGGRO;
    }
    
    if(US_reading_1 < US_reading_2){ //adjust left 
        turn_rate = -1.0 / (US_reading_1 / 10.0);
    }else if(US_reading_1 > US_reading_2){//adjust right
        turn_rate = 1.0 / (US_reading_2 / 10.0);
    }
    turn_rate *= 180;
    
    }else{//no adjustment 
      turn_rate = 0;
  }
  
  turn_rate *= aggro_temp;
  servo_angle += turn_rate;
  
  //sway choke
    if(servo_angle > 90 + servo_limit){
      servo_angle = 90 + servo_limit;
    }else if(servo_angle < 90 - servo_limit){
      servo_angle = 90 - servo_limit;
    }

  servo.write(servo_angle - servo_offset);
}

void print_range(){
Serial.print("Ultrasonic 1 = ");
Serial.print(US_reading_1);
Serial.print("mm");
Serial.print("; ");

Serial.print("Ultrasonic 2 = ");
Serial.print(US_reading_2);
Serial.print("mm");
Serial.print("; ");

Serial.print("Ultrasonic 3 = ");
Serial.print(US_reading_3);
Serial.print("mm");
Serial.println(" ");

Serial.print("servo angle = ");
Serial.print(servo_angle);
Serial.print(", change: ");
Serial.print(turn_rate);
Serial.println(" ");
}
