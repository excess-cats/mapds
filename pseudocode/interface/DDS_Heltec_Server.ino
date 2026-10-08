#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include "web_assets.h"

/*
  DDI HELTEC LOCAL WEB SERVER

  Board:
    Heltec WiFi LoRa 32 V3 / ESP32-S3

  Network:
    Heltec joins a phone hotspot.
    Cockpit laptop/tablet joins the same hotspot.
    Open the IP printed in Serial Monitor.
    If mDNS works on the hotspot, http://ddi.local/ also works.

  This file currently uses a lightweight hardware simulation so the complete
  Wi-Fi -> web server -> API -> interface chain can be tested before sensors,
  servo, and LoRa data are connected.
*/

// -----------------------------------------------------------------------------
// WIFI SETTINGS
// -----------------------------------------------------------------------------

const char* WIFI_SSID     = "Aliphone";
const char* WIFI_PASSWORD = "12345678";
const char* DDI_MDNS_NAME = "ddi";

WebServer ddiServer(80);

// -----------------------------------------------------------------------------
// TEMPORARY HELTEC-SIDE HARDWARE SIMULATION
// Replace these values later with the real launcher / LoRa / sensor data.
// -----------------------------------------------------------------------------

enum class DdsState {
  STANDBY,
  LOADING,
  LOADED,
  ARMING,
  ARMED,
  DEPLOYING,
  DEPLOYED,
  FAULT
};

DdsState systemState = DdsState::STANDBY;

bool dcmPresent = false;

const char* lockState = "NOT_LOADED";
const char* mechanicalCheck = "NOT_RUN";
const char* softwareCheck = "NOT_RUN";

float batteryPercent = 91.0f;
float batteryVoltage = 11.8f;

float latitude = 40.7934f;
float longitude = -77.8600f;
float altitude = 127.4f;

unsigned long transitionStarted = 0;

// -----------------------------------------------------------------------------
// STATE HELPERS
// -----------------------------------------------------------------------------

const char* stateText() {
  switch (systemState) {
    case DdsState::STANDBY:   return "STANDBY";
    case DdsState::LOADING:   return "LOADING";
    case DdsState::LOADED:    return "LOADED";
    case DdsState::ARMING:    return "ARMING";
    case DdsState::ARMED:     return "ARMED";
    case DdsState::DEPLOYING: return "DEPLOYING";
    case DdsState::DEPLOYED:  return "DEPLOYED";
    case DdsState::FAULT:     return "FAULT";
  }
  return "FAULT";
}

void updateStateMachine() {
  const unsigned long now = millis();

  if (systemState == DdsState::LOADING &&
      now - transitionStarted >= 3000) {
    dcmPresent = true;
    lockState = "LOADED";
    systemState = DdsState::LOADED;
  }

  if (systemState == DdsState::ARMING &&
      now - transitionStarted >= 700) {
    mechanicalCheck = "PASS";
    softwareCheck = "PASS";
    systemState = DdsState::ARMED;
  }

  if (systemState == DdsState::DEPLOYING &&
      now - transitionStarted >= 1000) {
    dcmPresent = false;
    lockState = "DEPLOYED";
    systemState = DdsState::DEPLOYED;
  }
}

// -----------------------------------------------------------------------------
// DDI WEB FILES
// -----------------------------------------------------------------------------

void sendDdiFlashFile(const char* contentType, const char* data) {
  ddiServer.sendHeader("Cache-Control", "no-store");
  ddiServer.send_P(200, contentType, data);
}

void handleDdiRoot() {
  sendDdiFlashFile("text/html; charset=utf-8", WEB_MAIN_HTML);
}

void handleDdiCss() {
  sendDdiFlashFile("text/css; charset=utf-8", WEB_COCKPIT_CSS);
}

void handleDdiJs() {
  sendDdiFlashFile("application/javascript; charset=utf-8", WEB_INTERFACE_JS);
}

// -----------------------------------------------------------------------------
// DDI API
// -----------------------------------------------------------------------------

void handleDdiStatus() {
  updateStateMachine();

  String json;
  json.reserve(360);

  json += F("{\"state\":\"");
  json += stateText();

  json += F("\",\"lock\":\"");
  json += lockState;

  json += F("\",\"dcmPresent\":");
  json += dcmPresent ? F("true") : F("false");

  json += F(",\"mechanical\":\"");
  json += mechanicalCheck;

  json += F("\",\"software\":\"");
  json += softwareCheck;

  json += F("\",\"battery\":{\"percent\":");
  json += String(batteryPercent, 0);

  json += F(",\"voltage\":");
  json += String(batteryVoltage, 1);

  json += F("},\"gps\":{\"lat\":");
  json += String(latitude, 4);

  json += F(",\"lon\":");
  json += String(longitude, 4);

  json += F(",\"altitude\":");
  json += String(altitude, 1);

  json += F("}}");

  ddiServer.sendHeader("Cache-Control", "no-store");
  ddiServer.send(200, "application/json", json);
}

