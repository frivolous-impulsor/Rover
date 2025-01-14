#define IR_sensor 2
#define Trig 4
#define Echo 3
#define IN1 9 //fwd left
#define IN2 10 //bwd left
#define IN3 11 //fwd right
#define IN4 12//bwd right
#define OnOffButton 4
#define PowerPin 7

const float ARENA_DIAMETRE = 50.0;   //needs tunning and verification

enum {searchState, engageState, correctionState, pushState};
unsigned char roverState;

const byte onOffState = 0;
byte buttonNew;
byte buttonOld = 1;

const int delayTime = 100;
const int pullBackTime = 5000;
const int searchDelay = 100;


float duration = 0.0;
float distance = 0.0;

//for search


//for engage
const float engageSpeed = 5;
const int engageDelay = 100;
float distanceOld = ARENA_DIAMETRE + 10;  //ensure that old distance is way larger than new distance, kick start the engage mode loop 
float distanceNew;

//for push
const int pushDelay = 100;
const float distancePush = 30;


//for correction
const int correctionDelay = 500;  //needs tunning to cover 100 degree after 2 stages of swing
const int rotateSpeed = 10;
byte correctionStage = 1;
byte correctionResult;

void setup() {
  // put your setup code here, to run once:
  //Define to rename pins to their purpose
  attachInterrupt(digitalPinToInterrupt(IR_sensor), pullBack, RISING);
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

int waitForStart(){
  while(1){
    buttonNew = digitalRead(OnOffButton);
    if(buttonNew && !buttonOld){
      //low -> high => switch on 
      return 0;
    }
    buttonOld = buttonNew;
    delay(100);
  }
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
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);

  //pulseIn measures duration after echo is turned on
  duration = pulseIn(Echo, HIGH);
  //0,0343 = speed of sound, time*speed=distance
  float distance = (duration/2)*0.0343;
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
  Serial.println("correction state start");
  //oscillates left and right till search span covered around 100 degree, then it's safe to say we lost target completely, will switch to search state
  //if found within 100 degree, switch to engage or push states depending on distance to target
  float distanceOld = ARENA_DIAMETRE;
  float distance = getDistance();

  for(int i = 0; i<2; i++){
    rotateLeft(rotateSpeed);
    delay(correctionDelay*correctionStage);
    distance = getDistance();
    if(distance < distanceOld){return (distance > distancePush);};  //1 => engage     0 => push
    correctionStage++;
    rotateRight(rotateSpeed);
    delay(correctionDelay*correctionStage);

    distance = getDistance();
    if(distance < distanceOld){return (distance > distancePush);}
    correctionStage++;
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
        //roverState = correctionState;
        roverState = searchState;
      }
      break;
    case pushState:
      push();
      //roverState = correctionState;
      roverState = searchState;
      break;
    case correctionState:
      correctionResult = correction();
      if(correctionResult == 2){
        roverState = searchState;
        break;
      }else if(correctionResult){
        roverState = engageState;
        break;
      }else{
        roverState = pushState;
        break;
      }
      
  }
}