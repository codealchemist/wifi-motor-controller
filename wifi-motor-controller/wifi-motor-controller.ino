/*
  NodeMCU pinout:
  - D1/GPIO5 : A4988.MS1
  - D2/GPIO4 : A4988.MS2
  - D3/GPIO0 : A4988.MS3
  - D5/GPIO14: A4988.STEP
  - D6/GPIO12: A4988.DIR
  - G        : A4988.GND
  - 3V       : A4988.VDD
*/
#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include "AccelStepper.h"
#include "motor.h"

AsyncWebServer server(80);
int DIR_PIN = 12;
int STEP_PIN = 14;
int MS1 = 5;
int MS2 = 4;
int MS3 = 0;
AccelStepper motor(1, STEP_PIN, DIR_PIN);
MotorHelper motorHelper(DIR_PIN, STEP_PIN, 200);

// Wifi config.
const char* ssid = "moon.deco";
const char* password = "----heladera nueva----";

void moveDegrees(int degrees) {
  Serial.println("moveDegrees: " + String(degrees));
  int steps = degrees * 200 / 360;

  Serial.println("Move steps: " + String(steps));
  motor.moveTo(steps);
  Serial.println("Movement completed!");
}

int getDegreesPosition() {
  int steps = motor.currentPosition();
  int degrees = steps * 360 / 200;
  return degrees;
}

void setFullStepResolution() {
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(MS3, LOW);
}

void setHalfStepResolution() {
  digitalWrite(MS1, HIGH);
  digitalWrite(MS2, LOW);
  digitalWrite(MS3, LOW);
}

void setQuarterStepResolution() {
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, LOW);
}

void setEightStepResolution() {
  digitalWrite(MS1, HIGH);
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, LOW);
}

void setSixteenthStepResolution() {
  digitalWrite(MS1, HIGH);
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, HIGH);
}

void resetConfig() {
  motor.setMaxSpeed(1000);
  motor.setAcceleration(60);
  motor.setSpeed(400);
  motor.setCurrentPosition(0);
}

//------------------------------------------------------------------------
/*
  Run mode:
  - off: motor.run() is not called.
  - normal
  - test
  - right
  - left
  - funky
*/
String runMode = "normal";

void normalMode() {
  motor.run();
}

void testMode() {
  if (motor.distanceToGo() == 0) {
    motor.moveTo(-motor.currentPosition());
  }
  motor.run();
}

void rightMode() {
  if (motor.distanceToGo() == 0) {
    motor.move(360);
  }
  motor.run();
}

void leftMode() {
  if (motor.distanceToGo() == 0) {
    motor.move(-360);
  }
  motor.run();
}

