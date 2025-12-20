#define TRIG_PIN 8
#define ECHO_PIN 9
#define PUMPE_PIN 3 //2 eller 3
#define VENTIL_PIN 2 
#define Bruger_IN A0

int distance;
long duration;

int dist1;
int dist2;
int distUsed;
int volumeUsed;
int buttonSavedState = 0; // 1 - button is pressed, 0 - button is released


void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);
   pinMode(TRIG_PIN, OUTPUT);
   pinMode(ECHO_PIN, INPUT);
   pinMode(PUMPE_PIN, OUTPUT);
   pinMode(VENTIL_PIN, OUTPUT);
   pinMode(Bruger_IN, INPUT_PULLUP); //INPUT_PULLUP
}


int getDistance() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.0344 / 2; // cm

}

void loop() {
  // put your main code here, to run repeatedly:
  int Input_ventil = analogRead(A0); //input from user to start pumpe

  distance = getDistance();

  bool buttonPressed = Input_ventil >= 1020;

  //Serial.println(distance);     //print distance from sencor to water


  if(buttonPressed) {
    if (buttonSavedState == 0){
      Serial.println("knappen on");
      dist1 = distance;         //function to start pumpe when used by user
      buttonSavedState = 1;
    }
    digitalWrite(PUMPE_PIN, HIGH);
  }
  else {
    Serial.println("knappen off");
    digitalWrite(PUMPE_PIN, LOW);

    if (buttonSavedState == 1){
      dist2 = distance;
      distUsed = dist2-dist1;   //find difference betveen distance before water been used, and after. can be used to find how much water in liters is used
      volumeUsed = distUsed * 350;     //calculate volume used in mililiters

      int Liters = volumeUsed / 1000;  //convert to liters

      Serial.println(Liters);

      buttonSavedState = 0;
    }  
  }

  if(distance <=20){
    digitalWrite(VENTIL_PIN, LOW);
  }else{
    digitalWrite(VENTIL_PIN, HIGH);
  }

  delay(1000);

}