/********************************************************************************
 * @file wifi_com.cpp
 * @brief Wifi communication Driver.
 * @author Gonzalo Puy.
 * @date Aug 2024
 *******************************************************************************/

#include "wifi_com.h"
#include "PinNames.h"
#include "arm_book_lib.h"
#include "commands.h"
#include "mbed.h"
#include <cstdio>
#include <cstring>
#include <string>

//=====[Implementations of public functions]===================================

namespace Drivers {

  void WifiCom::init()
  {
    
    getInstance()._init();
  }

  void WifiCom::update()
  {
    std::string esp32Command;
    static int startDelayTick;
    static int delayDuration;

    switch (wifiState) {

      case INIT:
      {
        wifiState = CMD_STATUS_SEND;
        wifiComDelay.Start(DELAY_3_SECONDS);
      }
      break;
      
      case CMD_STATUS_SEND:
      {
        if (wifiComDelay.HasFinished()) {
          wifiResponse.clear();
          esp32Command = COMMAND_STATUS_STR;
          esp32Command += STOP_CHAR;
          _sendCommand(esp32Command.c_str());
          wifiState = CMD_STATUS_WAIT_RESPONSE;
          wifiComDelay.Start(DELAY_3_SECONDS);
        }

      }
      break;
      
      case CMD_STATUS_WAIT_RESPONSE:
      {
        const bool isResponseCompleted = _isResponseCompleted(&wifiResponse);
        if( wifiComDelay.HasFinished() || (isResponseCompleted && (wifiResponse.compare(RESULT_NOT_CONNECTED) == 0)) ) {
          wifiState = CMD_CONNECT_SEND; //Wifi no conectado, tratando de reconectar.
        }else if( isResponseCompleted && (wifiResponse.compare(RESULT_CONNECTED) == 0) ) {
          wifiState = IDLE; //Wifi ya conectado.
        }
      }
      break;

      case CMD_CONNECT_SEND:
      {
        if(wifiComDelay.HasFinished()) {
          wifiResponse.clear();
          esp32Command = COMMAND_CONNECT_STR;
          esp32Command += PARAM_SEPARATOR_CHAR;
          esp32Command += wifiSsid;
          esp32Command += PARAM_SEPARATOR_CHAR;
          esp32Command += wifiPassword;
          esp32Command += STOP_CHAR;
          _sendCommand(esp32Command.c_str());
          wifiState = CMD_CONNECT_WAIT_RESPONSE;
          wifiComDelay.Start(DELAY_10_SECONDS);
        }
        
      }
      break;

      case CMD_CONNECT_WAIT_RESPONSE:
      {
        const bool isResponseCompleted = _isResponseCompleted(&wifiResponse);
        if( wifiComDelay.HasFinished() || (isResponseCompleted && (wifiResponse.compare(RESULT_ERROR) == 0)) ) {
          printf("WifiCom - Conection: [ERROR]\n\r");
          wifiState = INIT;
        } else if (isResponseCompleted && (wifiResponse.compare(RESULT_OK) == 0)) {
          printf("WifiCom - Conection: [OK]\n\r");
          wifiState = IDLE;
        }
      }
      break;

      case CMD_GET_SEND:
      {
        wifiCommandGetResponse.clear();
        esp32Command = COMMAND_GET_STR;
        esp32Command += PARAM_SEPARATOR_CHAR;
        esp32Command += wifiServer;
        esp32Command += STOP_CHAR;
        _sendCommand(esp32Command.c_str());
        wifiState = CMD_GET_WAIT_RESPONSE;
        wifiComDelay.Start(DELAY_3_SECONDS);
      }
      break;

      case CMD_GET_WAIT_RESPONSE:
      {
        const bool isResponseCompleted = _isResponseCompleted(&wifiResponse);
        if ((wifiComDelay.HasFinished()) || (isResponseCompleted && (wifiCommandGetResponse.compare(RESULT_ERROR) == 0))) {
          wifiState = ERROR;
        } else if (isResponseCompleted) {
          wifiState = CMD_GET_RESPONSE_READY;
          wifiIsGetResponseReady = true;
          wifiComDelay.Start(DELAY_10_SECONDS);
        }
      }
      break;

      case CMD_GET_RESPONSE_READY:
      {
        if (!wifiIsGetResponseReady) {
          wifiState = IDLE;
        } else if(wifiComDelay.HasFinished()) {
          wifiState = ERROR;
          wifiCommandGetResponse.clear();
        }
      }
      break;

      case CMD_POST_SEND:
      {
        wifiResponse.clear();
        esp32Command = COMMAND_POST_STR;
        esp32Command += PARAM_SEPARATOR_CHAR;
        esp32Command += wifiServer;
        esp32Command += PARAM_SEPARATOR_CHAR;
        esp32Command += wifiRequest;
        esp32Command += STOP_CHAR;
        _sendCommand(esp32Command.c_str());
        wifiState = CMD_POST_WAIT_RESPONSE;
        wifiComDelay.Start(DELAY_3_SECONDS);
      }
      break;

      case CMD_POST_WAIT_RESPONSE:
      {
        const bool isResponseCompleted = _isResponseCompleted(&wifiResponse);
        if ((wifiComDelay.HasFinished()) || (isResponseCompleted && (wifiResponse.compare(RESULT_ERROR) == 0))) {
          wifiState = CMD_POST_RESPONSE_READY;
          wifiResponse = RESULT_ERROR;

        } else if (isResponseCompleted) {
          wifiState = CMD_POST_RESPONSE_READY;
          wifiIsResponseReady = true;
          wifiComDelay.Start(DELAY_10_SECONDS);
        }
      }
      break;

      case CMD_POST_RESPONSE_READY:
      {
        if (!wifiIsResponseReady) {
          wifiState = IDLE;
        } else if(wifiComDelay.HasFinished()) {
          wifiState = ERROR;
          wifiResponse.clear();
        }
      }
      break;

      case ERROR:
      {
        wifiState = IDLE;
      }
      break;

      case IDLE:
      break;
    }
  } // WifiCom::update();

