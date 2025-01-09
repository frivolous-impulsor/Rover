  #define IRsensor1 3
  #define IRsensor2 2
  #define Trig 6
  #define Echo 5
  #define IN1 9 //fwd left
  #define IN2 10 //bwd left
  #define IN3 11 //fwd right
  #define IN4 12//bwd right
  float duration = 0.0;
  float distance = 0.0;
  int val = 0;
  int val2 = 0;
  int iterable = 0;
  bool LorR = true; //Stores the last turn direction, left is true right is false

void setup() {
  // put your setup code here, to run once:
  //Define to rename pins to their purpose

//Left motor
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
//right motor
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(IRsensor1, INPUT);
  pinMode(IRsensor2, INPUT);
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  Serial.begin(9600);
  delay(5000);
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

void turnLeft(int speed){
  // left motor
  analogWrite(IN1, 0);
  analogWrite(IN2,speed);
  // right motor
  analogWrite(IN3,speed);
  analogWrite(IN4,0);
}

void turnRight(int speed){
  // left motor
  analogWrite(IN1, speed);
  analogWrite(IN2,0);
  // right motor
  analogWrite(IN3,0);
  analogWrite(IN4,speed);
}

void stopMotor(){
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,LOW);
  
}

void searchLeft(){
  analogWrite(IN1, 0);
  analogWrite(IN2,0);
  // right motor
  analogWrite(IN3,200);
  analogWrite(IN4,0);
}
//IR sensor code *********************************
void avoidLineOnLeft(){
  backward();
  delay(200);
    //turn away from line at 50 speed
  turnRight(150); //change the 100 value to point to center of circle
  //delay(1000);
}
void avoidLineOnRight(){
    //turn away from line at 50 speed
  backward();
  delay(200);
  turnLeft(150); //change the 100 value to point to center of circle
  //delay(1000);
}
// ultra sound
void calcD(){
  // turns trig on for a fraction of a second,
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);

  //pulseIn measures duration after echo is turned on
  duration = pulseIn(Echo, HIGH);
  //0,0343 = speed of sound, time*speed=distance
  distance = (duration/2)*0.0343;

  //prints distance
  Serial.println("distance is" +String(distance));
}

void approach(){
  val = digitalRead(IRsensor1);//left IR sensor
  val2 = digitalRead(IRsensor2);//Right IR sensor
  if (distance <30){
    Serial.println("Moving forward");
    forward();
    delay(500);
  }else if (val == 0){
    Serial.println("Avoiding on left");
    avoidLineOnLeft(); //switch left and right accordingly
  }else if (val2 == 0){
    Serial.println("Avoiding on right");
    avoidLineOnRight(); //switch left and right accordingly
  }
  else{
    Serial.println("searching");
    searchLeft();
    }
}
void loop() {
  // put your main code here, to run repeatedly:
  calcD();
  approach();
}

