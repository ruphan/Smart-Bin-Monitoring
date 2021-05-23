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

#define length(var) ((size_t)(sizeof(var)/sizeof(var[0])))    //Macro to get the length of an array

/*
   ==============================================================
                       USER CHANGEABLE AREA START
                USER SHOULD CHANGE THIS PART OF THE CODE ONLY
*/

/*
   Provide the existing Wifi network credentials.
*/
const char* ssid = "NetworkSSID";
const char* password = "NetworkPassword";

/*
   Microcontroller and its Clock Setting
   Please select the appropriate clock frequecy code of the microcontroller
   to which this node mcu will be connected
   =========================================
              Clock            |      Code
   =========================================
    Greater or equal to 8 MHz         0
    Greater or equal to 1 MHz         1
    Greater or equal to 128 kHz       2
*/
#define F_CLK 0

#define PSW "NITT1234"                                              // Authentication password

const char* host = "smartbin";                                      // Host name for mDNS server

const long CHANNEL = <CHANNEL_ID>;
const char *WRITE_API = <API_KEY>;

/*
                      USER CHANGEABLE AREA ENDED
   ==============================================================
*/

WebServer server(80);                                               // Creating WebServer object on port 80

WiFiClient client;

SBSession session;

/* Global declarations for OTA Programming Logic */
String resp = "Some error occured. Please re-try";                // Response from avrdude will be stored
uint8_t file[32768] PROGMEM;                                      // HEX file will be stored
uint8_t prog[32768] PROGMEM;                                      // Data after parsing HEX file will be stored
int file_index = 0;
String fileName = "";

/* Global declarations for ThingSpeak IoT Cloud Platform */
long long prevMillisThingSpeak = 0;
int intervalThingSpeak = 20000;                                   // Milli-seconds after which data should be sent to ThingSpeak server

/* Global declaration for data expected from Microcontroller */
uint16_t currentWeight;
uint16_t currentIR;
uint16_t thresholdWeight;
uint16_t thresholdIR;
int _VERSION_ = -1;                                               // Software version
String DB_ID = "";                                                // Dustbin ID

// This function accepts a byte and returns its corresponding hexadecimal value in String
// format. The hexadecimal value will always be 8 bits and the MSBs leading 0's arent ignored. 
String hexString(byte a){
  int b = a;
  char hex[]="ABCDEF";
  char num[]="0123456789";
  char ch[2];
  int i=1;
  while(i>=0)
  {
     if((b%16)>9){
        ch[i]=hex[(b%16)%10];
     }
     else{
        ch[i]=num[b%16];
     }
     b/=16;
     i--;
  }
  String ans = "";
  ans += ch[0];
  ans += ch[1];
  return ans;
}

// Function to validate the cookie in the incoming HTTP request.
bool is_authentified() {
  Serial.print(F("Authenticating request..... "));
  if (server.hasHeader("Cookie")) {                                               // If request has Cookie header
    Serial.print(F("\nFound cookie: "));
    String cookie = server.header("Cookie");                                      // Get the full cookie value 
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=") != -1 && cookie.indexOf(",") == -1) {     // Validate the cookie value format
      String ses = cookie.substring(13);                                          // Get the actual session key value
      if (ses == session.getKey(PSW)) {                                           // Verify with the session key on server
        Serial.println(F("Successful"));
        return true;
      }
    }
    server.sendHeader("Set-Cookie", "ESPSESSIONID=0;Max-Age=-1");                 // Invalid cookie was found, destroy the cookie
  }
  Serial.println(F("Failed"));
  return false;
}

