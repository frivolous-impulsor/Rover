#define IR_sensor 2
#define Trig 4
#define Echo 3
#define IN1 9 //fwd left
#define IN2 10 //bwd left
#define IN3 11 //fwd right
#define IN4 12//bwd right
#define PowerPin 7

const float ARENA_DIAMETRE = 50.0;   //needs tunning and verification

enum {searchState, engageState, correctionState, pushState};
unsigned char roverState; //global var that indicates the current state

const byte onOffState = 0;

const int delayTime = 100;
const int pullBackTime = 5000;
const int searchDelay = 100;

//for search

//for engage
const float engageSpeed = 5;
const int engageDelay = 100;

//for push
const int pushDelay = 100;
const float distancePush = 30;


//for correction
const int correctionDelay = 500;  //needs tunning to cover 100 degree after 2 stages of swing

void setup() {
  // put your setup code here, to run once:
  //Define to rename pins to their purpose
  //attachInterrupt(digitalPinToInterrupt(IR_sensor), pullBack, RISING);
  //Left motor
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  //right motor
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  Serial.begin(9600);
}



void forward(){

  //May vary depending on motor wiring
  //left motor
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2,LOW);

  //right motor
  digitalWrite(IN3,HIGH);
  digitalWrite(IN4,LOW);
  //analogWrite(IN1,100);
  //analogWrite(IN2,0);
  // right motor
  //analogWrite(IN3, 100);
  //analogWrite(IN4,0);
}

void backward(){

  //May vary depending on motor wiring

  //left motor
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,HIGH);

  //right motor
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,HIGH);
}

void forwardAnalog(int speed){
  //left motor
  analogWrite(IN1, speed);
  analogWrite(IN2,0);
  // right motor
  analogWrite(IN3, speed);
  analogWrite(IN4,0);
}

void backwardAnalog(int speed){
  //left motor
  analogWrite(IN1, 0);
  analogWrite(IN2,speed);
  // right motor
  analogWrite(IN3, 0);
  analogWrite(IN4,speed);
}

void rotateLeft(int speed){
  // left motor
  analogWrite(IN1, 0);
  analogWrite(IN2,speed);
  // right motor
  analogWrite(IN3,speed);
  analogWrite(IN4,0);
}

void rotateRight(int speed){
  // left motor
  analogWrite(IN1, speed);
  analogWrite(IN2,0);
  // right motor
  analogWrite(IN3,0);
  analogWrite(IN4,speed);
}

void halt(){
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,LOW);
  Serial.println("halted");  
}

void pullBack(){
  backward();
  delay(pullBackTime);
  halt();
}

void searchLeft(){
  analogWrite(IN1, 0);
  analogWrite(IN2,0);
  // right motor
  analogWrite(IN3,200);
  analogWrite(IN4,0);
}


// ultra sound
float getDistance(){
  // turns trig on for a fraction of a second,
  float duration = 0.0;
  float distance = 0.0;
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);

  //pulseIn measures duration after echo is turned on
  duration = pulseIn(Echo, HIGH);
  //0,0343 = speed of sound, time*speed=distance
  distance = (duration/2)*0.0343;
  Serial.println("distance: " +String(distance));
  return distance;
}


int search(){
  Serial.println("search state start");
  halt();
  float distanceNew = getDistance();
  while(distanceNew !=0 && distanceNew > ARENA_DIAMETRE){
    rotateLeft(255);
    delay(searchDelay);
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
  while(distanceNew <= (distanceOld + eps) && distanceNew >= distancePush){
    forwardAnalog(125);
    delay(engageDelay);
    distanceOld = distanceNew;
    distanceNew = getDistance();
  }
  if(distanceNew <= (distancePush + eps) ){
    Serial.println("close to target, switch to push state");
    return 0;
  }else{
    Serial.println("lost target, switch to correction state");
    return 1;
  }

}

int push(){
  Serial.println("push state start");
  float eps = 1;
  float distanceOld = ARENA_DIAMETRE;
  float distanceNew = getDistance();
  while(distanceNew <= (distanceOld + eps)){
    forward();      //full thrust
    delay(pushDelay);
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
  int delayTime = 100;
  float proportionFactor = 0.5;
  float time;
  int rotateSpeed = 255;
  float eps = 1.0;

  for(int t = 1; t<4; ++t){ //t:= proportional time spent rotating left, 1->2->3   each left succeeded with right rotation of doouble time to recover
    time = t*proportionFactor;
    int iterationLeft = int(time*1000/delayTime);
    int iterationRight = 2*iterationLeft;

    rotateLeft(rotateSpeed);
    for(int l = 0; l<iterationLeft; ++l){
      delay(delayTime);
      distance = getDistance();
      if(distance < (ARENA_DIAMETRE + eps)){
        return (distance > distancePush);
      }
    }

    rotateRight(rotateSpeed);
    for(int r = 0; r<iterationLeft; ++r){
      delay(delayTime);
      distance = getDistance();
      if(distance < (ARENA_DIAMETRE + eps)){
        return (distance > distancePush);
      }
    }

  }
  return 2;
}

void loop() {
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
  //delay(300);  //delay for debug
}