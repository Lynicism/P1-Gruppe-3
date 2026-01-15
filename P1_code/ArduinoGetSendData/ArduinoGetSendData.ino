#define TRIG_PIN 8 // Definerer hvilken Arduino-pin der bruges
#define ECHO_PIN 9
#define PUMPE_PIN 3 
#define VENTIL_PIN 2 
#define Bruger_IN 13

int distance; // Variabel til at gemme den aktuelle afstand målt af sensoren
long duration; // Variabel til at gemme tiden ultralydspulsen tager

int dist1; // Afstand før vand bliver brugt
int dist2; // Afstand efter vand er brugt
int distUsed; // Forskellen mellem dist1 og dist2
int volumeUsed; // Volumen af vand brugt i milliliter
int buttonSavedState = 0; // 1 - button is pressed, 0 - button is released


void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);   // Starter seriel kommunikation, så Arduino kan sende data til computeren
   pinMode(TRIG_PIN, OUTPUT); // Sætter forskellige pins som output og input
   pinMode(ECHO_PIN, INPUT);
   pinMode(PUMPE_PIN, OUTPUT);
   pinMode(VENTIL_PIN, OUTPUT);
   pinMode(Bruger_IN, INPUT); 
}

// Funktion der måler afstanden til vandet
int getDistance() {

  digitalWrite(TRIG_PIN, LOW);  // Sørger for at trigger-pinnen starter slukket
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);  // Sender et ultralydssignal
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH); // Måler hvor lang tid det tager før signalet kommer tilbage
  return duration * 0.0344 / 2; // distance in cm

}

void loop() {
  // put your main code here, to run repeatedly:


  distance = getDistance();   // Måler den aktuelle afstand til vandet

  bool buttonPressed = digitalRead(Bruger_IN);   // Læser om brugeren har trykket på knappen

  //Serial.println(distance);     //print distance from sencor to water


  if(buttonPressed) { // Hvis knappen er trykket
    if (buttonSavedState == 0){// Hvis knappen lige er blevet trykket
      //Serial.println("knappen on");
      dist1 = distance;      // Gemmer afstanden før vand bruges
      buttonSavedState = 1;      // Husk at knappen nu er trykket

    }
    digitalWrite(PUMPE_PIN, HIGH);// Tænder pumpen
  }
  else {
    //Serial.println("knappen off");     // Hvis knappen ikke er trykket
    digitalWrite(PUMPE_PIN, LOW);  // Slukker pumpen

    if (buttonSavedState == 1){  // Hvis knappen lige er blevet sluppet
      dist2 = distance; // Gemmer afstanden efter vand er brugt
      distUsed = dist2-dist1;   // Beregner forskellen i afstand
      volumeUsed = distUsed * 350;    // Omregner afstand til vandmængde i milliliter

      int Liters = volumeUsed / 1000;   // Omregner milliliter til liter

      Serial.println(Liters);  // Sender antallet af liter til computeren

      buttonSavedState = 0; // Nulstiller knap-status
    }  
  }

  if(distance <=15){ // Hvis vandstanden er meget høj (afstand <= 20 cm)
    digitalWrite(VENTIL_PIN, HIGH); // Åbner for ventilen
  }else{
    digitalWrite(VENTIL_PIN, LOW); // Lukker for ventilen
  }

  delay(1000);  // Venter et sekund før næste måling

}