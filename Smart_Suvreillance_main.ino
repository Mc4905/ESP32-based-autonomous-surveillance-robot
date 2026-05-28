#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <WiFi.h>
#include <WebServer.h>

/* ================= WIFI ================= */
const char* ssid = "SMART_ROBOT";
const char* password = "12345678";
WebServer server(80);
bool robotEnabled = false;

/* ================= SENSORS ================= */
Adafruit_AMG88xx amg;
#define MPU_ADDR 0x68

#define TRIG_PIN 5
#define ECHO_PIN 18
#define GAS_PIN 34

/* ================= MOTOR PINS ================= */
#define ENA 25
#define IN1 26
#define IN2 27
#define ENB 33
#define IN3 32
#define IN4 4

#define PWM_A 0
#define PWM_B 1

/* ================= SPEED ================= */
#define SPEED_NORMAL 135
#define SPEED_TURN 115
#define SPEED_REVERSE 115
#define SPEED_HUMAN 115

/* ================= LIMITS ================= */
#define STOP_DIST 30 
#define OBSTACLE_DIST 40
#define GAS_LIMIT 700
#define HUMAN_TEMP 34.0
#define MPU_LIMIT 6000   //Adjusted realistic tilt threshold

int16_t ax=0, ay=0, az=0;

/* ================= ULTRASONIC ================= */
long getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) return 0;

  long distance = duration * 0.034 / 2;

  if (distance < 2) return 0;
  if (distance > 400) return 400;

  return distance;
}

/* ================= MPU ================= */
void readMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  if(Wire.available()>=6){
    ax = (Wire.read()<<8) | Wire.read();
    ay = (Wire.read()<<8) | Wire.read();
    az = (Wire.read()<<8) | Wire.read();
  }
}

/* ================= MOTOR ================= */
void motorStop() {
  ledcWrite(PWM_A, 0);
  ledcWrite(PWM_B, 0);
}

void motorForward(int spd) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  ledcWrite(PWM_A, spd);
  ledcWrite(PWM_B, spd);
}

void motorReverse(int spd) {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  ledcWrite(PWM_A, spd);
  ledcWrite(PWM_B, spd);
}

void motorRight() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  ledcWrite(PWM_A, SPEED_TURN);
  ledcWrite(PWM_B, 0);
}

void motorLeft() {
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  ledcWrite(PWM_B, SPEED_TURN);
  ledcWrite(PWM_A, 0);
}

/* ================= DATA API ================= */
void handleData() {

  long distance = getDistanceCM();
  int gasValue = analogRead(GAS_PIN);

  float pixels[64];
  amg.readPixels(pixels);

  float maxTemp = -40;
  for (int i = 0; i < 64; i++)
    if (pixels[i] > maxTemp) maxTemp = pixels[i];

  readMPU();

  bool human = (maxTemp > HUMAN_TEMP && maxTemp < 45);
  bool gasAlert = (gasValue >= GAS_LIMIT);
  bool tilt = (abs(ax) > MPU_LIMIT || abs(ay) > MPU_LIMIT);

  String json = "{";
  json += "\"distance\":" + String(distance) + ",";
  json += "\"gas\":" + String(gasValue) + ",";
  json += "\"temp\":" + String(maxTemp,1) + ",";
  json += "\"human\":" + String(human ? "true":"false") + ",";
  json += "\"tilt\":" + String(tilt ? "true":"false");
  json += "}";

  server.send(200, "application/json", json);
}

