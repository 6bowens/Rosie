#include <blynk.h>

char auth[] = "";
int counter = 0;
int pubInt = 60;
int state, level, capacity, temp, batpercent, distance, current, voltage, angle;
char sensorbytes[26];

BLYNK_WRITE(V1){ if (param.asInt() == 1) dock("0");}
BLYNK_WRITE(V2){ if (param.asInt() == 1) clean("0");}
BLYNK_WRITE(V3){ if (param.asInt() == 1) maxClean("0");}
BLYNK_WRITE(V4){ if (param.asInt() == 1) spotClean("0");}
BLYNK_WRITE(V5){ if (param.asInt() == 1) sleep("0");}
BLYNK_WRITE(V6){clean("0");}

String stringState;
WidgetLCD lcd(V0);

void setup() {
    
    //Enable the USB serial port for debug messages.
    //Serial.begin(115200);
    //Serial.println("Firing up Rosie!");
    Particle.variable("state", &state, INT);
    Particle.variable("level", &level, INT);
    Particle.variable("temp", &temp, INT);
    Particle.variable("capacity", &capacity, INT);
    
    Particle.function("dock", dock);
    Particle.function("clean", clean);
    Particle.function("maxClean", maxClean);
    Particle.function("spotClean", spotClean);
    Particle.function("sleep", sleep);
    
    //Setup Roomba serial comm
    Serial1.begin(115200);

    Blynk.begin(auth, "Server Address");
    delay(3000); //why not
    
    for (int i = 0; i < 26; i++) {
        sensorbytes[i] = 0;
    }
    
}

void loop() {
    
    Blynk.run(); 
    
    if ((millis() - counter) > (pubInt*1000) ){
        updateSensors(); //run byte dump
        delay(100);
        double lev = level;
        double cap = capacity;
        batpercent = (lev/cap*100);
        
        Blynk.virtualWrite(V10, batpercent);
        Blynk.virtualWrite(V11, temp);
        
        lcd.clear();
        if (state == 0) lcd.print(0,0,"Not Charging");
        else if (state == 1) lcd.print(0,0,"Charging Recovery");
        else if (state == 2) lcd.print(4,0,"Charging");
        else if (state == 3) lcd.print(0,0,"Trickle Charging");
        else if (state == 4) lcd.print(3,0,"Not Docked");
        else if (state == 5) lcd.print(0,0,"Charging Error");
        
/*      String stringTemp = String(temp);
        String stringState = String(state);
        String stringPercent = String(batpercent);
        
        Particle.publish("state", stringState);
        Particle.publish("temp", stringTemp);
        Particle.publish("percent", stringPercent);
        */
        counter = millis();
    }
    
    delay(100);
}


int dock(String command) {
    Serial1.write(128);
    delay(1000);
    Serial1.write(143);
    delay(3000);        // double print when active
    Serial1.write(128);
    delay(1000);
    Serial1.write(143);
    Particle.publish("action", "docking");
    return 0;
}

int clean(String command) {
    Serial1.write(128);
    delay(1000);
    Serial1.write(135);
    delay(100);
    Particle.publish("action", "cleaning");
    return 0;
}

int maxClean(String command) {
    Serial1.write(128);
    delay(1000);
    Serial1.write(136);
    delay(100);
    return 0;
}

int spotClean(String command) {
    Serial1.write(128);
    delay(1000);
    Serial1.write(134);
    delay(100);
    return 0;
}

int sleep(String command) {
    Serial1.write(128);
    delay(1000);
    Serial1.write(133);
    delay(100);
    return 0;
}

// Requests a sensor update from the Roomba.
void updateSensors() {                              
    
    Serial1.write(142);
    Serial1.write(0); // sensor packet 0, 26 bytes
    delay(100);                                       
    int i = 0;
    for (int i = 0; i < 26; i++) {
        sensorbytes[i] = (uint8_t) Serial1.read(); //decode byte dump
    }
    
    temp = sensorbytes[21]; // roomba sci document
    state = sensorbytes[16];
    level = ((sensorbytes[22] & 0xFF) << 8) + (sensorbytes[23] & 0xFF);
    capacity = ((sensorbytes[24] & 0xFF) << 8) + (sensorbytes[25] & 0xFF);
}