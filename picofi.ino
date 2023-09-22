#include <WiFi.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <LEAmDNS.h>
#include <DNSServer.h>

#define SETUP_SSID "picoFi SetupPoint"
#define SETUP_PASS "configuration"
#define NAME "picofi"
#define ALLOW_PICOFI_SERIAL true

IPAddress apIP(192, 168, 180, 1);
DNSServer dnsServer;
WebServer connectServer(80);
WebServer server(80);
bool runServer = false;
String ssid, pass;
String html_headers = "<title>picoFi</title><meta name=viewport content=\"width=device-width height=device-height\">";

void picofi_println(String s) {
  if(ALLOW_PICOFI_SERIAL) {
    Serial.println(s);
  }
}
void picofi_println() {
  if(ALLOW_PICOFI_SERIAL) {
    Serial.println();
  }
}
void picofi_print(String s) {
  if(ALLOW_PICOFI_SERIAL) {
    Serial.print(s);
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  delay(3000);
  LittleFS.begin();
  if(ALLOW_PICOFI_SERIAL) {
    if(Serial.readString() == "WIPE\n") {
      LittleFS.format();
      picofi_println("Wipe successful.");
    }
  }
  File wifi = LittleFS.open("/wifi.txt", "r");
  if (MDNS.begin(NAME)) {
    picofi_println("MDNS responder started");
  }
  if(!wifi) {
    blink();
    while(WiFi.status() != WL_CONNECTED) {
      picofi_println("Connection failed!");
      error();
      askForWiFi();
    }
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    ssid = wifi.readStringUntil('\n');
    pass = wifi.readStringUntil('\n');
    if(!wconnect(ssid, pass)) {
      error();
      delay(1000);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(2000);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
      if(!wconnect(ssid, pass)) {
        picofi_println("Connection failed!");
        while(WiFi.status() != WL_CONNECTED) {
          error();
          askForWiFi();
        }
      }
    }
  }
  
  picofi_println("------------");
  picofi_println("Connected!");
  runServer = true;
  digitalWrite(LED_BUILTIN, HIGH);

  server.on("/picofi", []() {
    server.send(200, "text/html", html_headers + "picoFi is set up! You can go home <a href=/><button type=button>here</button></a>, or you can change the network here: <form action=\"/picofi/connect\"><input name=ssid placeholder=SSID><br><input name=pass placeholder=PASSWORD><br><br><button type=submit>Connect</button></form>");
  });
  server.on("/picofi/connect", []() {
    if(server.hasArg("ssid") && server.hasArg("pass")) {
      server.send(200, "text/html", html_headers + "OK! Reloading soon... <script>setTimeout(()=>location.href='/',23000)</script>");
      ssid = server.arg("ssid");
      pass = server.arg("pass");
      runServer = false;
    }
    else {
      server.sendHeader("Location", "/");
      server.send(302);
    }
  });
  server.onNotFound([]() { 
    server.send(404, "text/html", html_headers + "Not found. <a href=/>Go back</a>");
  });
  picofi_setup();
  server.begin();
}

void askForWiFi() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255,255,255,0));
  WiFi.begin(SETUP_SSID, SETUP_PASS);
  picofi_println("Creating WiFi AP...");
  picofi_print("SSID: ");
  picofi_println(SETUP_SSID);
  picofi_print("Password: ");
  picofi_println(SETUP_PASS);
  picofi_print("IP address: ");
  picofi_println(WiFi.localIP().toString());

  delay(500);
  dnsServer.start(53, "*", apIP);
  runServer = true;
  connectServer.on("/", []() {
    connectServer.send(200, "text/html", html_headers + "<form action=\"/connect\"><input name=ssid placeholder=SSID><br><input name=pass placeholder=PASSWORD><br><br><button type=submit>Connect</button></form>");
  });
  connectServer.on("/picofi", []() {
    connectServer.send(200, "text/html", html_headers + "<form action=\"/connect\"><input name=ssid placeholder=SSID><br><input name=pass placeholder=PASSWORD><br><br><button type=submit>Connect</button></form>");
  });
  connectServer.on("/connect", []() {
    if(connectServer.hasArg("ssid") && connectServer.hasArg("pass")) {
      connectServer.send(200, "text/html", html_headers + "OK! Reloading soon... <script>setTimeout(()=>location.href='http://picofi/picofi',23000)</script>");
      ssid = connectServer.arg("ssid");
      pass = connectServer.arg("pass");
      runServer = false;
    }
    else {
      connectServer.sendHeader("Location", "/");
      connectServer.send(301);
    }
  });
  connectServer.begin();
  picofi_println("Server running.");
  while(runServer) {
    dnsServer.processNextRequest();
    connectServer.handleClient();
    MDNS.update();
  }
  long sa = millis();
  picofi_println("Server stopping...");
  while(millis() - sa < 100) {
    dnsServer.processNextRequest();
    connectServer.handleClient();
    MDNS.update();
  }
  connectServer.stop();
  dnsServer.stop();
  picofi_println("Server stopped.");
  blink();
  select(ssid, pass);
}

void select(String ssid, String pass) {
  ssid.trim();
  pass.trim();
  WiFi.end();
  if(ssid == "#WIPE") {
      error();
      error();
      error();
      error();
      error();
      LittleFS.format();
      LittleFS.end();
      rp2040.reboot();
  }
  if(wconnect(ssid, pass)) {
    File wifi = LittleFS.open("/wifi.txt", "w+");
    wifi.println(ssid);
    wifi.println(pass);
  }
}

bool wconnect(String ssid, String pass) {
  ssid.trim();
  pass.trim();
  picofi_print("Connecting to >" + ssid + "< with password >" + pass + "<...");
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(NAME);
  WiFi.begin(ssid.c_str(), pass.c_str());
  long sa = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    blink();
    picofi_print(".");
    if(WiFi.status() == WL_CONNECT_FAILED || millis() - sa > 20000) {
      picofi_println();
      WiFi.end();
      return false;
    }
  }
  picofi_println();
  picofi_print("IP address: ");
  picofi_println(WiFi.localIP().toString());
  return true;
}


void loop() {
  picofi_loop();
  server.handleClient();
  MDNS.update();
  if(WiFi.status() != WL_CONNECTED) {
    runServer = false;
  }
  long sa = millis();
  if(!runServer) {
    digitalWrite(LED_BUILTIN, LOW);
    picofi_println("Server stopping...");
    while(millis() - sa < 100) {
      server.handleClient();
      MDNS.update();
    }
    blink();
    server.stop();
    picofi_println("Server stopped.");
    select(ssid, pass);
    while(WiFi.status() != WL_CONNECTED) {
      picofi_println("Connection failed!");
      error();
      askForWiFi();
    }
  
    picofi_println("------------");
    picofi_println("Connected!");
    digitalWrite(LED_BUILTIN, HIGH);
    runServer = true;
    server.begin();
  }
}

void error() {
  blink();
  delay(50);
  blink();
  delay(50);
  blink();
  delay(50);
  blink();
  delay(50);
}

void blink() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
}





////////
// -------------- Your code:
////////

void picofi_setup() {
  server.on("/", []() {
    server.send(200, "text/html", html_headers + "<style>button{margin-bottom:10px;}</style><a href=/picofi><button type=button>picoFi interface</button></a> <br> <a href=/toggle-led><button type=button>Toggle LED (Currently " + (digitalRead(LED_BUILTIN) ? "on" : "off") + ")</button></a>");
  });
  server.on("/toggle-led", []() {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));    
    server.sendHeader("Location", "/");
    server.send(302);
  });
}

void picofi_loop() {
  
}