  bool WifiCom::isBusy()
  {
    return (wifiState != IDLE);
  }

  void WifiCom::post(const std::string &server, const std::string &request)
  {
    wifiState = CMD_POST_SEND;
    wifiServer = server;
    wifiRequest = request;
    wifiResponse.clear();
  }

  void WifiCom::request(const std::string &url)
  {
    wifiState = CMD_GET_SEND;
    wifiServer = url;
    wifiRequest = "";
    wifiCommandGetResponse.clear();
  }

  bool WifiCom::getPostResponse(std::string *response)
  {
    if (wifiState == CMD_POST_RESPONSE_READY) {
      (*response) = wifiResponse;
      wifiResponse.clear();
      wifiIsResponseReady = false;

      return true;
    }

    return false;
  }

  bool WifiCom::getGetResponse(std::string *response)
  {
    if (wifiState == CMD_GET_RESPONSE_READY) {
      (*response) = wifiCommandGetResponse;
      wifiCommandGetResponse.clear();
      wifiIsGetResponseReady = false;

      return true;
    }

    return false;
  }

//=====[Implementations of private functions]===================================

 /**
  * @brief Constructs the WifiCom object with specified UART pins and baud rate.
  * 
  * @param txPin TX pin for serial communication.
  * @param rxPin RX pin for serial communication.
  * @param baudRate Baud rate for UART.
  */
  WifiCom::WifiCom(PinName txPin, PinName rxPin, const int baudRate)
  : wifiSerial(txPin, rxPin, baudRate),
  wifiComDelay(0)
  {}

 /**
  * @brief Internal initialization routine.
  */
  void WifiCom::_init()
  {
    
    wifiState = INIT;
    wifiSsid = WIFI_SSID;
    wifiPassword = WIFI_PASSWORD;
    wifiSerial.enable_output(true);
  }

 /**
  * @brief Sends a command to the WiFi module.
  * 
  * @param command Null-terminated C string containing the command.
  */
  void WifiCom::_sendCommand(const char* command)
  {
    wifiSerial.enable_output(true);
    wifiSerial.write(command, strlen(command));
    wifiSerial.enable_output(false);
  }

 /**
  * @brief Checks if a response from the module is complete.
  * 
  * @param response Pointer to store the raw response string.
  * @return true if response is completed (e.g., "OK" or "ERROR" received).
  */
  bool WifiCom::_isResponseCompleted(std::string* response)
  {
    char receivedChar;

    if (_readCom(&receivedChar)) {
      if (receivedChar == STOP_CHAR) {
        return true;
      } else {
        (*response) += receivedChar;
      }
    }

    return false;
  }

  /**
  * @brief Reads a single character from the UART interface.
  * 
  * @param receivedChar Pointer to store received character.
  * @return true if a character was successfully read; false otherwise.
  */
  bool WifiCom::_readCom(char* receivedChar)
  {
    char receivedCharLocal = '\0';

    if (wifiSerial.readable()) {
      wifiSerial.read(&receivedCharLocal, 1);
      (*receivedChar) = receivedCharLocal;

      return true;
    }

    return false;
  }

} // namespace Drivers