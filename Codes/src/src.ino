/*
**************************************************************************************
                                SmartBin
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

#define length(var) ((size_t)(sizeof(var)/sizeof(var[0])))    //Macros to get the length of an array

/*
   ==============================================================
                       USER CHANGEABLE AREA START
                USER SHOULD CHANGE THIS PART OF THE CODE ONLY
*/

/*
   Provide the existing Wifi network credentials.
*/
const char* ssid = "MaataRaani";
const char* password = "698630LAK";

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

const char* host = "smartbin";                                      // Host name for mDNS server

WebServer server(3000);                                            // Creating WebServer object on port 80

WiFiClient client;

SBSession session;

#define PSW "NITT1234"

/* Global declarations for OTA Logic */
String resp = "Some error occured. Please re-try";                // Response from avrdude will be stored here 
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

// To validate the cookie in the upcoming HTTP request
bool is_authentified() {
  Serial.print(F("Authenticating request..... "));
  if (server.hasHeader("Cookie")) {
    Serial.print(F("\nFound cookie: "));
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=") != -1 && cookie.indexOf(",") == -1) {
      String ses = cookie.substring(13);
      if(ses == session.getKey(PSW)){
        Serial.println(F("Successful"));
        return true;
      }
    }
    server.sendHeader("Set-Cookie", "ESPSESSIONID=0;Max-Age=-1");
  }
  Serial.println(F("Failed"));
  return false;
}

// Login page, also called for disconnect
void handleLogin() {
  Serial.println("\nRoute: GET /login ");
  String msg;
  if(server.hasArg("LOGOUT")) {
    Serial.println("Loging Out...");
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Set-Cookie", "ESPSESSIONID=0;Max-Age=-1");
    server.send(301);
    return;
  }
  if(is_authentified()) {
      server.sendHeader("Location", "/");
      server.sendHeader("Cache-Control", "no-cache");
      server.send(301);
      return;
  }
  if (server.hasArg("PASSWORD")) {
    if (server.arg("PASSWORD") == PSW) {
      server.sendHeader("Location", "/");
      server.sendHeader("Cache-Control", "no-cache");
      session.genKey();
      String hash = session.getKey(PSW);
      server.sendHeader("Set-Cookie", "ESPSESSIONID="+hash+";HttpOnly");
      server.send(301);
      Serial.println("Log in Successful");
      return;
    }
    msg = "Wrong username/password! try again.";
    Serial.println(F("Log in Failed"));
  }
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/html", getLoginPage(msg));
}

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
  Serial.print(F("Connecting to WiFi"));
  WiFi.begin(ssid, password);
  pinMode(2,OUTPUT);
  digitalWrite(2,LOW);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(2,LOW);
    delay(125);
    digitalWrite(2,HIGH);
    delay(125);
    Serial.print(".");
  }
  digitalWrite(2,HIGH);
  Serial.println("");
  Serial.print(F("Connected to ")); Serial.println(ssid);
  Serial.print(F("IP address: ")); Serial.println(WiFi.localIP());
  /* Using mdns for host name resolution */
  if (!MDNS.begin(host)) {                                 
    Serial.println(F("Error setting up MDNS responder!"));
    while (1) {
      delay(1000);
    }
  }
  Serial.print(F("mDNS responder started.\nServing at http://")); Serial.print(host); Serial.println(".local      on Port 3000");

  // Route for root / web page
  server.on("/", HTTP_GET, []() {
    Serial.println(F("\nRoute: GET / "));
    if (!is_authentified()) {
      server.sendHeader("Location", "/login");
      server.sendHeader("Cache-Control", "no-cache");
      server.send(301);
      return;
    }
    if (server.hasHeader("User-Agent")) {
      Serial.println("The user agent used is : " + server.header("User-Agent"));
    }
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", root());
  });
  // Route to /login handler
  server.on("/login", handleLogin);
  // Route to /upload handler for GET Request
  server.on("/upload", HTTP_GET, []() {
    Serial.println(F("\nRoute: GET /update "));
    server.sendHeader("Connection", "close");
    server.send(404);
  });
  // Route to /upload handler for POST Request
  server.on("/upload", HTTP_POST, []() {
    Serial.println(F("\nRoute: POST /update "));
    if (!is_authentified()) {
      server.sendHeader("Connection", "close");
      server.send(401);
      return;
    }
    if (server.hasHeader("User-Agent")) {
      Serial.println("The user agent used is : " + server.header("User-Agent"));
    }
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
  //List of headers to be recorded
  const char * headerkeys[] = {"User-Agent", "Cookie"} ;
  //Ask server to track these headers
  server.collectHeaders(headerkeys, length(headerkeys));
  // Start server
  server.begin();
  Serial.println(F("\nWebserver Started."));
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  Serial.println(F("ThingSpeak instance initialized."));
  
}

void loop(void) {
  server.handleClient();
  /*if(millis() - prevMillisThingSpeak > intervalThingSpeak){
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
  }*/
  delay(500);
}
