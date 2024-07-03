#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <StreamString.h>
#include <Servo.h>
#include <DHT.h>

#define DHTPIN D2     // pin koneksi dht
#define DHTTYPE DHT22 // tipe dht
#define buzzerpin D5 // pin koneksi buzzer

DHT dht(DHTPIN, DHTTYPE);
Servo servoon;
Servo servooff;

#ifndef STASSID
#define STASSID "adin"
#define STAPSK "********"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WebServer server(80);

const int led = 13;
bool servoMoved = false;

void handleRoot() {
  digitalWrite(led, 1);
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  float temperature = dht.readTemperature();

  StreamString temp;
  temp.reserve(500);  // Preallocate a large chunk to avoid memory fragmentation
  temp.printf("\
<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
      .button {\
        display: inline-block;\
        padding: 10px 20px;\
        font-size: 20px;\
        cursor: pointer;\
        text-align: center;\
        text-decoration: none;\
        outline: none;\
        color: #fff;\
        background-color: #4CAF50;\
        border: none;\
        border-radius: 15px;\
        box-shadow: 0 9px #999;\
      }\
      .button:hover {background-color: #3e8e41}\
      .button:active {\
        background-color: #3e8e41;\
        box-shadow: 0 5px #666;\
        transform: translateY(4px);\
      }\
    </style>\
  </head>\
  <body>\
    <a href='/on' class='button'> ON </a>\
    <a href='/off' class='button'> OFF </a>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>Temperature: %.2f &deg;C</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",
              hr, min % 60, sec % 60, temperature);
  server.send(200, "text/html", temp.c_str());
  digitalWrite(led, 0);


  // Gerakkan servo jika suhu di atas 30 derajat dan servo belum bergerak karena suhu
  if (temperature > 30 && !servoMoved) {
    servooff.write(0);
    delay(1000);
    servooff.write(90);
    servoMoved = true;
    for(int i = 0; i < 3; i++){
      tone(buzzerpin, HIGH);
      delay(100);
      noTone(buzzerpin);
      delay(100);
    }
  }
  // Penanda agar servo bisa kembali bergerak jika suhu turun kemudian naik lagi ke 30 derajat
  else if (temperature <= 30 && servoMoved) {
    servoMoved = false;
  }
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void drawGraph() {
  String out;
  out.reserve(2600);
  char temp[70];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send(200, "image/svg+xml", out);
}

void setup(void) {
  pinMode(led, OUTPUT);
  pinMode(buzzerpin, OUTPUT);
 
  servoon.attach(D4);
  servoon.write(90);
  servooff.attach(D3);
  servooff.write(90);
  dht.begin();

  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/on", []() {
    servoon.write(0);
    delay(1000);
    servoon.write(90);
    server.send(200, "text/plain", "switch is on");
    tone(buzzerpin, HIGH);
    delay(100);
    noTone(buzzerpin);
  });
  server.on("/off", []() {
    servooff.write(0);
    delay(1000);
    servooff.write(90);
    server.send(200, "text/plain", "switch is off");
    tone(buzzerpin, HIGH);
    delay(100);
    noTone(buzzerpin);
  });
  server.on("/test.svg", drawGraph);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();


}
