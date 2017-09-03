#include <Wire.h>
#include <SparkFunHTU21D.h>

#define BUZZPIN 3

#define DEVICE_ID "a089489a-2118-4f77-8631-8ed7f9080e26"
#define TEMP_SENSOR_ID "19fe8d6d-7204-47a9-8a61-6e7b8fb6ffad"
#define HUM_SENSOR_ID "6b43bcc4-bc68-4db9-9c77-777f5fed4229"
#define FLOOD_SENSOR_ID "8e94b41e-51f0-462e-9c47-7c3a4765baf8"

// enable DEBUG
#define IS_DEBUG false
// poll sensors with interval (seconds)
#define POLLING_INTERVAL 10
// number of pollings before send to server
#define POLLINGS_TO_SEND 30

#define TEMPERATURE_LO_THRESHOLD -50
#define TEMPERATURE_HI_THRESHOLD 100
#define HUMIDITY_LO_THRESHOLD 0
#define HUMIDITY_HI_THRESHOLD 100
#define WATER_ALARM_THRESHOLD 50

HTU21D htu21d;

int session_counter = 0;
float t_accumulator = 0;
int t_counter = 0;
float h_accumulator = 0;
int h_counter = 0;
boolean is_alarm = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(BUZZPIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT); 

  Serial.begin(9600);

  digitalWrite(LED_BUILTIN, HIGH); 
  Serial.println("HTU21D starting..."); 
  htu21d.begin(); 
  Serial.println("HTU21D initialized."); 
  digitalWrite(LED_BUILTIN, LOW); 
}

// Read sensors data, average them and print to console
// WiFi module send values from console to the server
void loop() {
  digitalWrite(LED_BUILTIN, HIGH); 

  Serial.println(millis());
  session_counter++;

  update_counter(TEMP_SENSOR_ID, htu21d.readTemperature(), t_accumulator, t_counter, TEMPERATURE_LO_THRESHOLD, TEMPERATURE_HI_THRESHOLD);
  update_counter(HUM_SENSOR_ID,  htu21d.readHumidity(), h_accumulator, h_counter, HUMIDITY_LO_THRESHOLD, HUMIDITY_HI_THRESHOLD);

  float water_level = analogRead(A0);
  boolean water_alarm = water_level > WATER_ALARM_THRESHOLD;
  logging("DEBUG", FLOOD_SENSOR_ID, water_level);

  if (water_alarm && !is_alarm) {
    logging("ALARM", FLOOD_SENSOR_ID, water_level);
    is_alarm = true;
  } else if (!water_alarm && is_alarm) {
    logging("INFO", FLOOD_SENSOR_ID, 0);
    is_alarm = false;
    noTone(BUZZPIN);
  }

  // send to server and flush counter
  if(session_counter >= POLLINGS_TO_SEND) {
    // todo: enable wifi
    // todo: wait for connect
    logging("INFO", TEMP_SENSOR_ID, t_accumulator / t_counter);
    logging("INFO", HUM_SENSOR_ID, h_accumulator / h_counter);
    // todo: shutdown wifi

    session_counter = 0;
    t_accumulator = 0;
    h_accumulator = 0;
    t_counter = 0;
    h_counter = 0;
  }
  
  if(is_alarm) {
    // beeping
    for(int i = 0; i < 5 * POLLING_INTERVAL; i++) {
      digitalWrite(LED_BUILTIN, LOW); 
      tone (BUZZPIN, 250); //250Hz
      delay(50);

      tone (BUZZPIN, 500); //500Hz
      delay(50);
        
      digitalWrite(LED_BUILTIN, HIGH); 
      tone(BUZZPIN, 1000); //1000Hz
      delay(100);
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW); 
    delay(POLLING_INTERVAL * 1000); 
  }
}

void update_counter(char* sensor_id, float value, float &accumulator, int &counter, float lo, float hi) {
  if(value >= lo && value <= hi) {
    logging("DEBUG", sensor_id, value);
    accumulator += value;
    counter++;
  } else {
    logging("DEBUG", sensor_id, value, "value not in range");
  }
}

void logging(char* level, char* sensor, float value) {
  logging(level, sensor, value, "");
}

void logging(char* level, char* sensor, float value, char* comment) {
  if(IS_DEBUG || strcmp(level, "DEBUG") != 0) {
    Serial.print(level); 
    Serial.print(":"); 
    Serial.print(DEVICE_ID);
    Serial.print(":"); 
    Serial.print(sensor);
    Serial.print(":"); 
    Serial.print(value, 2);
    Serial.print(":"); 
    Serial.print(comment); 
    Serial.println(); 
  }
}

