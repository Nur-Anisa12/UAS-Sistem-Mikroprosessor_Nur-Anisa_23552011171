#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <PubSubClient.h>

#define LED_PIN 2
#define BUTTON_PIN 0

/* ===== MQTT ===== */
const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_topic  = "esp32/dashboard";

/* ===== GLOBAL ===== */
WebServer server(80);
Preferences prefs;
WiFiClient espClient;
PubSubClient mqtt(espClient);

String ssid;
String password;

bool ledState = false;
int brightness = 0;
bool apMode = false;

/* ===== INTERRUPT ===== */
volatile bool buttonFlag = false;
void IRAM_ATTR handleButton(){
  buttonFlag = true;
}

/* ===== SOFTWARE PWM ===== */
unsigned long pwmTimer = 0;
void softPWM() {
  if (millis() - pwmTimer >= 1) {
    pwmTimer = millis();

    static int counter = 0;
    counter++;
    if(counter >= 255) counter = 0;

    digitalWrite(LED_PIN, counter < brightness ? HIGH : LOW);
  }
}

/* ===== WEB PAGE ===== */
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>ESP32 IoT Control Dashboard</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body {
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  background: linear-gradient(135deg, #0f172a 0%, #1e293b 100%);
  min-height: 100vh;
  color: #fff;
  padding: 0;
  margin: 0;
}

.header {
  background: linear-gradient(135deg, #020617 0%, #0f172a 100%);
  padding: 30px 20px;
  text-align: center;
  font-size: 24px;
  font-weight: 700;
  letter-spacing: 1.5px;
  box-shadow: 0 4px 20px rgba(0, 0, 0, 0.5);
  border-bottom: 2px solid rgba(56, 189, 248, 0.3);
}

.container {
  max-width: 420px;
  margin: 40px auto;
  padding: 20px;
}

.card {
  background: linear-gradient(145deg, #1e293b 0%, #1a2332 100%);
  border-radius: 16px;
  padding: 25px;
  margin-bottom: 24px;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.6),
              inset 0 1px 0 rgba(255, 255, 255, 0.05);
  border: 1px solid rgba(56, 189, 248, 0.1);
  transition: transform 0.3s ease, box-shadow 0.3s ease;
}

.card:hover {
  transform: translateY(-2px);
  box-shadow: 0 12px 40px rgba(0, 0, 0, 0.7),
              inset 0 1px 0 rgba(255, 255, 255, 0.05);
}

.label {
  font-size: 13px;
  opacity: 0.7;
  text-transform: uppercase;
  letter-spacing: 1px;
  font-weight: 600;
  color: #94a3b8;
}

.value {
  font-size: 32px;
  font-weight: 700;
  margin-top: 10px;
  background: linear-gradient(135deg, #38bdf8 0%, #0ea5e9 100%);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

.slider {
  width: 100%;
  margin-top: 20px;
  height: 6px;
  border-radius: 5px;
  background: linear-gradient(to right, 
    #334155 0%, 
    #38bdf8 100%);
  outline: none;
  -webkit-appearance: none;
  appearance: none;
  position: relative;
}

.slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 22px;
  height: 22px;
  border-radius: 50%;
  background: linear-gradient(135deg, #38bdf8 0%, #0ea5e9 100%);
  cursor: pointer;
  box-shadow: 0 0 0 4px rgba(56, 189, 248, 0.2),
              0 4px 12px rgba(56, 189, 248, 0.4);
  transition: all 0.3s ease;
}

.slider::-webkit-slider-thumb:hover {
  transform: scale(1.2);
  box-shadow: 0 0 0 6px rgba(56, 189, 248, 0.3),
              0 6px 16px rgba(56, 189, 248, 0.6);
}

.slider::-webkit-slider-thumb:active {
  transform: scale(1.1);
}

.slider::-moz-range-thumb {
  width: 22px;
  height: 22px;
  border-radius: 50%;
  background: linear-gradient(135deg, #38bdf8 0%, #0ea5e9 100%);
  cursor: pointer;
  border: none;
  box-shadow: 0 0 0 4px rgba(56, 189, 248, 0.2),
              0 4px 12px rgba(56, 189, 248, 0.4);
  transition: all 0.3s ease;
}

.slider::-moz-range-thumb:hover {
  transform: scale(1.2);
  box-shadow: 0 0 0 6px rgba(56, 189, 248, 0.3),
              0 6px 16px rgba(56, 189, 248, 0.6);
}

.slider::-webkit-slider-runnable-track {
  width: 100%;
  height: 6px;
  border-radius: 5px;
}

.slider::-moz-range-track {
  width: 100%;
  height: 6px;
  border-radius: 5px;
  background: #334155;
}

input {
  width: 100%;
  padding: 14px 16px;
  border-radius: 10px;
  border: 2px solid rgba(56, 189, 248, 0.2);
  margin: 8px 0;
  background: #334155;
  color: white;
  font-size: 14px;
  transition: all 0.3s ease;
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
}

input:focus {
  outline: none;
  border-color: #38bdf8;
  background: #3d4c5f;
  box-shadow: 0 0 0 3px rgba(56, 189, 248, 0.1);
}

input::placeholder {
  color: #64748b;
  opacity: 1;
}

button {
  width: 100%;
  padding: 14px;
  border: none;
  border-radius: 10px;
  font-weight: 600;
  margin-top: 12px;
  cursor: pointer;
  font-size: 15px;
  transition: all 0.3s ease;
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  position: relative;
  overflow: hidden;
}

.btn-primary {
  background: linear-gradient(135deg, #38bdf8 0%, #0ea5e9 100%);
  color: #020617;
  box-shadow: 0 4px 15px rgba(56, 189, 248, 0.4);
}

.btn-primary:hover {
  background: linear-gradient(135deg, #0ea5e9 0%, #0284c7 100%);
  transform: translateY(-2px);
  box-shadow: 0 6px 20px rgba(56, 189, 248, 0.5);
}

.btn-primary:active {
  transform: translateY(0);
  box-shadow: 0 2px 10px rgba(56, 189, 248, 0.3);
}

.footer {
  text-align: center;
  font-size: 12px;
  opacity: 0.5;
  margin-top: 30px;
  padding: 20px;
  color: #94a3b8;
  letter-spacing: 0.5px;
}

@media (max-width: 480px) {
  .container {
    padding: 15px;
    margin: 20px auto;
  }
  
  .header {
    font-size: 20px;
    padding: 24px 15px;
  }
  
  .card {
    padding: 20px;
  }
}
</style>
</head>
<body>
<div class="header">
ESP32 IoT Control Dashboard
</div>
<div class="container">
<div class="card">
<div class="label">LED Status</div>
<div id="led" class="value">-</div>
<input class="slider" type="range" min="0" max="255"
onchange="setPWM(this.value)">
</div>
<div class="card">
<div class="label">WiFi Configuration</div>
<input id="ssid" placeholder="WiFi SSID">
<input id="password" type="password" placeholder="WiFi Password">
<button class="btn-primary" onclick="saveWiFi()">
Save WiFi Settings
</button>
</div>
<div class="footer">
Nur Anisa 23552011171
</div>
</div>
<script>
function refresh(){
fetch('/status').then(r=>r.json()).then(d=>{
led.innerText=d.led?'ON':'OFF';
});
}
function setPWM(v){
fetch('/pwm?value='+v);
}
function saveWiFi(){
fetch('/wifi',{
method:'POST',
headers:{'Content-Type':'application/json'},
body:JSON.stringify({
ssid:ssid.value,
password:password.value
})
});
alert('WiFi saved. Device reconnecting...');
}
setInterval(refresh,2000);
refresh();
</script>
</body>
</html>
)rawliteral";



/* ===== MQTT TASK ===== */
void mqttTask(void *pv){
  for(;;){
    if(!mqtt.connected()){
      Serial.print("MQTT connecting... ");
      if(mqtt.connect("esp32Client")){
        Serial.println("OK");
      } else {
        Serial.print("FAILED state=");
        Serial.println(mqtt.state());
      }
    }

    mqtt.loop();

    if(mqtt.connected()){
      String msg = String("{\"led\":") + (ledState?"true":"false") +
                   ",\"brightness\":" + brightness + "}";
      mqtt.publish(mqtt_topic, msg.c_str());
      Serial.println("MQTT publish OK");
    }

    vTaskDelay(2000/portTICK_PERIOD_MS);
  }
}

/* ===== WIFI ===== */
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  unsigned long start = millis();
  while (WiFi.status()!=WL_CONNECTED && millis()-start<15000){
    delay(500);
  }

  if (WiFi.status()==WL_CONNECTED){
    apMode=false;
    Serial.println("WiFi CONNECTED");
    Serial.println(WiFi.localIP());
    MDNS.begin("esp32");
  } else {
    startAPMode();
  }
}

void startAPMode() {
  apMode=true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-Config");
  Serial.println("AP MODE: 192.168.4.1");
}

/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(BUTTON_PIN, handleButton, FALLING);

  prefs.begin("wifi", false);
  ssid = prefs.getString("ssid","");
  password = prefs.getString("password","");

  if (ssid=="") startAPMode();
  else connectWiFi();

  mqtt.setServer(mqtt_server,1883);

  server.on("/", [](){ server.send_P(200,"text/html",webpage); });

  server.on("/pwm", [](){
    brightness = server.arg("value").toInt();
    ledState = brightness > 0;
    server.send(200);
  });

  server.on("/wifi", HTTP_POST, [](){
    String body = server.arg("plain");
    int s = body.indexOf("ssid");
    int p = body.indexOf("password");

    ssid = body.substring(body.indexOf(':',s)+2, body.indexOf('"', body.indexOf(':',s)+2));
    password = body.substring(body.indexOf(':',p)+2, body.lastIndexOf('"'));

    prefs.putString("ssid", ssid);
    prefs.putString("password", password);

    server.send(200);
    WiFi.disconnect(true);
    delay(500);
    connectWiFi();
  });

  server.on("/status", [](){
    String json="{\"led\":"+String(ledState?"true":"false")+"}";
    server.send(200,"application/json",json);
  });

  server.begin();

  xTaskCreatePinnedToCore(mqttTask,"mqtt",4096,NULL,1,NULL,1);

  Serial.println("System ready");
}

/* ===== LOOP ===== */
void loop() {
  server.handleClient();
  softPWM();

  // interrupt button logic
  if(buttonFlag){
    buttonFlag = false;
    brightness += 50;
    if(brightness > 255) brightness = 0;
    ledState = brightness > 0;
  }
}
