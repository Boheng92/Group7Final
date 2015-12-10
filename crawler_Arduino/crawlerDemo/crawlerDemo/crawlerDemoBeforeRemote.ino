#include <Wire.h>
#include <Servo.h>
#include <PID_v1.h>
#include <SoftwareSerial.h>

//create an xBee object
//SoftwareSerial xbee(2,3); // Rx, Tx

int turnCount = 3;
int wallCount = 0;
boolean isSpecial;

double Setpoint, Input, Output;
double Kp=3.0, Ki=0.00001, Kd=1.2;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
 
Servo wheels; // servo for turning the wheels
Servo esc; // not actually a servo, but controlled like one!
bool startup = true; // used to ensure startup only happens once
int startupDelay = 1000; // time to pause at each calibration step
double maxSpeedOffset = 45; // maximum speed magnitude, in servo 'degrees'
//double maxWheelOffset = 85; // maximum wheel turn magnitude, in servo 'degrees'
double maxWheelOffset = 40; // maximum wheel turn magnitude, in servo 'degrees'

double wheelOffset = 0.0; // For Adjusting the wheel
double threshHoldDistance = 27.5 * 27.5;
//double threshHoldDistance = 69.85 * 69.85;

int pin_head = 1;
int pin_tail = 2;

double steerAngle = 0.0;
//double car_length = 13;         //inches
double car_length = 35.5;       //cm 
//double wheelStartUpOffset = 0.0; // For Adjusting the steering

void setup()
{ Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register ev
//  xbee.begin(9600);
  Serial.begin(9600);

  
  wheels.attach(8); // initialize wheel servo to Digital IO Pin #8
  esc.attach(9); // initialize ESC to Digital IO Pin #9
  /*  If you're re-uploading code via USB while leaving the ESC powered on, 
   *  you don't need to re-calibrate each time, and you can comment this part out.
   */
  calibrateESC();
    //initialize the variables we're linked to
  Input = calcDistance(getHeadDis(), getTailDis());
  Setpoint = threshHoldDistance;

  //turn the PID on
  myPID.SetOutputLimits(-0.5,0.5);
  myPID.SetMode(AUTOMATIC);

  
  setVelocity(0.5);
}


double getHeadDis() {
  double A1 = (double)analogRead(pin_head);
  double distance_head = exp(8.5841-log(A1));
//  Serial.println("head: "+ (String)distance_head);
  return distance_head;
}

double getTailDis() {
  double A2 = (double)analogRead(pin_tail);
  double distance_tail = exp(8.5841-log(A2));
//  Serial.println("tail: "+ (String)distance_tail);
  return distance_tail;
}


double calcDistance(double head, double tail) {
  double distance_head = head;
  double distance_tail = tail;
//  Serial.println("head: " + (String)distance_head);
//  Serial.println("tail: " + (String)distance_tail);
  double distance = (car_length * car_length * distance_head * distance_head) / (car_length * car_length + (distance_head - distance_tail) * (distance_head - distance_tail));
//  Serial.println("distance:  " + (String)distance);
  return distance;
}

/* Calibrate the ESC by sending a high signal, then a low, then middle.*/
void calibrateESC(){
    esc.write(180); // full backwards
    delay(startupDelay);
    esc.write(0); // full forwards
    delay(startupDelay);
    esc.write(90); // neutral
    delay(startupDelay);
    esc.write(90); // reset the ESC to neutral (non-moving) value
}

void handleWall(){
  Serial.println("truning left");
    setVelocity(0.0);
    delay(1000);
    steerLeft(0.9);
    setVelocity(-0.3);
    delay(2000);
    setVelocity(0.0);
    steerRight(0.9);
    setVelocity(0.3);
    delay(2000);
//    steerLeft(1.0);  
//    setVelocity(0.3); 
//    delay(4000);  
    steerRight(0.0);
}

void steerLeft(double d)
{ 
  if( (d >= 0.0 ) && (d <= 1.0))
  {
    double temp = min( (d * maxWheelOffset + wheelOffset), maxWheelOffset);
    
    wheels.write(70 + temp);
  }
}

void steerRight(double d)
{
  if( (d >= 0.0 ) && (d <= 1.0)){
    double temp = min( (d * maxWheelOffset + wheelOffset), maxWheelOffset);
    wheels.write(70 - temp);
  }
}

void setVelocity(double s)
{
  if( (s >= -1.0 ) && (s <= 1.0)){
    esc.write(90 - (s * maxSpeedOffset));
  }
}


void receiveEvent(int howMany)
// S, A, N, R, L, M, F, B, X
{
  char c = Wire.read();// receive byte as an integer
  if(c == X){
    isSpecial = true;
    Serial.println("setting the flag ffffffffffffffffffffffffffffffffff");
  }
}



void loop()
{
    double head_dis = getHeadDis();
    double tail_dis = getTailDis();
    if ( head_dis < 500 && tail_dis < 500) {
//        Serial.println("head_dis: " + (String)head_dis + "   tail_dis:  "+ (String)tail_dis);
        //delay(5000);
        Input = calcDistance(getHeadDis(), getTailDis());
        Serial.println("run once");
        myPID.Compute();
        if (Output < 0) {
           steerRight(-Output);
        } else {
           steerLeft(Output);
        }
    }
    else {
        int temp = turnCount%4;
        if(isSpecial){
//          Serial.println("sepcial turning right!==============");
//          Serial.println("head_dis: " + (String)head_dis + "   tail_dis:  "+ (String)tail_dis);
          wheels.write(5);  
          delay(2500);
          wheels.write(50);
          delay(1000);
          wheels.write(120);
          delay(1000);
          wheels.write(90);
          delay(1500);
          wheels.write(70);
          delay(500);
          wheels.write(90);
          delay(2000);
          isSpecial = false;
//          turnCount++;
        }else{
          Serial.println("normal turning right!==============");
          Serial.println((String)temp);
          wheels.write(5);  
          delay(2500);
          wheels.write(50);
          delay(500);
          wheels.write(120);
          delay(500);
          wheels.write(90);
          delay(500);
//          turnCount++;
        }
    }
}

