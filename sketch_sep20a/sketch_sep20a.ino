#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <ESP32Servo.h>
#include <iostream>
#include <sstream>

struct ServoPins
{
  Servo servo;
  int servoPin;
  String servoName;
  int initialPosition;  
};

// Adjusted pins for ESP32 DevKit (Check that the chosen pins are not reserved for other functions)
std::vector<ServoPins> servoPins = 
{
  { Servo(), 27 , "Base", 90},     // Keep pin 27 for Base
  { Servo(), 14 , "Shoulder", 90}, // Use pin 14 for Shoulder (as an example)
  { Servo(), 12 , "Elbow", 90},    // Use pin 12 for Elbow
  { Servo(), 13 , "Gripper", 90},  // Use pin 13 for Gripper
};

struct RecordedStep
{
  int servoIndex;
  int value;
  int delayInStep;
};
std::vector<RecordedStep> recordedSteps;

bool recordSteps = true;
bool playRecordedSteps = true;

unsigned long previousTimeInMilli = millis();

const char* ssid     = "Redmi9";
const char* password = "45454455";

AsyncWebServer server(80);
AsyncWebSocket wsRobotArmInput("/RobotArmInput");

const char* htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html>
  <head>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
    <style>

    input[type=button]
    {
      background-color:red;color:white;border-radius:30px;width:100%;height:40px;font-size:20px;text-align:center;
    }
        
    .noselect {
      -webkit-touch-callout: none; /* iOS Safari */
        -webkit-user-select: none; /* Safari */
         -khtml-user-select: none; /* Konqueror HTML */
           -moz-user-select: none; /* Firefox */
            -ms-user-select: none; /* Internet Explorer/Edge */
                user-select: none; /* Non-prefixed version, currently
                                      supported by Chrome and Opera */
    }

    .slidecontainer {
      width: 100%;
    }

    .slider {
      -webkit-appearance: none;
      width: 100%;
      height: 20px;
      border-radius: 5px;
      background: #d3d3d3;
      outline: none;
      opacity: 0.7;
      -webkit-transition: .2s;
      transition: opacity .2s;
    }

    .slider:hover {
      opacity: 1;
    }
  
    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 40px;
      height: 40px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }

    .slider::-moz-range-thumb {
      width: 40px;
      height: 40px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }

    </style>
  
  </head>
  <body class="noselect" align="center" style="background-color:white">
     
    <h1 style="color: teal;text-align:center;">HARI</h1>
    <h2 style="color: teal;text-align:center;">Robot Arm Control</h2>
    
    <table id="mainTable" style="width:400px;margin:auto;table-layout:fixed" CELLSPACING=10>
      <tr/><tr/>
      <tr/><tr/>
      <tr>
        <td style="text-align:left;font-size:25px"><b>Gripper:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Gripper" oninput='sendButtonInput("Gripper",value)' >
          </div>
        </td>
      </tr> 
      <tr/><tr/>
      <tr>
        <td style="text-align:left;font-size:25px"><b>Elbow:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Elbow" oninput='sendButtonInput("Elbow",value)' >
          </div>
        </td>
      </tr> 
      <tr/><tr/>      
      <tr>
        <td style="text-align:left;font-size:25px"><b>Shoulder:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Shoulder" oninput='sendButtonInput("Shoulder",value)' >
          </div>
        </td>
      </tr>  
      <tr/><tr/>      
      <tr>
        <td style="text-align:left;font-size:25px"><b>Base:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Base" oninput='sendButtonInput("Base",value)' >
          </div>
        </td>
      </tr> 
      <tr/><tr/> 
      <tr>
        <td style="text-align:left;font-size:25px"><b>Record:</b></td>
        <td><input type="button" id="Record" value="OFF" ontouchend='onclickButton(this)'></td>
        <td></td>
      </tr>
      <tr/><tr/> 
      <tr>
        <td style="text-align:left;font-size:25px"><b>Play:</b></td>
        <td><input type="button" id="Play" value="OFF" ontouchend='onclickButton(this)'></td>
        <td></td>
      </tr>      
    </table>
  
    <script>
      var webSocketRobotArmInputUrl = "ws:\/\/" + window.location.hostname + "/RobotArmInput";      
      var websocketRobotArmInput;
      
      function initRobotArmInputWebSocket() 
      {
        websocketRobotArmInput = new WebSocket(webSocketRobotArmInputUrl);
        websocketRobotArmInput.onopen    = function(event){};
        websocketRobotArmInput.onclose   = function(event){setTimeout(initRobotArmInputWebSocket, 2000);};
        websocketRobotArmInput.onmessage    = function(event)
        {
          var keyValue = event.data.split(",");
          var button = document.getElementById(keyValue[0]);
          button.value = keyValue[1];
          if (button.id == "Record" || button.id == "Play")
          {
            button.style.backgroundColor = (button.value == "ON" ? "green" : "red");  
            enableDisableButtonsSliders(button);
          }
        };
      }
      
      function sendButtonInput(key, value) 
      {
        var data = key + "," + value;
        websocketRobotArmInput.send(data);
      }
      
      function onclickButton(button) 
      {
        button.value = (button.value == "ON") ? "OFF" : "ON" ;        
        button.style.backgroundColor = (button.value == "ON" ? "green" : "red");          
        var value = (button.value == "ON") ? 1 : 0 ;
        sendButtonInput(button.id, value);
        enableDisableButtonsSliders(button);
      }
      
      function enableDisableButtonsSliders(button)
      {
        if(button.id == "Play")
        {
          var disabled = "auto";
          if (button.value == "ON")
          {
            disabled = "none";            
          }
          document.getElementById("Gripper").style.pointerEvents = disabled;
          document.getElementById("Elbow").style.pointerEvents = disabled;          
          document.getElementById("Shoulder").style.pointerEvents = disabled;          
          document.getElementById("Base").style.pointerEvents = disabled; 
          document.getElementById("Record").style.pointerEvents = disabled;
        }
        if(button.id == "Record")
        {
          var disabled = "auto";
          if (button.value == "ON")
          {
            disabled = "none";            
          }
          document.getElementById("Play").style.pointerEvents = disabled;
        }        
      }
           
      window.onload = initRobotArmInputWebSocket;
      document.getElementById("mainTable").addEventListener("touchend", function(event){
        event.preventDefault()
      });      
    </script>
  </body>    
