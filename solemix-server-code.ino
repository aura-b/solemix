#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>

char ssid[] = "musical shoes";        // your network SSID (name)
char pass[] = "";    // your network password (use for WPA, or use as key for WEP)

unsigned int leftPort = 2390;
unsigned int rightPort = 2391;

WiFiServer leftServer(leftPort);
WiFiServer rightServer(rightPort);

int led =  LED_BUILTIN;
int status = WL_IDLE_STATUS;
WiFiUDP leftUdp;
WiFiUDP rightUdp;

#define MSG_BUFFER_SIZE 128
char rightMsgBuffer[MSG_BUFFER_SIZE] = "0.0,0,0,0";
char leftMsgBuffer[MSG_BUFFER_SIZE] = "0.0,0,0,0";

WiFiClient leftClient;
WiFiClient rightClient;

bool leftConnected = false;
bool rightConnected = false;

int lastLeftRx = 0;
int lastRightRx = 0;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  delay(1000);

  //Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8,7,4,2);
  
  Serial.println("Access Point Server");

  pinMode(led, OUTPUT);      // set the LED pin mode

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // print the network name (SSID);
  Serial.print("Creating access point named: ");
  Serial.println(ssid);

  // Create open network. Change this line if you want to create an WEP network:
  status = WiFi.beginAP(ssid);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    // don't continue
    while (true);
  }

  // wait 10 seconds for connection:
  delay(5000);

  Serial.print("Listening on UDP port ");
  Serial.print(leftPort);
  Serial.print(" and ");
  Serial.println(rightPort);
  
  leftUdp.begin(leftPort);
  rightUdp.begin(rightPort);

  leftServer.begin();
  rightServer.begin();

  // you're connected now, so print out the status
  printWiFiStatus();
}


void loop() {
  if (status != WiFi.status()) {
    // it has changed update the variable
    status = WiFi.status();

    if (status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      leftServer.begin();
      rightServer.begin();
      Serial.println("Device connected to AP");
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      Serial.println("Device disconnected from AP");
    } 
  }
  
  WiFiClient newClient = leftServer.available();
  if (newClient) {
    if (!leftConnected) {
      Serial.println("Left shoe connected");
      leftClient = newClient;
      lastLeftRx = millis();
      leftConnected = true;
    }
    // check for connection timeout
    int dataLen = leftClient.available();
    if (dataLen > 0) {
      uint8_t* msg = (uint8_t*)malloc(dataLen);
      int len = leftClient.read(msg, dataLen);
      memcpy(leftMsgBuffer, msg, dataLen);
      if (len > 0) leftMsgBuffer[len] = 0;
      lastLeftRx = millis();
    }
  }

  newClient = rightServer.available();
  if (newClient) {
    if (!rightConnected) {
      Serial.println("Right shoe connected");
      rightClient = newClient;
      lastRightRx = millis();
      rightConnected = true;
    }
    // check for connection timeout
    int dataLen = rightClient.available();
    if (dataLen > 0) {
      uint8_t* msg = (uint8_t*)malloc(dataLen);
      int len = rightClient.read(msg, dataLen);
      memcpy(rightMsgBuffer, msg, dataLen);
      if (len > 0) rightMsgBuffer[len] = 0;
      lastRightRx = millis();
    }
  }
  if (rightConnected && (millis() - lastRightRx) > 1000) {
    Serial.println("Right shoe disconnected");
    rightConnected = false;
    rightClient.stop();
    rightServer.begin();
  }

  if (leftConnected && (millis() - lastLeftRx) > 1000) {
    Serial.println("Left shoe disconnected");
    leftConnected = false;
    leftClient.stop();
  }

  Serial.print(leftMsgBuffer);
  Serial.print(",");
  Serial.println(rightMsgBuffer);

  delay(100);
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
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);

}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}
