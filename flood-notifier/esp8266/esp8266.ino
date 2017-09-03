#include <RestClient.h>

// heartbeat frequency in seconds
#define HEATBEAT 120
// turn off wifi after inactive period (seconds)
#define WIFI_KEEP_ALIVE 15

RestClient client = RestClient("192.168.1.65", 8080);
String console_line;

boolean is_wifi_enabled = false;
unsigned long wifi_last_used = 0; // milliseconds
unsigned long last_heartbeat = 0; // milliseconds


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

// send hearbeat every 5 minute
// send each log line once it read
void loop() {

  while (Serial.available() > 0) {
     char c = Serial.read(); // read the next char received
     // look for the newline character, this is the last character in the string
     if (c == '\n') {
       wifi_on();
       Serial.println("Sending line: "  + console_line);
       console_line = "";
     } else {
       console_line += c;
     }
  }

  // send hearbeat
  if(last_heartbeat == 0 || millis() - last_heartbeat > HEATBEAT * 1000 || millis() - last_heartbeat < 0) {
    heartbeat();
  }

  // disable wifi if its not used for idle time
  if (millis() - wifi_last_used > WIFI_KEEP_ALIVE * 1000) {
    wifi_off();
  }

  delay(100);
}


void heartbeat() {
  wifi_on();
  Serial.println("Sending hearbeat");
  last_heartbeat = millis();
//  client.get("/api/heartbeat");
}

void wifi_on() {
    if (!is_wifi_enabled) {
      Serial.println("Turning wifi on");
      WiFi.forceSleepWake();
      client.begin("home", "rfhfdtkkf");
      is_wifi_enabled = true;
    }
    wifi_last_used = millis();
}

void wifi_off() {
  if (is_wifi_enabled) {
    Serial.println("Turning wifi off");
    WiFi.forceSleepBegin();
    is_wifi_enabled = false;
  }
}