// Function to handle /login route and serve the login page. Also called during log out.
void handleLogin() {
  Serial.println("\nRoute: GET /login ");
  String msg;
  if (server.hasArg("LOGOUT")) {                                                 // Check if request has LOGOUT header
    Serial.println("Loging Out...");
    server.sendHeader("Location", "/login");                                     // Redirect to /login
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Set-Cookie", "ESPSESSIONID=0;Max-Age=-1");                // Destroy the session cookie
    server.send(301);
    return;
  }
  if (is_authentified()) {                                                       // Check if session cookie exists already 
    server.sendHeader("Location", "/");                                          // Redirect to / (root)
    server.sendHeader("Cache-Control", "no-cache");
    server.send(301);
    return;
  }
  if (server.hasArg("PASSWORD")) {                                               
    if (server.arg("PASSWORD") == PSW) {                                         // Check if user entered the correct password
      server.sendHeader("Location", "/");                                        // Redirect to / (root)
      server.sendHeader("Cache-Control", "no-cache");
      session.genKey();                                                          
      String hash = session.getKey(PSW);                                         // Get the new session key
      server.sendHeader("Set-Cookie", "ESPSESSIONID=" + hash + ";HttpOnly");     // set the cookie header 
      server.send(301);
      Serial.println("Log in Successful");
      return;
    }
    msg = "Wrong username/password! try again.";
    Serial.println(F("Log in Failed"));
  }
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/html", getLoginPage(msg));                              // serve the login page
}

// Function to intiate parsing of HEX file and then initiate boot loader. 
void flashProgram() {
  Serial.print(F("\nUploading Success: ")); Serial.print(file_index); Serial.println(F(" bytes transferred.\n"));
  for (int i = 0; i < file_index; i++) {
    Serial.write(file[i]);
  }
  // Call the HexParser to parse the file first
  size_t len = 0;
  Parser(file, file_index, &len, prog);
  // Call the Avrdude here to upload the file
  Serial.println(F("\nStarting Avrdude......\n\n"));
  for (int i = 0; i < 200; i++) {
    Serial.print(" ");
  }
  Avrdude avrdude;
  resp = avrdude.begin(fileName, prog, len);                     // Get the response from the avrdude
  file_index = 0;                                                          // Reset the file index for next file transfer. 
}

// main() function of Arduino IDE
void setup(void) {
  // Check whether the clock is defined or not.
  #ifndef F_CLK
    #error "Please select the appropriate clock value"
  #else
    #if (F_CLK==0)
      Serial.begin(115200);
      Serial2.begin(115200);
    #elif (F_CLK==1)
      Serial.begin(9600);
      Serial2.begin(9600);
    #elif (F_CLK==2)
      Serial.begin(4800);
      Serial2.begin(4800);
    #else
      Serial.begin(1200);
      Serial2.begin(1200);
    #endif
  #endif
  Serial.println("");
  Serial.print(F("Connecting to WiFi"));
  WiFi.begin(ssid, password);                                       // Connecting to WiFi
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  while (WiFi.status() != WL_CONNECTED) {                           // while Wi-Fi is connecting
    // Blink the on-board LED
    digitalWrite(2, LOW);
    delay(125);
    digitalWrite(2, HIGH);
    delay(125);
    Serial.print(".");
  }
  digitalWrite(2, HIGH);
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
  Serial.print(F("mDNS responder started.\nServing at http://")); Serial.print(host); Serial.println(".local");

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
    flashProgram();                                                                       // After transferring of file is done, start flashing process
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

  // Initialize ThingSpeak instance
  ThingSpeak.begin(client);
  Serial.println(F("ThingSpeak instance initialized."));
}

