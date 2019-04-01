#include <Arduino.h>

#include <Taskscheduler.h>

#include <TinyGPS++.h>

#include <WiFi.h>

#include <WebServer.h>

#include <Arduinojson.h>

/*
   This sample sketch demonstrates the normal use of a TinyGPS++ (TinyGPSPlus)
   object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/
static const int RXPin = 12, TXPin = 15;
static const uint32_t GPSBaud = 9600;
void displayInfo();
// The TinyGPS++ object
TinyGPSPlus gps;

/* Put IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

// The serial connection to the GPS device
HardwareSerial GPS(1);

Scheduler userScheduler;

const char *ssid = "smavagpsserver";
const char *password = "smava1234";

float lat = 0;
float lon = 0;
int year = 0;
int month = 0;
int day = 0;
int hour = 0;
int minute = 0;
int second = 0;
long previusMillis = 0;
long now = 0;

Task gpsTask(100, TASK_FOREVER, []() {
  while (GPS.available() > 0) {
    if (gps.encode(GPS.read())) {
      lat = gps.location.lat();
      lon = gps.location.lng();
      year = gps.date.year();
      month = gps.date.month();
      day = gps.date.day();
      hour = gps.time.hour();
      minute = gps.time.minute();
      second = gps.time.second();
      if (millis() - previusMillis > 1000) {
        displayInfo();
        previusMillis = millis();
      }
    }
    if (millis() > 5000 && gps.charsProcessed() < 10) {
      Serial.println(F("No GPS detected: check wiring."));
    }
  }
});

void handle_OnConnect();

void setup() {
  Serial.begin(115200);
  GPS.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);

  Serial.println(F("Initializing GPS Server"));

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  server.on("/smava", handle_OnConnect);

  server.begin();
  Serial.println("HTTP server started");
  userScheduler.addTask(gpsTask);
  gpsTask.enable();
}

void loop() {
  userScheduler.execute();
  server.handleClient();
}

void handle_OnConnect() {
  const size_t capacity =
      JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(6);
  DynamicJsonDocument doc(capacity);

  doc["zoneId"] = "5ac0129e3e194204e0afef6d";
  doc["topic"] = "A2";

  JsonObject gps = doc.createNestedObject("gps");
  gps["lat"] = lat;
  gps["lon"] = lon;

  JsonObject time = doc.createNestedObject("time");
  hour -= 5;
  time["year"] = year;
  time["month"] = month;
  time["day"] = day;
  time["hour"] = hour-5;
  time["minute"] = minute;
  time["second"] = second;
  String payload;

  serializeJson(doc, payload);

  server.send(200, "text/html", payload);
}

void displayInfo() {

  const size_t capacity =
      JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(6);
  DynamicJsonDocument doc(capacity);

  doc["zoneId"] = "5ac0129e3e194204e0afef6d";
  doc["topic"] = "A2";

  JsonObject gps = doc.createNestedObject("gps");
  gps["lat"] = lat;
  gps["lon"] = lon;

  JsonObject time = doc.createNestedObject("time");
  time["year"] = year;
  time["month"] = month;
  time["day"] = day;
  time["hour"] = hour-5;
  time["minute"] = minute;
  time["second"] = second;

  serializeJsonPretty(doc, Serial);
  
}