void funkyMode() {
  int acceleration = random(10, 200);
  int speed = random(10, 200);
  int maxSpeed = random(50, 1000);
  motor.setAcceleration(acceleration);
  motor.setSpeed(speed);
  motor.setMaxSpeed(maxSpeed);
  
  int currentDegrees = getDegreesPosition();
  if (motor.distanceToGo() == 0) {
    if (currentDegrees >= 90) {
      motor.moveTo(0);
    } else {
      motor.moveTo(90);
    }
  }
  motor.run();
}
//------------------------------------------------------------------------

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void printLine () {
  Serial.println();
  Serial.println("--------------------------------------------------------------------------------");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println("SETUP...");
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n");
    return;
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Motor setup.
  motor.setMaxSpeed(1000);
  motor.setAcceleration(60);
  motor.setSpeed(400);
  motor.setCurrentPosition(0);

  // Microstep resolution setup.
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);

  // JSON API status.
  AsyncCallbackJsonWebHandler* apiStatus = new AsyncCallbackJsonWebHandler("/api/status", [](AsyncWebServerRequest *request, JsonVariant &json) {
    Serial.println("-> /api/status");
    
    // JSON response.
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonResponse(1024);
    jsonResponse["heap"] = ESP.getFreeHeap();
    jsonResponse["ssid"] = WiFi.SSID();
    int positionDegrees = getDegreesPosition();
    int positionSteps = motor.currentPosition();
    Serial.println("positionDegrees: " + String(positionDegrees));
    Serial.println("positionSteps: " + String(positionSteps));
    jsonResponse["positionDegrees"] = positionDegrees;
    jsonResponse["positionSteps"] = positionSteps;
    jsonResponse["mode"] = runMode;
    serializeJson(jsonResponse, Serial);
    serializeJson(jsonResponse, *response);
    printLine();
    request->send(response);
  });
  server.addHandler(apiStatus);

  // JSON API move degrees.
  AsyncCallbackJsonWebHandler* apiMove = new AsyncCallbackJsonWebHandler("/api/move", [](AsyncWebServerRequest *request, JsonVariant &json) {  
    Serial.println("-> /api/move");

    // Error.
    if (not json.is<JsonObject>()) {
      request->send(400, "text/plain", "Invalid request. Expected JSON body.");
      return;
    }

    // Read request data.
    StaticJsonDocument<200> data;
    data = json.as<JsonObject>();
    String requestData;
    serializeJson(data, requestData);
    Serial.println(requestData);
    int degrees = data["degrees"].as<int>();
    int acceleration = data["acceleration"].as<int>();
    int speed = data["speed"].as<int>();
    int maxSpeed = data["maxSpeed"].as<int>();
    int resolution = data["resolution"].as<int>();

    // Apply microstep resolution.
    if (resolution) {
      if (resolution == 1) setFullStepResolution();
      if (resolution == 2) setHalfStepResolution();
      if (resolution == 3) setEightStepResolution();
      if (resolution == 4) setSixteenthStepResolution();
    }

    // Config.
    if (acceleration) {
      motor.setAcceleration(acceleration);
    }
    if (speed) {
      motor.setSpeed(speed);
    }
    if (maxSpeed) {
      motor.setMaxSpeed(maxSpeed);
    }

    // Move.
    moveDegrees(degrees);

    // JSON response.
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonResponse(1024);
    jsonResponse["received"] = true;
    serializeJson(jsonResponse, *response);
    printLine();
    request->send(response);
  });
  server.addHandler(apiMove);

  // JSON API config.
  AsyncCallbackJsonWebHandler* apiConfig = new AsyncCallbackJsonWebHandler("/api/config", [](AsyncWebServerRequest *request, JsonVariant &json) {  
    Serial.println("-> /api/move");

    // Error.
    if (not json.is<JsonObject>()) {
      request->send(400, "text/plain", "Invalid request. Expected JSON body.");
      return;
    }

    // Read request data.
    StaticJsonDocument<200> data;
    data = json.as<JsonObject>();
    String requestData;
    serializeJson(data, requestData);
    Serial.println(requestData);
    int acceleration = data["acceleration"].as<int>();
    int speed = data["speed"].as<int>();
    int maxSpeed = data["maxSpeed"].as<int>();
    int resolution = data["resolution"].as<int>();

    // Apply microstep resolution.
    if (resolution) {
      if (resolution == 1) setFullStepResolution();
      if (resolution == 2) setHalfStepResolution();
      if (resolution == 3) setEightStepResolution();
      if (resolution == 4) setSixteenthStepResolution();
    }

    // Apply config.
    if (acceleration) {
      motor.setAcceleration(acceleration);
    }
    if (speed) {
      motor.setSpeed(speed);
    }
    if (maxSpeed) {
      motor.setMaxSpeed(maxSpeed);
    }

    // JSON response.
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonResponse(1024);
    jsonResponse["received"] = true;
    serializeJson(jsonResponse, *response);
    printLine();
    request->send(response);
  });
  server.addHandler(apiConfig);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "It's working! Yay! 🎉");
  });

  // Send a GET request to <IP>/get?message=<message>
  server.on("/status", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String message;
    request->send(200, "text/plain", "STATUS: moved degrees: " + getDegreesPosition());
  });

  server.on("/test", HTTP_GET, [] (AsyncWebServerRequest *request) {
    moveDegrees(360);
    if (motor.currentPosition() == 360) {
      moveDegrees(0);
    }
    request->send(200, "text/plain", "TEST: OK");
  });

  server.on("/mode/off", HTTP_GET, [] (AsyncWebServerRequest *request) {
    runMode = "off";
    resetConfig();
    request->send(200, "text/plain", "TEST: OK");
  });

  server.on("/mode/normal", HTTP_GET, [] (AsyncWebServerRequest *request) {
    runMode = "normal";
    resetConfig();
    request->send(200, "text/plain", "TEST: OK");
  });

  server.on("/mode/test", HTTP_GET, [] (AsyncWebServerRequest *request) {
    runMode = "test";
    request->send(200, "text/plain", "TEST: OK");
  });

  server.on("/mode/right", HTTP_GET, [] (AsyncWebServerRequest *request) {
    runMode = "right";
    request->send(200, "text/plain", "TEST: OK");
  });

  server.on("/mode/left", HTTP_GET, [] (AsyncWebServerRequest *request) {
    runMode = "right";
    request->send(200, "text/plain", "TEST: OK");
  });

  server.on("/mode/funky", HTTP_GET, [] (AsyncWebServerRequest *request) {
    runMode = "funky";
    request->send(200, "text/plain", "TEST: OK");
  });

  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
    String degrees;
    if (request->hasParam("degrees", true)) {
      degrees = request->getParam("degrees", true)->value();
    } else {
      request->send(500, "text/plain", "Param 'degrees' is required!");
      return;
    }

    // Move degrees.
    Serial.print("Move degrees:");
    Serial.println(degrees);
    request->send(200, "text/plain", "Move degrees: " + degrees);
    moveDegrees(degrees.toInt());
  });

  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  if (runMode == "normal") normalMode();
  if (runMode == "test") testMode();
  if (runMode == "right") rightMode();
  if (runMode == "left") leftMode();
  if (runMode == "funky") funkyMode();
}