// Function to read data (which will be logged to IoT server) from the microcontroller. 
void readDataFromMicroController() {
  int i = 0;
  while (i < 4) {
    byte command = Serial2.read();
    uint16_t incomingData = (Serial2.read() << 8) | Serial2.read();
    if ( command == 0x10 ) {              // Start of the Current Weight block
      Serial.println("Current Weight Block... DONE");
      currentWeight = incomingData;
      Serial.print("Current Weight: 0x");Serial.print(hexString((currentWeight>>8) & 0xFF));Serial.println(hexString(currentWeight & 0xFF));
    }
    else if ( command == 0x20 ) {         // Start of the Current IR LED block
      Serial.println("Current IR LED Block... DONE");
      currentIR = incomingData;
      Serial.print("Current IR: 0x");Serial.print(hexString((currentIR>>8) & 0xFF));Serial.println(hexString(currentIR & 0xFF));
    }
    else if ( command == 0x11 ) {         // Start of the Threshold Weight block
      Serial.println("Threshold Weight Block... DONE");
      thresholdWeight = incomingData;
      Serial.print("Threshold Weight: 0x");Serial.print(hexString((thresholdWeight>>8) & 0xFF));Serial.println(hexString(thresholdWeight & 0xFF));
    }
    else if ( command == 0x21 ) {         // Start of the Threshold IR LED block
      Serial.println("Threshold IR LED Block... DONE");
      thresholdIR = incomingData;
      Serial.print("Threshold IR: 0x");Serial.print(hexString((thresholdIR>>8) & 0xFF));Serial.println(hexString(thresholdIR & 0xFF));
    }
    else {
      Serial.println("Invalid packet was received.");
      return;                            // Invalid Packet(s) recieved
    }
    i++;
  }
  
  // Get the CODE Version
  byte data = Serial2.read();
  if(data == 0x25){
    _VERSION_ = Serial2.read();
    Serial.println("Version Block... DONE");
    Serial.print("Version: ");Serial.println(_VERSION_);
  }
  else{
    Serial.println(hexString(data));
    Serial.println("Invalid Version packet was received.");
    return;
  }
  
  // Get the Dustbin ID
  data = Serial2.read();
  if(data == 0x30){
    byte temp = Serial2.read();
    DB_ID = (char)temp;
    int count = 1;
    while(true){
      count ++;
      temp = Serial2.read();
      if(temp == 0x0A){
        break;
      }
      DB_ID += (char)temp;
      if(count >= 100){
        break;
      }
    }
    Serial.println("ID Block... DONE");
    Serial.print("DB ID: ");Serial.println(DB_ID);
  }
  else{
    Serial.println(hexString(data));
    Serial.println("Invalid ID packet was received.");
    return;
  }
  
  // Check the termination
  byte terminate = Serial2.read();
  if ( terminate == 0x04 ) {            // Termination command received successfully
    Serial.print("Successfully recieved END OF TRANSMISSION. Transfer complete.\n");
    Serial.print("Sending Acknowledgement (ACK)..... ");
    Serial2.flush();                   // Flush any residual data on the serial line
    
    // Send acknowledgement packets back to microcontroller
    Serial2.write((byte)(0x06));       // ACK
    Serial2.write((byte)(0x04));       // End Transmission
    Serial.println(" DONE");
    
    // Converting Current IR to byte[i] to String[i]
    String curr_ir = "";
    for(int i=16;i>=0;i--){
      curr_ir += (currentIR >> i ) & 0x01;
    }

    if(millis() - prevMillisThingSpeak >= intervalThingSpeak) {
      // Log the data to ThingsSpeak Server
      Serial.println("\nThingSpeak: Attempting to log the data...");
      ThingSpeak.setField(1, DB_ID);                             // DB_ID Field
      ThingSpeak.setField(2, String(_VERSION_));                 // Version field
      ThingSpeak.setField(3, String((int)currentWeight/1000));             // Current Weight field
      ThingSpeak.setField(4, curr_ir);                           // Current IR field
      ThingSpeak.setField(5, String((int)thresholdWeight/1000));           // Threshold Weight field
      ThingSpeak.setField(6, String(thresholdIR));               // Threshold IR field
      
      // Write to the ThingSpeak channel
      int x = ThingSpeak.writeFields(CHANNEL, WRITE_API);
      if (x == 200) {
        Serial.println("ThingSpeak: Channel update successful.");
      }
      else {
        Serial.println("ThingSpeak: Problem updating channel. HTTP error code " + String(x));
      }
      prevMillisThingSpeak = millis();
    }
  }
  else{
    Serial.print("Failed. END OF TRANSMISSION. Transfer incomplete.\n");
    return;
  }
}

void loop(void) {
  server.handleClient();
  if(Serial2.available()){
    byte res = Serial2.read();
    if (res == 0x01) {      // Start of the Transmission
        Serial.println("\nIncoming data from Microcontroller...");
        readDataFromMicroController();
    }
  }
  delay(250);
}
