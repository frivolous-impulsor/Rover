#define IR_sensor 2
#define Trig 6
#define Echo 5
#define IN1 9 //fwd left
#define IN2 10 //bwd left
#define IN3 11 //fwd right
#define IN4 12//bwd right
#define OnOffButton 4
#define PowerPin 7

const float ARENA_DIAMETRE = 10.0   //needs tunning and verification

enum {searchState, engageState, correctionState, pushState};
unsigned char roverState;

const byte onOffState = 0;
const byte buttonNew;
const byte buttonOld = 1;

const int delayTime = 100;
const int pullBackTime = 5000;
const int searchDelay = 100;


float duration = 0.0;
float distance = 0.0;

//for search


//for engage
const float engageSpeed = ?;
const int engageDelay = 100;
float distanceOld = ARENA_DIAMETRE + 10;  //ensure that old distance is way larger than new distance, kick start the engage mode loop 
float distanceNew;

//for push
const int pushDelay = 100;
const float distancePush = ?;


//for correction
const int correctionDelay = 500;  //needs tunning to cover 100 degree after 2 stages of swing
const rotateSpeed = ?;
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

  pinMode(onButton, INPUT);
  pinMode(IRsensor1, INPUT);
  pinMode(IRsensor2, INPUT);
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  Serial.begin(9600);
}


int waitForStart(){
  while(1){
    buttonNew = digitalRead(OnOffButton)
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
  
}

void pullBack(){
  backwardAnalog()
  delay(pullBackTime)
  halt()
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
  return (duration/2)*0.0343;

  //prints distance
  //Serial.println("distance is" +String(distance));
}


int search(){
  stop();
  distanceNew = getDistance();
  while(distance > ARENA_DIAMETRE){
    rotateLeft();
    delay(searchDelay);
    distanceNew = getDistance();
  }
  if(distance < distancePush){
    return 0; //close to target, push mode 
  }else{
    return 1; //far to target, engage mode
  }
}

int engage(){
  distanceNew = getDistance();
  while(disanceNew <= distanceOld && distanceNew >= distancePush){
    forwardAnalog(engageSpeed);
    delay(engageDelay);
    distanceNew = getDistance();
  }
  if(distanceNew < distancePush){
    return 0;   //close to target, switch to push state
  }else{
    retur 1;    //lost target, switch to correction state
  }

}

int push(){
  distanceNew = getDistance();
  while(disanceNew <= distanceOld){
    forward();      //full thrust
    delay(pushDelay);
  }
  return 1;
}



int correction(){
  //oscillates left and right till search span covered around 100 degree, then it's safe to say we lost target completely, will switch to search state
  //if found within 100 degree, switch to engage or push states depending on distance to target
  distanceOld = ARENA_DIAMETRE
  distance = getDistance()

  for(int i = 0; i<2; i++){
    rotateLeft(rotateSpeed);
    delay(correctionDelay*correctionStage);
    distance = getDistance()
    if(distance < distanceOld){return (distance > distancePush)}    //1 => engage     0 => push
    correctionStage++;
    rotateRight(rotateSpeed);
    delay(correctionDelay*correctionStage);

    distance = getDistance()
    if(distance < distanceOld){return (distance > distancePush)}
    correctionStage++;
  }

  return 2;
}

void loop() {
  // put your main code here, to run repeatedly:
  switch(roverState){
    case searchState:
      Serial.println("search state start");
      if(search() == 0){
        roverState = pushState;
      }else{
        roverState = engageState;
      }
      break;
    case engageState:
      Serial.println("engage state start");

      if(engage() == 0){
        roverState = pushState;
      }else{
        roverState = correctionState;
      }
      break;
    case pushState:
      Serial.println("push state start");
      push()
      roverState = correctionState;
      break;
    case correctionState:
      Serial.println("correction state start");
      correctionResult = correction()
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