/* ================= WEB PAGE ================= */
void handleRoot() {

String page =
"<!DOCTYPE html><html><head>"
"<meta name='viewport' content='width=device-width, initial-scale=1'>"
"<title>SMART ROBOT</title>"
"</head>"
"<body style='background:white;color:black;font-family:Arial;text-align:center;'>"

"<h2>SMART ROBOT DASHBOARD</h2>"
"<button onclick=\"fetch('/start')\">START</button>"
"<button onclick=\"fetch('/stop')\">STOP</button>"

"<h3>Distance: <span id='d'>--</span></h3>"
"<h3>Gas: <span id='g'>--</span></h3>"
"<h3>Temp: <span id='t'>--</span></h3>"

"<h3>Human: <span id='humanStatus'>--</span></h3>"
"<h3>Tilt: <span id='tiltStatus'>--</span></h3>"

"<script>"

"var audioCtx = new (window.AudioContext || window.webkitAudioContext)();"
"var humanDetected=false;"

"function beep(){"
"var oscillator = audioCtx.createOscillator();"
"var gainNode = audioCtx.createGain();"
"oscillator.connect(gainNode);"
"gainNode.connect(audioCtx.destination);"
"oscillator.frequency.value=1000;"
"oscillator.start();"
"setTimeout(()=>{oscillator.stop();},300);"
"}"

"setInterval(()=>{"
"fetch('/data').then(r=>r.json()).then(x=>{"

"document.getElementById('d').innerHTML=x.distance;"
"document.getElementById('g').innerHTML=x.gas;"
"document.getElementById('t').innerHTML=x.temp;"

"if(x.human){"
"document.getElementById('humanStatus').innerHTML='HUMAN DETECTED';"
"document.body.style.background='yellow';"
"if(!humanDetected){beep(); humanDetected=true;}"
"}"
"else{"
"document.getElementById('humanStatus').innerHTML='NO HUMAN';"
"document.body.style.background='white';"
"humanDetected=false;"
"}"

"if(x.tilt){"
"document.body.style.background='orange';"
"document.getElementById('tiltStatus').innerHTML='TILT DETECTED';"
"}"
"else{"
"document.getElementById('tiltStatus').innerHTML='STABLE';"
"}"

"});"
"},1000);"

"</script></body></html>";

server.send(200,"text/html",page);
}

/* ================= START / STOP ================= */
void handleStart(){ robotEnabled = true; server.send(200,"text/plain","START"); }
void handleStop(){ robotEnabled = false; motorStop(); server.send(200,"text/plain","STOP"); }

/* ================= SETUP ================= */
void setup() {

Serial.begin(115200);

pinMode(TRIG_PIN, OUTPUT);
pinMode(ECHO_PIN, INPUT);
pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

ledcSetup(PWM_A, 5000, 8);
ledcSetup(PWM_B, 5000, 8);
ledcAttachPin(ENA, PWM_A);
ledcAttachPin(ENB, PWM_B);

Wire.begin(21,22);
amg.begin();

Wire.beginTransmission(MPU_ADDR);
Wire.write(0x6B);
Wire.write(0);
Wire.endTransmission(true);

WiFi.mode(WIFI_AP);
WiFi.softAP(ssid,password);

server.on("/", handleRoot);
server.on("/data", handleData);
server.on("/start", handleStart);
server.on("/stop", handleStop);

server.begin();
}

/* ================= LOOP ================= */
void loop() {

server.handleClient();

if(!robotEnabled){
  motorStop();
  return;
}

long distance = getDistanceCM();

float pixels[64];
amg.readPixels(pixels);

float maxTemp=-40;
for(int i=0;i<64;i++)
  if(pixels[i]>maxTemp) maxTemp=pixels[i];

int gasValue = analogRead(GAS_PIN);
readMPU();

bool gasAlert = (gasValue >= GAS_LIMIT);
bool tilt = (abs(ax) > MPU_LIMIT || abs(ay) > MPU_LIMIT);

if(gasAlert || tilt){
  motorStop();
  return;
}

bool humanDetected = (maxTemp > HUMAN_TEMP && maxTemp < 45);
int speed = humanDetected ? SPEED_HUMAN : SPEED_NORMAL;

if(distance == 0 || distance <= STOP_DIST){
  motorStop();
  delay(100);
  motorReverse(SPEED_REVERSE);
  delay(300);
  motorRight(); delay(250);
  motorForward(speed); delay(300);
}
else if(distance <= OBSTACLE_DIST){
  motorRight(); delay(250);
  motorForward(speed); delay(350);
  motorLeft(); delay(250);
}
else{
  motorForward(speed);
}
}