</html>
)HTMLHOMEPAGE";

void handleRoot(AsyncWebServerRequest *request)
{
  request->send(200, "text/html", htmlHomePage);
}

void onWsRobotArmInputEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_DATA)
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
      String msg = "";
      for (size_t i = 0; i < len; i++)
      {
        msg += (char)data[i];
      }
      std::stringstream ss(msg.c_str());
      std::string key;
      if (getline(ss, key, ','))
      {
        std::string valueStr;
        if (getline(ss, valueStr, ','))
        {
          if (key == "Play")
          {
            if (valueStr == "1")
            {
              playRecordedSteps = true;
            }
            else
            {
              playRecordedSteps = false;
            }
          }
          if (key == "Record")
          {
            if (valueStr == "1")
            {
              recordSteps = true;
            }
            else
            {
              recordSteps = false;
            }
          }
          else
          {
            int servoValue = atoi(valueStr.c_str());
            for (int i = 0; i < servoPins.size(); i++)
            {
              if (servoPins[i].servoName == key.c_str())
              {
                servoPins[i].servo.write(servoValue);
                if (recordSteps == true)
                {
                  recordedSteps.push_back({i, servoValue, (int)millis()});
                }
              }
            }
          }
        }
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  for (int i = 0; i < servoPins.size(); i++)
  {
    servoPins[i].servo.attach(servoPins[i].servoPin);
    servoPins[i].servo.write(servoPins[i].initialPosition);
  }

  server.on("/", HTTP_GET, handleRoot);
  wsRobotArmInput.onEvent(onWsRobotArmInputEvent);
  server.addHandler(&wsRobotArmInput);
  server.begin();
}

void loop()
{
  if (playRecordedSteps)
  {
    for (int i = 0; i < recordedSteps.size(); i++)
    {
      unsigned long currentTimeInMilli = millis();
      if (previousTimeInMilli + recordedSteps[i].delayInStep < currentTimeInMilli)
      {
        previousTimeInMilli = currentTimeInMilli;
        servoPins[recordedSteps[i].servoIndex].servo.write(recordedSteps[i].value);
      }
    }
  }
}
