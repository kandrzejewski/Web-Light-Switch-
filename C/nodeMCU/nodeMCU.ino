#include <ss.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>

#define HTTP_PORT 80
#define RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50
#define DHTTYPE DHT11

IPAddress ip(192, 168, 0, 101);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

struct Relay {
    byte id;
    byte gpio;
    bool status;
}   relay;
struct Sensor {
    byte id;
    byte gpio = 4;
    byte temperature;
    byte oldTemperature;
    byte humidity;
    byte oldHumidity;
}   sensor;
struct Button {
    byte id;
    byte gpio;
    bool state;
    bool block;
}   button;

const char* wifi_ssid = "Embeded113";
const char* wifi_passwd = "Embeded113";

DHT dht(sensor.gpio, DHTTYPE);
WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer http_rest_server(HTTP_PORT);

void InitResource() {
    relay.id = 1;
    relay.gpio = 2;
    relay.status = HIGH;
    sensor.id = 1;
    sensor.gpio = 4;
    sensor.temperature = 0;
    sensor.oldTemperature = 0;
    sensor.humidity = 0;
    sensor.oldHumidity = 0;
    button.id = 1;
    button.gpio = 0;
    button.state = false;
    button.block = false;
}

int InitWifi() {
    int retries = 0;
    Serial.print("Connecting to WiFi");
    WiFi.mode(WIFI_STA);
    WiFi.config(ip, gateway, subnet);
    WiFi.begin(wifi_ssid, wifi_passwd);
    while ((WiFi.status() != WL_CONNECTED) && (retries < MAX_WIFI_INIT_RETRY)) {
        retries++;
        delay(RETRY_DELAY);
        Serial.print(".");
    }
    return WiFi.status();
}

void GetTempValue(){
  sensor.temperature = dht.readTemperature();
}

void GetHumidityValue(){
  sensor.humidity = dht.readHumidity();
}

void GetIndex() {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& main = jsonBuffer.createObject();
    char JSONmessageBuffer[400];
  
  GetTempValue(); GetHumidityValue();
   
  JsonObject& sensors = main.createNestedObject("sensors");
    JsonObject& dth = sensors.createNestedObject("dth");
      dth["id"] = sensor.id;
      dth["gpio"] = sensor.gpio;
      dth["temperature"] = sensor.temperature;
      dth["humidity"] = sensor.humidity;
    JsonObject& actuators = main.createNestedObject("actuators");
    JsonObject& buttonJSON = actuators.createNestedObject("button");
      buttonJSON["id"] = button.id;
      buttonJSON["gpio"] = button.gpio;
      buttonJSON["state"] = button.state;
    JsonObject& relayJSON = actuators.createNestedObject("relay");
      relayJSON["id"] = relay.id;
      relayJSON["gpio"] = relay.gpio;
      relayJSON["status"] = relay.status;
    
  main.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    http_rest_server.send(200, "application/json", JSONmessageBuffer);
}
void GetSensors() {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& main = jsonBuffer.createObject();
    char JSONmessageBuffer[200];
    GetTempValue(); GetHumidityValue();
    JsonObject& dth = main.createNestedObject("dth");
      dth["id"] = sensor.id;
      dth["gpio"] = sensor.gpio;
      dth["temperature"] = sensor.temperature;
      dth["humidity"] = sensor.humidity;
      
    main.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    http_rest_server.send(200, "application/json", JSONmessageBuffer);
}
void GetDTH() {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& main = jsonBuffer.createObject();
    char JSONmessageBuffer[200];
    GetTempValue(); GetHumidityValue();
  if (sensor.id == 0)
        http_rest_server.send(204);
    else {
      GetTempValue(); GetHumidityValue();
      
      main["id"] = sensor.id;
      main["gpio"] = sensor.gpio;
      main["temperature"] = sensor.temperature;
      main["humidity"] = sensor.humidity;
        
      main.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
      http_rest_server.send(200, "application/json", JSONmessageBuffer);
    }
}
void GetActuators() {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& main = jsonBuffer.createObject();
    char JSONmessageBuffer[200];
    
    JsonObject& buttonJSON = main.createNestedObject("button");
      buttonJSON["id"] = button.id;
      buttonJSON["gpio"] = button.gpio;
      buttonJSON["state"] = button.state;
    JsonObject& relayJSON = main.createNestedObject("relay");
      relayJSON["id"] = relay.id;
      relayJSON["gpio"] = relay.gpio;
      relayJSON["status"] = relay.status;
      
    main.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    http_rest_server.send(200, "application/json", JSONmessageBuffer);
}
void GetButton() {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& main = jsonBuffer.createObject();
    char JSONmessageBuffer[200];
    
    if (sensor.id == 0)
        http_rest_server.send(204);
    else {
    main["id"] = button.id;
    main["gpio"] = button.gpio;
    main["state"] = button.state;
    
    main.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    http_rest_server.send(200, "application/json", JSONmessageBuffer);
  }
}
void GetRelay() {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& main = jsonBuffer.createObject();
    char JSONmessageBuffer[200];
    if (sensor.id == 0)
        http_rest_server.send(204);
    else {
    main["id"] = relay.id;
    main["gpio"] = relay.gpio;
    main["status"] = relay.status;
    
    main.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    http_rest_server.send(200, "application/json", JSONmessageBuffer);
  }
}


