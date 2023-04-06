
#include <FS.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <AsyncTCP.h>
#include <Ticker.h>
#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

Ticker timer;


const char* ssid = "LA1.404";
const char* password = "bmeiulab404";

//bsocketsServer webSocket = WebSocketsServer(81);  

char webpage[] PROGMEM = R"=====(
<html>
<!-- Adding a data chart using Chart.js -->
<head>
  <script src='https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.5.0/Chart.min.js'></script>
</head>
<body onload="javascript:init()">
<!-- Adding a slider for controlling data rate -->
<hr />
<div>
  <canvas id="line-chart" width="800" height="450"></canvas>
</div>
<!-- Adding a websocket to the client (webpage) -->
<script>
  var webSocket, dataPlot;
  var maxDataPoints = 150;
  function removeData(){
    dataPlot.data.labels.shift();
    dataPlot.data.datasets[0].data.shift();
  }
  function addData(label, data) {
    if(dataPlot.data.labels.length > maxDataPoints) removeData();
    dataPlot.data.labels.push(label);
    dataPlot.data.datasets[0].data.push(data);
    dataPlot.update();
  }
  function init() {
    webSocket = new WebSocket('ws://' + window.location.hostname + ':81/');
    dataPlot = new Chart(document.getElementById("line-chart"), {
      type: 'line',
      data: {
        labels: [],
        datasets: [{
          data: [],
          label: "ECG",
          borderColor: "#3e95cd",
          fill: false
        }]
      }
    });
    webSocket.onmessage = function(event) {
      var data = JSON.parse(event.data);
      var today = new Date();
      var t = today.getHours() + ":" + today.getMinutes() + ":" + today.getSeconds();
      addData(t, data.value);
    }
  }
/*  function sendDataRate(){
    var dataRate = document.getElementById("dataRateSlider").value;
    webSocket.send(dataRate);
    dataRate = 1.0/dataRate;
    document.getElementById("dataRateLabel").innerHTML = "Rate: " + dataRate.toFixed(2) + "Hz";
  }*/
</script>
</body>
</html>
)=====";

/*IPAddress staticIP(192, 168, 100, 15);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);
//IPAddress dns(192, 168, 1, 254);
//IPAddress primaryDNS(8, 8, 8, 8);   
//IPAddress secondaryDNS(8, 8, 4, 4);*/

void setup() {
//  if (!WiFi.config(staticIP, gateway, subnet)) {
//    Serial.println("STA Failed to configure");
  
    // Connect to Wi-Fi
 // WiFi.begin(ssid, NULL);
   WiFi.begin(ssid, password);
//  WiFi.mode(WIFI_STA);
 // WiFiManager wm;
 // wm.resetSettings();
 /* bool res;
    res = wm.autoConnect("cykablyat");
    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    }  
  IPAddress _ip = IPAddress(10, 0, 1, 78);
  IPAddress _gw = IPAddress(10, 0, 1, 1);
  IPAddress _sn = IPAddress(255, 255, 255, 0);
  wm.setSTAStaticIPConfig(_ip, _gw, _sn);*/
  Serial.begin(115200);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.subnetMask());
  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.dnsIP(0));
  Serial.println(WiFi.dnsIP(1));

  // initialize the serial communication:
   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", webpage);
  });


  // Start server
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  timer.attach(0.025,getData);
  }

 
void loop() {
  // put your main code here, to run repeatedly:
  webSocket.loop();
 // server.handleClient();

}

void getData() {
    String json = "{\"value\":";
    json += analogRead(A0);
    json += "}";
    webSocket.broadcastTXT(json.c_str(), json.length());
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t*payload, size_t length){
  if(type == WStype_TEXT){
    float dataRate = (float) atof((const char*)&payload[0]);
    timer.detach();
    timer.attach(dataRate, getData);
  }
}
