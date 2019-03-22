/*
  WiFi UDP Send and Receive String

 This sketch wait an UDP packet on localPort using a WiFi shield.
 When a packet is received an Acknowledge packet is sent to the client on port remotePort

 Circuit:
 * WiFi shield attached

 created 30 December 2012
 by dlf (Metodo2 srl)

 */


#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>

int status = WL_IDLE_STATUS;

char ssid[] = "musical shoes";
char pass[] = "";

#define MSG_BUFFER_SIZE 128
char msgBuffer[MSG_BUFFER_SIZE];

WiFiUDP Udp;

unsigned int shoeId = 0;
//unsigned int shoeId = 1;

unsigned int localPort = 2390;
IPAddress serverIP(192, 168, 1, 1);

WiFiClient client;

bool accelPresent = false;

int fsrPin_toe = A2;
int fsrReading_toe;
bool toeDown = false;
int fsrToe_Threshold = 800;

int fsrPin_heel = A3;
int fsrReading_heel; 
bool heelDown = false;
int fsrHeel_Threshold = 800;

int flxPin = 4;
int flxReading;

/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Starting up...");

  /* Initialise the accelerometer */
  if(accel.begin())
  {
    accelPresent = true;
    displaySensorDetails();
  } else {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
  }

  // Adafruit Feather M0 Wifi pins
  WiFi.setPins(8,7,4,2);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid);

    // wait 5 seconds for connection:
    delay(5000);
  }
  Serial.println("Connected to wifi");
  printWiFiStatus();

  while (!client.connect(serverIP, localPort)) {
    Serial.println("connecting to server");
    delay(1000);
  }
  Serial.println("connected");
}

int count = 0;

bool aboveThreshold = false;
bool timedOut = false;
int triggerTime = 0;

int minMag = 0;
int maxMag = 0;
void loop() {
  // Reconnect to WiFi if it has disconnected
  if (status != WiFi.status()) {
    status = WiFi.status();
    while (status != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      status = WiFi.begin(ssid);
  
      // wait 5 seconds for connection:
      delay(5000);
    }
    while (!client.connect(serverIP, localPort)) {
      Serial.println("connecting to server");
      delay(1000);
    }
  }
  
  // Read from sensors
  // Acceleromoter
  float accelMag = 0.0f;
  sensors_event_t event;
  if (accelPresent) {
    accel.getEvent(&event);
    accelMag = sqrt(event.acceleration.x * event.acceleration.x + event.acceleration.y * event.acceleration.y + event.acceleration.z * event.acceleration.z);
  }
  /* Display the results (acceleration is measured in m/s^2) */
//  Serial.print("Accel reading = X: "); Serial.print(event.acceleration.x); Serial.print("  ");
//  Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
//  Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  ");Serial.println("m/s^2 ");

  int isKick = 0;
  if (event.acceleration.x > -1 && !aboveThreshold) {
    aboveThreshold = true;
    triggerTime = millis();
    timedOut = false;
  }
  if (aboveThreshold) {
    if (event.acceleration.x < -2) {
      if (!timedOut) {
//        Serial.println("KICK");
        isKick = 1;
      }
      aboveThreshold = false;
    }
    if (millis() - triggerTime > 200 && !timedOut) {
//      Serial.println("Resetting");
      timedOut = true;
    }
  }
  
  // Next, check if the reading on the FSR is above the threshold. 
  // If yes, send a "1" exactly 1 time
  // until the sensor drops back below the threshold
  // ie, only sends a 1 on the rising edge of the FSR
  
  // FSR Toe
  int fsrToe_signal = 0;
  fsrReading_toe = analogRead(fsrPin_toe);  
  if (fsrReading_toe > fsrToe_Threshold && !toeDown) {
    toeDown = true;
    fsrToe_signal = 1;
  }
  else if (fsrReading_toe < fsrToe_Threshold) {
    toeDown = false;
  }
  Serial.println(fsrReading_toe);

  // FSR Heel
  int fsrHeel_signal = 0;
  fsrReading_heel = analogRead(fsrPin_heel);  
  if (fsrReading_heel > fsrHeel_Threshold && !heelDown) {
    heelDown = true;
    fsrHeel_signal = 1;
  }
  else if (fsrReading_heel < fsrHeel_Threshold) {
    heelDown = false;
  }

//  Serial.print(fsrReading_toe);
//  Serial.print(", ");
//  Serial.println(fsrReading_heel);

  // Flex sensor (are we using this??)
  flxReading = analogRead(flxPin);

  // Send values to server in the format:
  // id,accelMag,toe_signal,heel_signal,flexSensor
  memset(msgBuffer, 0, MSG_BUFFER_SIZE);
  sprintf(msgBuffer, "%f,%d,%d,%d", accelMag, fsrToe_signal, fsrHeel_signal, isKick);
  Serial.println(fsrReading_heel);

  if (accelMag > maxMag) {
    maxMag = accelMag;
  }
  if (accelMag < minMag) {
    minMag = accelMag;
  }
//  Serial.print(minMag);
//  Serial.print(",");
//  Serial.println(maxMag);
  Serial.println(msgBuffer);

//  Udp.beginPacket(serverIP, localPort);
//  Udp.write(msgBuffer);
//  Udp.endPacket();
  delay(100);
  client.print(msgBuffer);
}


void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void displaySensorDetails(void)
{
  sensor_t sensor;
  accel.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" m/s^2");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" m/s^2");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" m/s^2");
  Serial.println("------------------------------------");
  Serial.println("");
  //delay(500);
}
