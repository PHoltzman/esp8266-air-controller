#include <Ticker.h>
#include <ESP8266WiFi.h>            
#include <ESP8266WebServer.h>
#include <EEPROM.h>

/* define custom things here such as pins, Wifi SSID and password, and a password (APIKEY)
   for authorizing calls to your web service.
*/
////////////////////////////////////////////////////////
#define LEFT_PIN 0
#define RIGHT_PIN 2
#define NETWORK_NAME "my_network_ssid"
#define NETWORK_PASSWORD "my_password"
const String APIKEY = "my_super_secret_password_that_i_made_up";
/////////////////////////////////////////////////////////

const String LEFT = "left";
const String RIGHT = "right";
const byte LEFT_ADDR = 0;
const byte RIGHT_ADDR = 8;


ESP8266WebServer server(80);   //Web server object. Will be listening in port 80 (default for HTTP)

// define the ticker objects. These are the timers that control when to pulse and for how long
Ticker leftTicker;
Ticker leftPulseTicker;
Ticker rightTicker;
Ticker rightPulseTicker;

// global variables needed because it is hard to pass variables into a Ticker callback function
int LEFT_PULSE_MILLIS = 0;
int RIGHT_PULSE_MILLIS = 0;

// structure for reading and writing data to EEPROM storage (technically EEPROM emulated on Flash memory
// for the ESP8266
struct {
  int pulseMillis = 0;
  int delaySecs = 0;
} record;

void setup() {
  pinMode(LEFT_PIN, OUTPUT);    
  pinMode(RIGHT_PIN, OUTPUT);

  Serial.begin(115200);

  WiFi.begin(NETWORK_NAME, NETWORK_PASSWORD); //Connect to the WiFi network
  while (WiFi.status() != WL_CONNECTED) { //Wait for connection
    delay(500);
  }
  Serial.println("Wifi Connected");

  EEPROM.begin(16);
  
  //read current data from EEPROM and setup timers
  EEPROM.get(LEFT_ADDR, record);
  updateTicker(LEFT, record.pulseMillis, record.delaySecs);
  EEPROM.get(RIGHT_ADDR, record);
  updateTicker(RIGHT, record.pulseMillis, record.delaySecs);

  //Associate the handler functions to the path
  server.on("/settings", handleSettings);
  server.on("/pulse", handlePulse);
  server.on("/read", handleRead);

  // Start the webserver
  server.begin();
  Serial.println("Setup Complete");
}

void loop() {
  server.handleClient();    //Handling of incoming requests
}

/*
 * Consistently format data for printing to Serial output.
 */
String getLogParams(String side, int pulseMillis = -1, int delaySecs = -1) {
  String temp;
  temp = "side[" + side + "]";
  if (pulseMillis != -1) {
    temp = temp + ", pulse[" + String(pulseMillis) + "]";
  }
  if (delaySecs != -1) {
    temp = temp + ", delay[" + String(delaySecs) + "]";
  }
  return temp;
}

/*
 * Handler for the ticker controlling the left solenoid
 */
void leftTickerHandler() {
  startPulse(LEFT, LEFT_PULSE_MILLIS);
}

/*
 * Handler for the ticker controlling stopping the pulse of the left solenoid
 */
void leftTickerStopHandler() {
  digitalWrite(LEFT_PIN, LOW);
  leftPulseTicker.detach();
  Serial.println("Pulse complete: " + getLogParams(LEFT));
}

/*
 * Handler for the ticker controlling the left solenoid
 */
void rightTickerHandler() {
  startPulse(RIGHT, RIGHT_PULSE_MILLIS);
}

/*
 * Handler for the ticker controlling stopping the pulse of the left solenoid
 */
void rightTickerStopHandler() {
  digitalWrite(RIGHT_PIN, LOW);
  rightPulseTicker.detach();
  Serial.println("Pulse complete: " + getLogParams(RIGHT));
}

/*
 * Update a ticker when settings change
 */
void updateTicker(String side, int pulseMillis, int delaySecs) {
  Serial.println("Updating ticker: " + getLogParams(side, pulseMillis, delaySecs));
  if (side == LEFT) {
    LEFT_PULSE_MILLIS = pulseMillis;
    leftTicker.detach();
    leftTicker.attach(delaySecs, leftTickerHandler);
  } else if (side == RIGHT) {
    RIGHT_PULSE_MILLIS = pulseMillis;
    rightTicker.detach();
    rightTicker.attach(delaySecs, rightTickerHandler);
  }
}

/*
 * Start a pulse of a solenoid and setup the Ticker for stopping the pulse.
 */
