#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h> // for server functionality
#include <FS.h> // database (file system) functionality

/*
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
*/

/* function prototypes */
// connections
void connectWiFi(const char *ssid); // connects to @ssid network with @password
// HTTP server functions
void handleLogin(); // on "PolyESP/login"
void handleLED(); // on "PolyESP/LED"
// File System functions
String getContentType(String filename); // on any undefined URI, convert extension to MIME
bool handleFileRead(String path); // on any undefined URI, look for file and send it
void handleFileUpload(); // on /upload
void fileUpload(); // clean up upload handling code

/* objects and variables */
ESP8266WebServer server(8083); // webserver object that listens to port 8083 (given permission from school)
// house access
const char *ssid = "robots";
// ESP server
int led = 16; // for LED server thing
const char *success = "<h1>File upload succeeded!</h1>";
File fsUploadFile; String uploadFile; // for uploading to resolve scope issues
  
// user data (/login)
const char *userName = "username";
const char *userPass = "password";
char loggedin = 0;

/* actual program */
void setup() {
  /* misc setup */
  Serial.begin(115200);
  pinMode(led, OUTPUT); // for LED server thing

  /* wifi setup */
  // connect to house WiFi
  connectWiFi(ssid);

  /* server setup */
  SPIFFS.begin(); // start database (file system)
  
  server.on("/login",HTTP_POST,handleLogin); // POST request to /login
  server.on("/",HTTP_POST,handleLogout); // POST request to /
  server.on("/",HTTP_GET,
    [](){
      handleFileRead("/");
    });
  server.on("/LED",HTTP_POST,handleLED); // POST request to /LED
  fileUpload(); // allows uploading on /upload. code below
  // instead of a handler function place the code directly. careful with parentheses
  server.onNotFound(
    [](){
      if (!handleFileRead(server.uri())) // for any undefined case, call this function
        server.send(404,"text/plain","404: Not found"); // send 404 (not found) and text
    });
  server.begin(); // start the server
  Serial.println("HTTP server started!");
}

void loop() {
  // listen for server requests
  server.handleClient();
}

/* function definitions */
void connectWiFi(const char *ssid){
  WiFi.begin(ssid);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Serial.print("...");

  while(WiFi.status() != WL_CONNECTED){
    Serial.print('.');
    delay(1000);
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}

void handleLogin(){
  if(!server.hasArg("username") || !server.hasArg("password") || server.arg("username") == NULL || server.arg("password") == NULL){
    server.send(400,"text/plain","400: Invalid Request");
    return;
  }
  if(server.arg("username") == userName && server.arg("password") == userPass){
    loggedin = 1;
    server.sendHeader("Location","/upload.html"); // Send the client to the upload page
    server.send(303);
  }
  else server.send(401,"text/plain","401: Unauthorized");
}
void handleLogout(){
  loggedin = 0;
  server.sendHeader("Location","/"); // tell browser to go to / location thru header
  server.send(303); // send redirect request
}
void handleLED(){
  digitalWrite(led,!digitalRead(led)); // flip LED state
  server.sendHeader("Location","/"); // tell browser to go to / location thru header
  server.send(303); // send redirect request
}

String getContentType(String filename){
  if (filename.endsWith(".html") || filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".js")) return "application/javascript";
  return "text/plain";
}
bool handleFileRead(String path){
  if (path.endsWith("/")) path += "index.htm";
  // requires login for file uploads
  if (path == "/upload.html") {if (loggedin==0) path = "/login.html";}
  Serial.print("Looking for " + path);
  String contentType = getContentType(path);
  Serial.println(" of MIME type " + contentType);
  if (SPIFFS.exists(path)){
    Serial.println("\tFile found!");
    File file = SPIFFS.open(path,"r");
    size_t sent = server.streamFile(file,contentType);
    file.close();
    return true;
  }
  Serial.println("\tFile not found :(");
  return false;
}
void handleFileUpload(){
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    uploadFile = upload.filename;
    if(!uploadFile.startsWith("/")) uploadFile = "/"+uploadFile;
    Serial.print("handleFileUpload Name: "); Serial.println(uploadFile);
    fsUploadFile = SPIFFS.open(uploadFile,"w"); // Open the file for writing in SPIFFS (create if it doesn't exist)
  }
  else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  }
  else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) { // If the file was successfully created, close it
      fsUploadFile.close();
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize); Serial.println(uploadFile);
      if (getContentType(uploadFile) == "text/html") server.sendHeader("Location",uploadFile); // redirect to uploaded file
      else server.sendHeader("Location","/success"); // Redirect the client to the success page
      server.send(303);
    }
    else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}
void fileUpload(){
  server.on("/upload.html",HTTP_GET,
    [](){
      String path = server.uri();
      handleFileRead(path);
      //if(!handleFileRead(path)) server.send(404,"text/plain","404: Not Found"); // relic
    }
  ); // GET request to /upload
  server.on("/upload.html",HTTP_POST,
    [](){server.send(200);},
    handleFileUpload
  ); // POST request to /upload
  server.on("/success",
    [](){server.send(200,"text/html",success);} // send status 200 (ok) and html}
  ); // when file upload succceeds
}

