/*
**************************************************************************************
                       Avrdude for ESP32
                   ================================
   Version: 0.7.0
***************************************************************************************
*/
// Including essential library files
#include <stdint.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ThingSpeak.h>

#include "avrdude.h"
#include "otaserver.h"
#include "HexParser.h"

/*
   ==============================================================
                       USER CHANGEABLE AREA START
                USER SHOULD CHANGE THIS PART OF THE CODE ONLY
*/

/*  Select the WiFi Mode from Station Mode and Access Point(AP) Mode.
 *  In AP Mode, the nod will create its own WiFi hotspot.
 *  In Station mode, node will get connected to an existing WiFi
 *  0 -> AP Mode, 1 -> Station Mode
 *  Default is AP Mode.
*/
#define OTA_WIFI_MODE 1

/*
   If Wifi mode is selected as Station i.e 1, ESP32 will connect to
   one of the exisitng Wifi Network. Hence, provide the existing Wifi
   network credentials.
*/
#if (OTA_WIFI_MODE == 1)
  const char* ssid = "MaataRaani";     //NETWORK_SSID_HERE
  const char* password = "698630LAK";  //NETWORK_PASSWORD_HERE
#endif

/*
 * Microcotroller and its Clock Setting
 * Please select the appropriate clock frequecy code of the microcontroller
 * to which this node mcu will be connected
 * =========================================
 *            Clock            |      Code 
 * =========================================
 *  Greater or equal to 8 MHz         0
 *  Greater or equal to 1 MHz         1
 *  Greater or equal to 128 kHz       2
 */
#define F_CLK 0
/*
                      USER CHANGEABLE AREA ENDED
   ==============================================================
*/
/*
   If Wifi mode is selected as AP i.e 0, ESP32 will create its
   own Wifi Network. The credentials will be-
   SSID = Avrdude
   Password = spider.123
*/
#if (OTA_WIFI_MODE == 0)
  const char* ssid = "Avrdude";
  const char* password = "spider.123";
#endif

const char* host = "avrdude";                                    // Host name for mDNS server

WebServer server(80);                                            // Creating WebServer object on port 80
WiFiClient client;

/* Global declarations for OTA Logic */
String resp = "Some error occured. Please re-try";               // Response from avrdude will be stored here 
uint8_t file[32768] PROGMEM;
uint8_t prog[32768] PROGMEM;
int file_index = 0;
String fileName = "";
long baudRate = 1200;

/* Global declarations for ThingSpeak IoT Cloud Plaotform */
long prevMillisThingSpeak = 0;
int intervalThingSpeak = 20000; 
const long CHANNEL = 1268523;
const char *WRITE_API = "H1VXBE4MWXH7D302";

void flashProgram(){
  Serial.print(F("\nUploading Success: "));Serial.print(file_index);Serial.println(F(" bytes transferred.\n"));
  for(int i=0;i<file_index;i++){
    Serial.write(file[i]);
  }
  // Call the HexParser to parse the file first
  size_t len=0;
  Parser(file, file_index, &len, prog);
  // Call the Avrdude here to upload the file
  Serial.println(F("\nStarting Avrdude......\n\n"));
  for(int i=0;i<200;i++){
    Serial.print(" ");
  }
  Avrdude avrdude;
  resp = avrdude.begin(baudRate, fileName, prog, len);
  file_index = 0;
}

void setup(void) {
  // Check whether clock is defined or not.
  #ifndef F_CLK
    #error "Please select the appropriate clock value"
  #else
    #if (F_CLK==0)
      Serial.begin(115200);
      baudRate = 115200;
    #elif (F_CLK==1)
      Serial.begin(9600);
      baudRate = 9600;
    #elif (F_CLK==2)
      Serial.begin(4800);
      baudRate = 4800;
    #else
      Serial.begin(1200);
    #endif
  #endif 
  Serial.println("");
  #ifdef OTA_WIFI_MODE
    #if (OTA_WIFI_MODE == 0)
      Serial.println("Setting AP (Access Point)..");
      WiFi.softAP(ssid, password);
      IPAddress IP = WiFi.softAPIP();
      Serial.print("Network SSID: ");Serial.println(ssid);
      Serial.print("Network Password: ");Serial.println(password);
      Serial.print("AP IP address: "); Serial.println(IP);
    #elif (OTA_WIFI_MODE == 1)
      Serial.print("Connecting to WiFi");
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        Serial.print(".");
      }
      Serial.println("");
      Serial.print("Connected to "); Serial.println(ssid);
      Serial.print("IP address: "); Serial.println(WiFi.localIP());
    #else
      #error Un-identfied WiFi Mode
    #endif
  #else
    #error WiFi mode not defined
  #endif
  /* Using mdns for host name resolution */
  if (!MDNS.begin(host)) {                                 
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.print("mDNS responder started.\nServing at http://"); Serial.print(host); Serial.println(".local      on Port 80");
  
  // Route for root / web page
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", root());
  });
  // Route to /upload handler for GET Request
  server.on("/upload", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(404);
  });
  // Route to /upload handler for POST Request
  server.on("/upload", HTTP_POST, []() {
    flashProgram();
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", resp);
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("\nIncoming file: %s\n", upload.filename.c_str());
      Serial.println(F("Reading file......"));
    }
    else if (upload.status == UPLOAD_FILE_WRITE) {
      for (int i = 0; i < upload.currentSize; i++) {
        file[file_index++] = upload.buf[i];
      }
    }
    else if (upload.status == UPLOAD_FILE_END) {
      fileName = upload.filename;
    }
  });
  // Route to handle not found
  server.onNotFound([]() {
    server.sendHeader("Connection", "close");
    server.send(404);
  });
  // Start server
  server.begin();
  Serial.println("Webserver Started.");
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  Serial.println("ThingSpeak instance initialized.");
  
}

void loop(void) {
  server.handleClient();
  if(millis() - prevMillisThingSpeak > intervalThingSpeak){
    Serial.println("ThingSpeak: Attempting to log the data and update the channel.");
    ThingSpeak.setField(1, (float)(millis()/1000));           // Weight field
    ThingSpeak.setField(2, 5);                                // IR field
    ThingSpeak.setField(3, "Sat 5th Jun 20202");              // ExpectedFillBy field
    ThingSpeak.setField(4, (float)0.5);                       // FirmwareVersion field
    // Write to the ThingSpeak channel
    int x = ThingSpeak.writeFields(CHANNEL, WRITE_API);
    if(x == 200) {
      Serial.println("ThingSpeak: Channel update successful.");
    }
    else {
      Serial.println("ThingSpeak: Problem updating channel. HTTP error code " + String(x));
    }
    prevMillisThingSpeak = millis();
  }
  delay(500);
}