void startPulse(String side, int pulseMillis) {
  if (pulseMillis > 0) {
    if (side == LEFT) {
      Serial.println("Running pulse: " + getLogParams(side, pulseMillis));
      digitalWrite(LEFT_PIN, HIGH);
      leftPulseTicker.attach_ms(pulseMillis, leftTickerStopHandler);
    } else if (side == RIGHT) {
      Serial.println("Running pulse: " + getLogParams(side, pulseMillis));
      digitalWrite(RIGHT_PIN, HIGH);
      rightPulseTicker.attach_ms(pulseMillis, rightTickerStopHandler);
    } else {
      Serial.println("Skipping pulse: " + getLogParams(side, pulseMillis));
    }
  } else {
    Serial.println("Skipping pulse: " + getLogParams(side, pulseMillis));
  }
}

/*
 * Write settings to EEPROM.
 */
void writeData(String side, int pulseMillis, int delaySecs) {
  byte addr;
  if (side == LEFT) {
    addr = LEFT_ADDR;
  } else if (side == RIGHT) {
    addr = RIGHT_ADDR;
  }

  Serial.println("Updating EEPROM");
  EEPROM.get(addr, record);
  Serial.println("From: " + getLogParams(side, record.pulseMillis, record.delaySecs));
  record.pulseMillis = pulseMillis;
  record.delaySecs = delaySecs;
  Serial.println("To: " + getLogParams(side, pulseMillis, delaySecs));
  EEPROM.put(addr, record);
  EEPROM.commit();
  Serial.println("EEPROM Update Complete");
}

/*
 * Handler for RESTful GET request for a manual pulse of a solenoid.
 * Processes the URL arguments and kicks off the pulse.
 */
void handlePulse() {
  String message;
  int statusCode = 200;
  String side = "none";
  int pulseMillis = 1000;
  String apikey = "none";

  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "side") {
      side = server.arg(i);
    } else if (server.argName(i) == "pulse") {
      pulseMillis = server.arg(i).toInt();
    } else if (server.argName(i) == "apikey") {
      apikey = server.arg(i);
    }
  }

  if (apikey != APIKEY) {
    message = "Incorrect apikey";
    statusCode = 400;
    Serial.println(message);
    server.send(statusCode, "text/plain", message);       //Response to the HTTP request
  } else if (side == "none" || pulseMillis < 0) {
    message = "Must specify left or right side and provide pulse in millis";
    statusCode = 400;
    Serial.println(message);
    server.send(statusCode, "text/plain", message);       //Response to the HTTP request
  } else {
    message = "Running manual pulse: " + getLogParams(side, pulseMillis);
    statusCode = 200;
    server.send(statusCode, "text/plain", message);       //Response to the HTTP request
    
    //run the pulse
    Serial.println(message);
    startPulse(side, pulseMillis);
  }
}

/*
 * Handler for RESTful GET request for changing the settings
 * Processes the URL arguments, updates the appropriate ticker, and saves the data to EEPROM.
 */
void handleSettings() {
  String message;
  String argsMsg;
  String side = "none";
  int pulseMillis = 0;
  int delaySecs = 600;
  int statusCode = 200;
  String apikey = "none";
  
  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "side") {
      side = server.arg(i);
    } else if (server.argName(i) == "pulse") {
      pulseMillis = server.arg(i).toInt();
    } else if (server.argName(i) == "delay") {
      delaySecs = server.arg(i).toInt();
    } else if (server.argName(i) == "apikey") {
      apikey = server.arg(i);
    }
  }

  argsMsg = getLogParams(side, pulseMillis, delaySecs);
  if (apikey != APIKEY) {
    message = "Incorrect apikey";
    statusCode = 400;
    Serial.println(message);
    server.send(statusCode, "text/plain", message);       //Response to the HTTP request
  } else if (side == "none" || pulseMillis < 0 || delaySecs < 1) {
    message = "Settings Rejected: " + argsMsg;
    statusCode = 400;
  } else {
    //write the data to EEPROM
    writeData(side, pulseMillis, delaySecs);

    //update the timers
    updateTicker(side, pulseMillis, delaySecs);

    message = "Settings Accepted: " + argsMsg;
    statusCode = 200;
  }
  server.send(statusCode, "text/plain", message);       //Response to the HTTP request
} 

/*
 * Handler for RESTful GET request for reading the current settings.
 * Processes the URL arguments and reads the settings from EEPROM and returns them
 */
void handleRead() {
  String message;
  String apikey = "none";
  
  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "apikey") {
      apikey = server.arg(i);
    }
  }

  if (apikey != APIKEY) {
    message = "Incorrect apikey";
    Serial.println(message);
    server.send(400, "text/plain", message);       //Response to the HTTP request
  } else {
    EEPROM.get(LEFT_ADDR, record);
    message = getLogParams(LEFT, record.pulseMillis, record.delaySecs);
    EEPROM.get(RIGHT_ADDR, record);
    message = message + "\n" + getLogParams(RIGHT, record.pulseMillis, record.delaySecs);

    Serial.println("Reading settings:\n" + message);
    server.send(200, "text/plain", message);       //Response to the HTTP request
  }
}

