#include <Arduino.h>
#include <functional>
#include <HTTPClient.h>
#include <map>
#include <vector>
#include <WiFi.h>
#include <ArduinoJson.h>

#include "commands.h"

#define RXD2 16
#define TXD2 17
#define LED_WIFI_STATUS 2
#define MAX_PARAMS 10

#define DEBUG_PRINTLN(format, ...) \
    Serial.printf(format "\r\n", ##__VA_ARGS__)


// Map of the possible commands and their associated function
using CommandFunction = std::function<String(const std::array<String, MAX_PARAMS>&, size_t)>;
std::map<String, CommandFunction> commandsMap;

// Functions declarations
String CommandConnectToWiFi(const std::array<String, MAX_PARAMS>& params, size_t paramCount);
String CommandPostToServer(const std::array<String, MAX_PARAMS>& params, size_t paramCount);
String CommandGet(const std::array<String, MAX_PARAMS>& params, size_t paramCount);
String CommandStatus(const std::array<String, MAX_PARAMS>& params, size_t paramCount);

std::array<String, MAX_PARAMS> _ParseParameters(const String &input, size_t &paramCount);
bool _IsConnected();

// ---------------------------------------------------------------------------------------
void setup() 
{
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

    pinMode(LED_WIFI_STATUS,OUTPUT);

    commandsMap[COMMAND_CONNECT_STR]        = CommandConnectToWiFi;
    commandsMap[COMMAND_POST_STR]           = CommandPostToServer;
    commandsMap[COMMAND_GET_STR]            = CommandGet;
    commandsMap[COMMAND_STATUS_STR]         = CommandStatus;
}

// ---------------------------------------------------------------------------------------
void loop()
{
    digitalWrite(LED_WIFI_STATUS, (_IsConnected()) ? HIGH : LOW);
    
    if (Serial2.available()) 
    {
        String commandExecutionResult;

        String strReceived = Serial2.readStringUntil(STOP_CHAR);
        strReceived.trim();
        DEBUG_PRINTLN("Command and parameters received \n\r[%s]", strReceived.c_str());

        size_t paramCount = 0;
        std::array<String, MAX_PARAMS> params = _ParseParameters(strReceived, paramCount);
        String cmd = params[0];

        if (commandsMap.find(cmd) != commandsMap.end()) 
        {
            CommandFunction commandFunction = commandsMap[cmd];
            commandExecutionResult = commandFunction(params, paramCount);

            Serial2.flush();
            Serial2.print(commandExecutionResult.c_str());
            Serial2.print(STOP_CHAR);

            DEBUG_PRINTLN("Result = [%s] sent to Nucleo Board", commandExecutionResult.c_str());
        } 
        else 
        {
            DEBUG_PRINTLN("Command [%s] not found", cmd.c_str());
        }
    }
}

// ---------------------------------------------------------------------------------------
String CommandConnectToWiFi(const std::array<String, MAX_PARAMS>& params, size_t paramCount)
{
    if (paramCount == 3) 
    {
        String ssid = params[1];
        String password = params[2];

        WiFi.begin(ssid.c_str(), password.c_str());

        DEBUG_PRINTLN("CommandConnectToWiFi - Connecting to WiFi: [%s]", ssid.c_str());

        int attempts = 0;

        while ((WiFi.status() != WL_CONNECTED) && (attempts < 10)) 
        {
            delay(1000);
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) 
        {
            DEBUG_PRINTLN("\nCommandConnectToWiFi - Connected to IP: [%s]", WiFi.localIP().toString().c_str());
            return RESULT_OK;
        } 
        else 
        {
            DEBUG_PRINTLN("CommandConnectToWiFi - Error while connecting");
            return RESULT_ERROR;
        }
    } 
    else 
    {
        DEBUG_PRINTLN("CommandConnectToWiFi- Incorrect amount of parameters [%d]", (params.size() - 1));
        return RESULT_ERROR;
    }
}

// ---------------------------------------------------------------------------------------
String CommandPostToServer(const std::array<String, MAX_PARAMS>& params, size_t paramCount)
{
    if (paramCount == 3) 
    {
        String server = params[1];
        String request = params[2];

        if (!_IsConnected()) 
        {
            DEBUG_PRINTLN("CommandPostToServer - No Connection to WiFi");
            return RESULT_ERROR;
        }

        DEBUG_PRINTLN("CommandPostToServer - Post to server = [%s]\n\r%s", server.c_str(), request.c_str());
        
        HTTPClient http;

        http.begin(server.c_str());
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        int httpResponseCode = http.POST(request);
        String response;

        if (httpResponseCode > 0) 
        {
            response = http.getString();
            DynamicJsonDocument doc(4096);  // Ajustá el tamaño según lo que recibas
            deserializeJson(doc, response); // response es tu String recibido

            serializeJsonPretty(doc, Serial);
            
            DEBUG_PRINTLN("CommandPostToServer - Success\n\r[%d]\n\r[%s]", httpResponseCode, response.c_str());
        } 
        else 
        {
            response = RESULT_ERROR;
        }

        http.end();

        return response;
    } 
    else 
    {
        DEBUG_PRINTLN("CommandPostToServer - Incorrect amount of parameters [%d]", (params.size() - 1));
        return RESULT_ERROR;
    }
}

// ---------------------------------------------------------------------------------------
String CommandGet(const std::array<String, MAX_PARAMS>& params, size_t paramCount)
{
    if (paramCount == 2) 
    {
        if (!_IsConnected()) 
        {
            DEBUG_PRINTLN("CommandGet - No Connection to WiFi");
            return RESULT_ERROR;
        }

        String url = params[1];

        HTTPClient http;
        http.begin(url.c_str());

        int httpResponseCode = http.GET();
        String response;

        if (httpResponseCode > 0) 
        {
            response = http.getString();
            DEBUG_PRINTLN("CommandGet - Success\n\r[%d]\n\r[%s]", httpResponseCode, response.c_str());
        } 
        else 
        {
            response = RESULT_ERROR;
        }

        http.end();

        return response;
    } 
    else 
    {
        DEBUG_PRINTLN("CommandGet - Incorrect amount of parameters [%d]", (params.size() - 1));
        return RESULT_ERROR;
    }
}

// ---------------------------------------------------------------------------------------
String CommandStatus(const std::array<String, MAX_PARAMS>& params, size_t paramCount)
{
    if (paramCount == 1) 
    {
        return (_IsConnected()) ? (RESULT_CONNECTED) : (RESULT_NOT_CONNECTED);
    }
    else
    {
        DEBUG_PRINTLN("CommandStatus - Incorrect amount of parameters [%d]", (params.size() - 1));
        return RESULT_ERROR;
    }
}

// ---------------------------------------------------------------------------------------
bool _IsConnected()
{
    return (WiFi.status() == WL_CONNECTED);
}
// ---------------------------------------------------------------------------------------
std::array<String, MAX_PARAMS> _ParseParameters(const String &input, size_t &paramCount) 
{
    std::array<String, MAX_PARAMS> params;
    paramCount = 0;

    int index_from = -1;
    int index_to;

    while (paramCount < MAX_PARAMS) 
    {
        index_to = input.indexOf(PARAM_SEPARATOR_CHAR, index_from + 1);
        
        if (index_to < 0)
            index_to = input.length();

        params[paramCount++] = input.substring(index_from + 1, index_to);

        if (index_to == input.length())
            break;

        index_from = index_to;
    }

    return params;
}