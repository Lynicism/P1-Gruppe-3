#define TRIG_PIN 3
#define ECHO_PIN 2
#define PUMPE_PIN 6
#define VENTIL_PIN 1 //change later


int distance;
long duration;

int dist1;
int dist2;
int distUsed;
int volumeUsed;
int n = 0;


void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);
   pinMode(TRIG_PIN, OUTPUT);
   pinMode(ECHO_PIN, INPUT);
   pinMode(PUMPE_PIN, OUTPUT);
   pinMode(VENTIL_PIN, OUTPUT);
}


int getDistance() {

  delay(50);
  digitalWrite(TRIG_PIN, HIGH);
  delay(50);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2; // cm

}

void loop() {
  // put your main code here, to run repeatedly:
  int value = analogRead(A5); 

  distance = getDistance();

  //Serial.println(distance);     //print distance from sencor to water


  if(value>=500 && n==1){         //function to start pumpe when used by user
    digitalWrite(PUMPE_PIN, HIGH);
    dist1 = getDistance();

    n = 0;
  }else if(value<=500 && n==0){
    digitalWrite(PUMPE_PIN, LOW);
    dist2 = getDistance();
    
    distUsed = dist2-dist1;   //find difference betveen distance before water been used, and after. can be used to find how much water in liters is used
  

    volumeUsed = distUsed * 350;     //calculate volume used in liters


    Serial.println(volumeUsed);  //print volume used to serial monitor

    n = 1;
  }else if(value>=500 && n == 0){
    digitalWrite(PUMPE_PIN, HIGH);
  }else if(value<=500 && n == 1){
    digitalWrite(PUMPE_PIN, LOW);
  }

  if(distance <=20){
    digitalWrite(VENTIL_PIN, LOW);
  }else{
    digitalWrite(VENTIL_PIN, HIGH);
  }
  
  delay(1000);

}
