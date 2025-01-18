#define IR_sensor 2
#define Trig 5
#define Echo 6
#define IN1 3 //fwd left
#define IN2 9 //bwd left
#define IN3 10 //fwd right
#define IN4 11//bwd right
//#define PowerPin 7

const float ARENA_DIAMETRE = 77.0;   //needs tunning and verification

enum {engageState, searchState, correctionState, pushState, pullbackState};
volatile unsigned char roverState = correctionState; //global var that indicates the current state

//const byte onOffState = 0;

const int delayTime = 100;
const long pullBackTime = 300;
const int searchDelay = 100;

//for search
const int searchSpeed = 64;

//for engage
const int engageSpeed = 64;
const int engageDelay = 100;

//for push
const int delayTimePush = 100;
const float distancePush = 30;


//for correction
const int correctionSpeed = 64;
const int delayTimeCorrection = 100;

void setup() {
  // put your setup code here, to run once:
  //Define to rename pins to their purpose
  //Left motor
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  //right motor
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(IR_sensor, INPUT);
  //attachInterrupt(digitalPinToInterrupt(IR_sensor), pullBack, HIGH);

  Serial.begin(9600);
}

void forwardAnalog(int speed){
  Serial.println("forward analog");

  //left motor
  analogWrite(IN1, speed);
  analogWrite(IN2,0);
  // right motor
  analogWrite(IN3, speed);
  analogWrite(IN4,0);
}

void backwardAnalog(int speed){
  Serial.println("backward analog");
  //left motor
  analogWrite(IN1, 0);
  analogWrite(IN2,speed);
  // right motor
  analogWrite(IN3, 0);
  analogWrite(IN4,speed);
}


void rotateLeft(int speed){
  Serial.println("rotate left");
  // left motor
  analogWrite(IN1, 0);
  analogWrite(IN2,speed);
  // right motor
  analogWrite(IN3,speed);
  analogWrite(IN4,0);

  analogWrite(IN1, 0);
  analogWrite(IN2,speed);
  // right motor
  analogWrite(IN3,speed);
  analogWrite(IN4,0);
}

void rotateLeftSlow(){
  rotateLeft(64);
  delay(60);
  rotateLeft(0);
}

void rotateRight(int speed){
  Serial.println("rotate right");
  // left motor
  analogWrite(IN1, speed);
  analogWrite(IN2,0);
  // right motor
  analogWrite(IN3,0);
  analogWrite(IN4,speed);
}

void rotateRightSlow(){
  rotateRight(64);
  delay(60);
  rotateRight(0);
}


void halt(){
  analogWrite(IN1,HIGH);
  analogWrite(IN2,HIGH);
  analogWrite(IN3,HIGH);
  analogWrite(IN4,HIGH);
  Serial.println("halted");  
}


void pullBack(){
  halt();
  roverState = pullbackState;
  Serial.println("IR sensor detected! pull back");
}

// ultra sound
float getDistance(){
  // turns trig on for a fraction of a second,
  float duration = 0.0;
  float distance = 0.0;
  const float errReadingMax = 0.2;
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);

  //pulseIn measures duration after echo is turned on
  duration = pulseIn(Echo, HIGH);
  //0,0343 = speed of sound, time*speed=distance
  distance = (duration/2)*0.0343;
  if(distance <= errReadingMax){
    Serial.println("distance off");
    
    return getDistance();
  }
  Serial.println("distance: " +String(distance));
  return distance;
}


int search(){
  Serial.println("search state start");
  float distanceNew = getDistance();
  float eps = 1.0;
  
  while(distanceNew > ARENA_DIAMETRE + eps){

    rotateLeftSlow();
    distanceNew = getDistance();
  }
  halt();
  if(distanceNew < distancePush){
    Serial.println("search ended, target close, will push");
    return 0; //close to target, push mode 
  }else{
    Serial.println("search ended, target far, will engage");
    return 1; //far to target, engage mode
  }
}

int engage(){
  Serial.println("engage state start");
  const float eps = 1;

  float distanceOld = ARENA_DIAMETRE;
  float distanceNew = getDistance();
  forwardAnalog(engageSpeed);
  
  while(distanceNew <= (distanceOld + eps) && distanceNew >= distancePush){
    delay(engageDelay);
    distanceOld = distanceNew;
    distanceNew = getDistance();
  }
  
  if(distanceNew <= (distancePush + eps) ){
    Serial.println("close to target, switch to push state");
    return 0;
  }else{
    halt();
    Serial.println("lost target, switch to correction state");
    return 1;
  }

}

int push(){
  Serial.println("push state start");
  float eps = 1;
  float distanceOld = ARENA_DIAMETRE;
  float distanceNew = getDistance();
  forwardAnalog(255);      //full thrust
  while(distanceNew < distancePush || distanceNew <= (distanceOld + eps)){
    delay(delayTimePush);
    distanceOld = distanceNew;
    distanceNew = getDistance();
  }
  halt();
  Serial.println("lost target, will switch to correction state");
  return 1;
}

int correction(){
  //return 0 => push, 1 => engage, 2 => search
  Serial.println("correction state start");
  //oscillates left and right till search span covered around 100 degree, then it's safe to say we lost target completely, will switch to search state
  //if found within 100 degree, switch to engage or push states depending on distance to target
  float distance;
  int correctionStage = 1;
  float proportionFactor = 0.5;
  float time;
  float eps = 1.0;
  const float swingTimeFactor = 200;

  int iterationLeft;
  int iterationRight;
  for(int t = 1; t<5; ++t){ //t:= proportional time spent rotating left, 1->2->3   each left succeeded with right rotation of doouble time to recover
    time = t*proportionFactor;
    iterationLeft = int(time*swingTimeFactor/delayTime);
    iterationRight = 2*iterationLeft;

    for(int l = 0; l<iterationLeft; ++l){
      rotateLeftSlow();
      distance = getDistance();
      if(distance < (ARENA_DIAMETRE + eps)){
        halt();
        Serial.println("found opponent");
        return (distance > distancePush);
      }
    }

    for(int r = 0; r<iterationRight; ++r){
      rotateRightSlow();
      distance = getDistance();
      if(distance < (ARENA_DIAMETRE + eps)){
        halt();
        Serial.println("found opponent");
        return (distance > distancePush);
      }
    }

  }
  return 2;
}

void test(){
  pullBack();
}

void s_d(){
    // put your main code here, to run repeatedly:
    switch(roverState){
    case searchState:
      if(search() == 0){
        roverState = pushState;
      }else{
        roverState = engageState;
      }
      break;
    case engageState:
      if(engage() == 0){
        roverState = pushState;
      }else{
        roverState = correctionState;
        //roverState = searchState;
      }
      break;
    case pushState:
      push();
      roverState = correctionState;
      //roverState = searchState;
      break;
    case correctionState:
      int correctionResult = correction();
      if(correctionResult == 0){
        roverState = pushState;
      }else if(correctionResult == 1){
        roverState = engageState;
      }else{
        roverState = searchState;
      }
      break;
      
  }
  //delay(1000);  //delay for debug
}

void loop() {
  s_d();
}