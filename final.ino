#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#ifndef STASSID
#define STASSID "TPlink630-30"//"BYOD"
#define STAPSK "kasraisdick"//"zFqzkADxyjNc6EJ4"

#define digitalInput 5 // Connect to input A
#define motorOutput 4 // Connect to output A
#define digitalInput2 16 // Connect to input B
#define motorOutput2 14 // Connect to output B
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
bool manualMode = true;

void stoped() {
  digitalWrite(motorOutput2, HIGH);
  digitalWrite(motorOutput, HIGH);
}
void goFoward() {
  digitalWrite(motorOutput2, LOW);
  digitalWrite(motorOutput, LOW);
}
void turnLeft() {
  digitalWrite(motorOutput, LOW);
  digitalWrite(motorOutput2, HIGH);
}

void turnRight() {
  digitalWrite(motorOutput2, LOW);
  digitalWrite(motorOutput, HIGH);
}
ESP8266WebServer server(80);

const int led = 13;

String fileContent;
const char* filePath = "/index.html";

void handleRoot() {
  readFile(filePath, fileContent);
  loadFromSPIFFS(filePath);
}
bool readFile(const char* path, String& outStr) {
  Serial.printf("Reading file: %s\r\n", path);

  if (!SPIFFS.begin()) {
    Serial.println("Error mounting the file system");
    return false;
  }
  Serial.println("FS mounted ok");

  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file");
    return false;
  }
  Serial.println("File opened ok");

  Serial.print("The file size is ");
  Serial.println(file.size());

  // Read file into the provided String object
  while (file.available()) {
    outStr += (char)file.read();
  }

  Serial.println("Closing file and FS");
  file.close();
  return true;
}

bool loadFromSPIFFS(String path) {
  String dataType = "text/html";

  Serial.print("Requested page -> ");
  Serial.println(path);
  if (SPIFFS.exists(path)) {

    Serial.println("if spff exists path hit");
    File dataFile = SPIFFS.open(path, "r");
    if (!dataFile) {
      Serial.print("File not found");
      handleNotFound();
      return false;
    }

    if (server.streamFile(dataFile, dataType) != dataFile.size()) {
      Serial.println("Sent less data than expected!");
    } else {
      Serial.println("Page served!");
    }

    dataFile.close();

    Serial.println("the file closed");
  } else {
    handleNotFound();

    Serial.println("file doesnt exist");
    return false;
  }
  return true;
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


void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  pinMode(motorOutput, OUTPUT);
  pinMode(motorOutput2, OUTPUT);
  pinMode(digitalInput, INPUT);
  pinMode(digitalInput2, INPUT);
  stoped();
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
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

  Serial.print(F("Inizializing FS..."));
  if (SPIFFS.begin()) {
    Serial.println(F("done."));
  } else {
    Serial.println(F("fail."));
  }
  server.on("/", handleRoot);
  server.addHook([](const String&, const String & url, WiFiClient*, ESP8266WebServer::ContentTypeFunction) {
    if (url.startsWith("/forward")) {
      if (manualMode) {
        goFoward();
      }
      return ESP8266WebServer::CLIENT_MUST_STOP;
    }
    return ESP8266WebServer::CLIENT_REQUEST_CAN_CONTINUE;
  });
  server.addHook([](const String&, const String & url, WiFiClient*, ESP8266WebServer::ContentTypeFunction) {
    if (url.startsWith("/left")) {
      if (manualMode) {
        turnLeft();
      }
      return ESP8266WebServer::CLIENT_MUST_STOP;
    }
    return ESP8266WebServer::CLIENT_REQUEST_CAN_CONTINUE;
  });

  server.addHook([](const String&, const String & url, WiFiClient*, ESP8266WebServer::ContentTypeFunction) {
    if (url.startsWith("/right")) {
      if (manualMode) {
        turnRight();
      }
      return ESP8266WebServer::CLIENT_MUST_STOP;
    }
    return ESP8266WebServer::CLIENT_REQUEST_CAN_CONTINUE;
  });
  server.addHook([](const String&, const String & url, WiFiClient*, ESP8266WebServer::ContentTypeFunction) {
    if (url.startsWith("/stop")) {
      stoped();
      return ESP8266WebServer::CLIENT_MUST_STOP;
    }
    return ESP8266WebServer::CLIENT_REQUEST_CAN_CONTINUE;
  });
  server.addHook([](const String&, const String & url, WiFiClient*, ESP8266WebServer::ContentTypeFunction) {
    if (url.startsWith("/Mode")) {
      // Toggle between auto and manual mode
      manualMode = !manualMode;
      return ESP8266WebServer::CLIENT_MUST_STOP;
    }
    return ESP8266WebServer::CLIENT_REQUEST_CAN_CONTINUE;
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  int digitalValue = digitalRead(digitalInput);
  int digitalValue2 = digitalRead(digitalInput2);
  if (!manualMode) {
    Serial.println("go");
   // if (digitalValue == 0 && digitalValue2 == 1) {
     // digitalWrite(motorOutput2, LOW);
     // digitalWrite(motorOutput, HIGH);
    //}
   // else if (digitalValue2 == 0 && digitalValue == 1) {
     // digitalWrite(motorOutput, LOW);
     // digitalWrite(motorOutput2, HIGH);
   // }
    if (digitalValue > digitalValue2) {
      digitalWrite(motorOutput2, LOW);
      digitalWrite(motorOutput, HIGH);
    }
    else if (digitalValue2 > digitalValue) {
      digitalWrite(motorOutput, LOW);
      digitalWrite(motorOutput2, HIGH);
    }
    else{
      Serial.println("nothing");
      }
  }
  server.handleClient();
  MDNS.update();
}
