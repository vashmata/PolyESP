#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h> // for a cleaner access name
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
void createAP(const char *ssid, const char *password); // creates @ssid access point with @password
// HTTP server functions
void handleLogin(); // on "PolyESP/login"
void handleLED(); // on "PolyESP/LED"
// File System functions
String getContentType(String filename); // on any undefined URI, convert extension to MIME
bool handleFileRead(String path); // on any undefined URI, look for file and send it
void handleFileUpload(); // on /upload
void handleFileDelete();
void fileUpload(); // clean up upload handling code
String formatBytes(size_t bytes); // for directory printDir
String genDir(); // generates current SPIFFS dir

/* objects and variables */
ESP8266WebServer server(80); // webserver object that listens to port 80 (requests)
// house access
const char *ssid = "robots";
// ESP access (access point)
const char *ssid1 = "PolyESP";
const char *password1 = "password";
// ESP server
int led = 16; // for LED server thing
const char *success = "<h1>File upload succeeded!</h1>";
//const char *rootHTML = "<form action=\"/login\" method=\"POST\"><input type=\"text\" name=\"username\" placeholder=\"Username\"></br><input type=\"password\" name=\"password\" placeholder=\"Password\"></br><input type=\"submit\" value=\"Login\"></form><p>Enter the correct username and password ...</p>";
File fsUploadFile; // for uploading to resolve scope issues
  
// user data (/login)
const char *userName = "PolyESP";
const char *userPass = "password";

/* actual program */
void setup() {
  /* misc setup */
  Serial.begin(115200);
  delay(50);
  Serial.println();
  pinMode(led, OUTPUT); // for LED server thing

  /* wifi setup */
  // connect to house WiFi
  connectWiFi(ssid);
  MDNS.begin("PolyESP"); // set up cleaner access name, used when connecting to network
  Serial.println("mDNS responder started!");

  // create access point
  //createAP(ssid1,password1);
  
  /* server setup */
  SPIFFS.begin(); // start database (file system)
  Serial.println("File system started!\n");
  String ESPdir = genDir(); // print files in system to serial
  Serial.println(ESPdir);
  
  //server.on("/login",HTTP_POST,handleLogin); // POST request to /login
  server.on("/LED",HTTP_POST,handleLED); // POST request to /LED
  fileUpload(); // allows uploading/deleting on /upload. code below
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
  Serial.printf("Connecting to %s\n...",ssid);
  while(WiFi.status() != WL_CONNECTED) {Serial.print('.'); delay(1000);}
  Serial.print("\nConnection established!\nIP address: ");
  Serial.println(WiFi.localIP()+"\n");
}

void createAP(const char *ssid, const char *password){
  WiFi.softAP(ssid,password);
  Serial.printf("Access Point \"%s\" started\nIP address: ",ssid);
  Serial.println(WiFi.softAPIP());
}

void handleLogin(){
  if(!server.hasArg("username") || !server.hasArg("password") || server.arg("username") == NULL || server.arg("password") == NULL){
    server.send(400,"text/plain","400: Invalid Request");
    return;
  }
  if(server.arg("username") == userName && server.arg("password") == userPass){
    server.send(200,"text/html","<h1>Welcome, " + server.arg("username") + "!</h1><p>Login successful</p>");
  }
  else server.send(401,"text/plain","401: Unauthorized");
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
  if (SPIFFS.exists(path)){
    File file = SPIFFS.open(path,"r");
    String contentType = getContentType(path);
    size_t sent = server.streamFile(file,contentType);
    file.close(); return true;
  }
  Serial.println("\tFile not found :("); return false;
}
void handleFileUpload(){
  static String uploadFile;
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
      if (uploadFile.endsWith(".html") || uploadFile.endsWith(".htm")) server.sendHeader("Location",uploadFile); // redirect to uploaded file
      else server.sendHeader("Location","/success"); server.send(303); // Redirect the client to the success page
    }
    else server.send(500, "text/plain", "500: couldn't create file");
  }
}
void handleFileDelete(){
  String path;
  Serial.println("ok!");
  if(!server.hasArg("Delete") || server.arg("Delete") == NULL){
    server.send(400,"text/plain","400: Invalid Request");
    return;
  }
  else path = server.arg("Delete");
  if (!path.startsWith("/")) path = "/" + path;
  Serial.println("handleFileDelete: " + path);
  if(path == "/") return server.send(500,"text/plain","500: BAD PATH");
  if(path == "/index.htm" || path == "/Users.xml" || path == "/upload.html")
    return server.send(403,"text/plain","403: Forbidden! This file is critical!");
  if(!SPIFFS.exists(path)) return server.send(404,"text/plain","404: File Not Found");
  SPIFFS.remove(path); server.send(200, "text/plain", "");
  path = String(); return;
}
String formatBytes(size_t bytes){
  if (bytes < 1024) return String(bytes)+"B";
  else if (bytes < (1024 * 1024)) return (String(bytes/1024.0)+"KB");
  else return (String(bytes/1024.0/1024.0)+"MB");
}
String genDir(){
  String files = "<h1>PolyESP Files:</h1><p>";
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    files += String(dir.fileName().c_str()+1);
    files += String("\tsize: ") + formatBytes(dir.fileSize()) + String("<br>");
  }
  files += "</p>";
  return files;
}
void fileUpload(){
  server.on("/list",HTTP_GET,
    [](){
      String list = genDir();
      list += "<h3>Delete a file:</h3><form method=\"POST\"><input type=\"text\" name=\"name\"><input class=\"button\" type=\"submit\" value=\"Delete\"></form>";
      server.send(200,"text/html",list);
    }); // generate file list page
  server.on("/list",HTTP_POST,
    [](){server.send(200);},
    handleFileDelete); // DELETE file
  server.on("/upload",HTTP_GET,
    [](){
      if(!handleFileRead("/upload.html")) server.send(404,"text/plain","404: Not Found");
    }); // GET request to /upload
  server.on("/upload",HTTP_POST,
    [](){server.send(200);},
    handleFileUpload); // POST request to /upload
  server.on("/success",
    [](){server.send(200,"text/html",success);} // send status 200 (ok) and html}
  ); // when file upload succceeds
}