void FromJsonToRelay(JsonObject& jsonBody) {
    relay.id = jsonBody["id"];
    relay.gpio = jsonBody["gpio"];
    relay.status = jsonBody["status"];
}

void PostPutRealy() {
    DynamicJsonBuffer jsonBuffer;
    String post_body = http_rest_server.arg("plain");
    Serial.println(post_body);

    JsonObject& jsonBody = jsonBuffer.parseObject(http_rest_server.arg("plain"));
    if (!jsonBody.success()) {
        Serial.println("error in parsin json body");
        http_rest_server.send(400);
    }
    else {   
        if (http_rest_server.method() == HTTP_POST) {
            if ((jsonBody["id"] != 0) && (jsonBody["id"] != relay.id)) {
                FromJsonToRelay(jsonBody);
                http_rest_server.sendHeader("Location", "/relay/" + String(relay.id));
                http_rest_server.send(201);
                pinMode(relay.gpio, OUTPUT);
            }
            else if (jsonBody["id"] == 0)
              http_rest_server.send(404);
            else if (jsonBody["id"] == relay.id)
              http_rest_server.send(409);
        }
        else if (http_rest_server.method() == HTTP_PUT) {
            if (jsonBody["id"] == relay.id) {
                FromJsonToRelay(jsonBody);
                http_rest_server.sendHeader("Location", "/relay/" + String(relay.id));
                http_rest_server.send(200);
                digitalWrite(relay.gpio, relay.status);
            }
            else
              http_rest_server.send(404);
        }
    }
}

void ConfigRouting() {
  http_rest_server.on("/", HTTP_GET, GetIndex);
  http_rest_server.on("/sensors", HTTP_GET, GetSensors);
  http_rest_server.on("/sensors/dth", HTTP_GET, GetDTH);
  http_rest_server.on("/actuators", HTTP_GET, GetActuators);
  http_rest_server.on("/actuators/button", HTTP_GET, GetButton);
  http_rest_server.on("/actuators/relay", HTTP_GET, GetRelay);
  http_rest_server.on("/actuators/relay", HTTP_POST, PostPutRealy);
  http_rest_server.on("/actuators/relay", HTTP_PUT, PostPutRealy);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
   if (type == WStype_TEXT){
    for(int i = 0; i < length; i++) Serial.print((char) payload[i]);
    Serial.println();
   }
}

void socketSendData(String  quantities, byte value){
  DynamicJsonBuffer jsonBuffer(128);
  JsonObject& root = jsonBuffer.createObject();
  root["quantities"] = quantities;
  root["value"] = value;
  StreamString databuf;
  root.printTo(databuf);
  webSocket.broadcastTXT(databuf);  
}

void setup(void) {
    Serial.begin(115200); 
    InitResource();
    
    pinMode(button.gpio, INPUT_PULLUP);   
    pinMode(relay.gpio, OUTPUT);

    dht.begin();
  
    if (InitWifi() == WL_CONNECTED) {
        Serial.println("Connetted to ");
        Serial.print(wifi_ssid);
        Serial.print("--- IP: ");
        Serial.println(WiFi.localIP());
    }
    else {
        Serial.print("Error connecting to: ");
        Serial.println(wifi_ssid);
    }
  
    ConfigRouting();

    http_rest_server.begin();
    Serial.println("HTTP REST Server Started");


    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void loop(void) {
    http_rest_server.handleClient();
    if (digitalRead(button.gpio) == LOW) {
      if (button.block == false) {
        relay.status = !relay.status;
        digitalWrite(relay.gpio, relay.status);
        button.state = !button.state;
        button.block = true;
      }
    }
    else {
      button.block = false;
    }
    
    webSocket.loop();

    GetTempValue(); GetHumidityValue();
    if (sensor.temperature != sensor.oldTemperature) {
      socketSendData("temperature", sensor.temperature);
      sensor.oldTemperature = sensor.temperature;
    }
      
   if (sensor.humidity != sensor.oldHumidity){
    socketSendData("humidity", sensor.humidity);
    sensor.oldHumidity = sensor.humidity;
  }
}