void sendDdiCommandResult(bool ok, const char* message) {
  String json;
  json.reserve(120);

  json += F("{\"ok\":");
  json += ok ? F("true") : F("false");
  json += F(",\"message\":\"");
  json += message;
  json += F("\"}");

  ddiServer.send(ok ? 200 : 409, "application/json", json);
}

void handleDdiCommand() {
  updateStateMachine();

  const String body = ddiServer.arg("plain");

  if (body.indexOf("\"LOAD\"") >= 0) {
    if (systemState == DdsState::DEPLOYED) {
      sendDdiCommandResult(false, "LOAD rejected. DCM has already been deployed.");
      return;
    }

    if (systemState != DdsState::STANDBY) {
      sendDdiCommandResult(false, "LOAD rejected. DDS is not in standby.");
      return;
    }

    dcmPresent = false;
    lockState = "UNLOCKED";
    mechanicalCheck = "NOT_RUN";
    softwareCheck = "NOT_RUN";

    systemState = DdsState::LOADING;
    transitionStarted = millis();

    sendDdiCommandResult(true, "LOAD accepted.");
    return;
  }

  if (body.indexOf("\"ARM\"") >= 0) {
    if (systemState == DdsState::DEPLOYED) {
      sendDdiCommandResult(false, "ARM rejected. DCM has already been deployed.");
      return;
    }

    if (systemState != DdsState::LOADED || !dcmPresent) {
      sendDdiCommandResult(false, "ARM rejected. LOAD must complete first.");
      return;
    }

    mechanicalCheck = "RUNNING";
    softwareCheck = "RUNNING";

    systemState = DdsState::ARMING;
    transitionStarted = millis();

    sendDdiCommandResult(true, "ARM accepted.");
    return;
  }

  if (body.indexOf("\"DEPLOY\"") >= 0) {
    if (systemState == DdsState::DEPLOYED) {
      sendDdiCommandResult(false, "DEPLOY rejected. DCM has already been deployed.");
      return;
    }

    if (systemState != DdsState::ARMED ||
        strcmp(mechanicalCheck, "PASS") != 0 ||
        strcmp(softwareCheck, "PASS") != 0) {
      sendDdiCommandResult(false, "DEPLOY rejected. DDS is not armed and healthy.");
      return;
    }

    lockState = "UNLOCKED";
    systemState = DdsState::DEPLOYING;
    transitionStarted = millis();

    sendDdiCommandResult(true, "DEPLOY accepted.");
    return;
  }

  ddiServer.send(400, "application/json",
                 "{\"ok\":false,\"message\":\"Unknown command.\"}");
}

// -----------------------------------------------------------------------------
// DDI WIFI / SERVER
// -----------------------------------------------------------------------------

void connectToHotspot() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to phone hotspot");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected.");
  Serial.print("DDI local address: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void startDdiMdns() {
  if (MDNS.begin(DDI_MDNS_NAME)) {
    MDNS.addService("http", "tcp", 80);
    Serial.print("DDI mDNS address: http://");
    Serial.print(DDI_MDNS_NAME);
    Serial.println(".local/");
  } else {
    Serial.println("DDI mDNS unavailable. Use the IP address above.");
  }
}

void startDdiWebServer() {
  ddiServer.on("/", HTTP_GET, handleDdiRoot);
  ddiServer.on("/main.html", HTTP_GET, handleDdiRoot);
  ddiServer.on("/cockpit_style.css", HTTP_GET, handleDdiCss);
  ddiServer.on("/interface_logic.js", HTTP_GET, handleDdiJs);

  ddiServer.on("/api/status", HTTP_GET, handleDdiStatus);
  ddiServer.on("/api/command", HTTP_POST, handleDdiCommand);

  ddiServer.onNotFound([]() {
    ddiServer.send(404, "text/plain", "DDI resource not found.");
  });

  ddiServer.begin();
  Serial.println("DDI web server started.");
}

// -----------------------------------------------------------------------------
// ARDUINO
// -----------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(500);

  connectToHotspot();
  startDdiMdns();
  startDdiWebServer();

  Serial.println();
  Serial.println("Connect the cockpit device to the same phone hotspot.");
  Serial.println("Then open the DDI address shown above.");
}

void loop() {
  updateStateMachine();
  ddiServer.handleClient();

  // Try to recover automatically if the phone hotspot briefly disconnects.
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastReconnectAttempt = 0;

    if (millis() - lastReconnectAttempt >= 5000) {
      lastReconnectAttempt = millis();
      Serial.println("Wi-Fi lost. Reconnecting...");
      WiFi.reconnect();
    }
  }

  delay(2);
}