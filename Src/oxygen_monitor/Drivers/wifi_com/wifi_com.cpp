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

namespace Drivers {

  WifiCom::WifiCom(PinName txPin, PinName rxPin, const int baudRate) : wifiSerial(txPin, rxPin, baudRate)
  {
  }

  void WifiCom::init()
  {
    wifiState = INIT;
    wifiSsid = WIFI_SSID;
    wifiPassword = WIFI_PASSWORD;
    wifiSerial.enable_output(true);
  }

  void WifiCom::update()
  {
    std::string esp32Command;
    static int startDelayTick;
    static int delayDuration;

    switch (wifiState) {

      case INIT:
      {
        //wifiState = CMD_STATUS_SEND; //Aca en realidad deberia solo iniciar el timer del RTC y en el callback de este cambiar el estado
        //Timer con el RTC.
      }
      break;
      
      case CMD_STATUS_SEND:
      {
        wifiResponse.clear();
        esp32Command = COMMAND_STATUS_STR;
        esp32Command += STOP_CHAR;
        _sendCommand(esp32Command.c_str());
        wifiState = CMD_STATUS_WAIT_RESPONSE;
        //Volver a setear el timer del RTC. Posibleemente sea otro.
      }
      break;
      
      case CMD_STATUS_WAIT_RESPONSE:
      {
        const bool isResponseCompleted = _isResponseCompleted(&wifiResponse);
        if( isResponseCompleted && (wifiResponse.compare(RESULT_NOT_CONNECTED)) == 0 ) {
          wifiState = CMD_CONNECT_SEND; //Wifi no conectado, tratando de reconectar. Probablemente otro timer del RTC
        }else if( isResponseCompleted && (wifiResponse.compare(RESULT_CONNECTED)) == 0 ) {
          wifiState = IDLE; //Wifi ya conectado.
        }
      }
      break;

      case CMD_CONNECT_SEND:
      {
        wifiResponse.clear();
        esp32Command = COMMAND_CONNECT_STR;
        esp32Command += PARAM_SEPARATOR_CHAR;
        esp32Command += wifiSsid;
        esp32Command += PARAM_SEPARATOR_CHAR;
        esp32Command += wifiPassword;
        esp32Command += STOP_CHAR;
        _sendCommand(esp32Command.c_str());
        //Iniciar Otro Timer del RTC o uno mismo, despues se ve.
      }
      break;

      case CMD_CONNECT_WAIT_RESPONSE:
      {
        const bool isResponseCompleted = _isResponseCompleted(&wifiResponse);
        if( isResponseCompleted && (wifiResponse.compare(RESULT_ERROR)) == 0 ) {
          wifiState = CMD_ACCESSPOINT_SEND;
        } else if (isResponseCompleted && (wifiResponse.compare(RESULT_OK)) == 0) {
          wifiState = IDLE;
        }
      }
      break;

      case CMD_ACCESSPOINT_SEND:
      {
        esp32Command = COMMAND_ACCESSPOINT_STR;
        esp32Command += STOP_CHAR;
        _sendCommand(esp32Command.c_str());
        //Iniciar Timer CMD_ACCESPOINT_WAIT_TIMER
      }
      break;

      case CMD_ACCESSPOINT_WAIT_RESPONSE:
      {
        const bool isResponseCompleted = _isResponseCompleted(&wifiResponse);
        if( isResponseCompleted && (wifiResponse.compare(RESULT_ERROR)) == 0 ) {
          wifiState = CMD_ACCESSPOINT_SEND;
        } else if (isResponseCompleted) {
          int paramIndex = wifiResponse.find(PARAM_SEPARATOR_CHAR);
          if (paramIndex != std::string::npos) {
            wifiSsid = wifiResponse.substr(0, paramIndex);
            wifiPassword = wifiResponse.substr(paramIndex + 1, wifiResponse.length());
            wifiState = CMD_CONNECT_SEND; //U otro timer RTC hay que ver.
          }
        } else {
          wifiState = CMD_ACCESSPOINT_SEND;
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
        // Timer RTC (otro mas y van...)
      }
      break;

      case CMD_GET_WAIT_RESPONSE:
      {
        const bool isResponseCompleted = _isResponseCompleted(&wifiResponse);
        if( isResponseCompleted && (wifiResponse.compare(RESULT_ERROR)) == 0 ) {
          wifiState = ERROR;
        } else if (isResponseCompleted) {
          wifiState = CMD_GET_RESPONSE_READY;
          wifiIsGetResponseReady = true;
          // Otro Timer? Setear la variable de arriba en el callback ?
        }
      }
      break;

      case CMD_GET_RESPONSE_READY:
      {
        if (!wifiIsGetResponseReady) {
          wifiState = IDLE;
        } else {
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
        //Otro Timer
      }
      break;

      case CMD_POST_WAIT_RESPONSE:
      {
        const bool isResponseCompleted = _isResponseCompleted(&wifiResponse);
        if( isResponseCompleted && (wifiResponse.compare(RESULT_ERROR)) == 0 ) {
          wifiState = CMD_POST_RESPONSE_READY;
          wifiResponse = RESULT_ERROR;

        } else if (isResponseCompleted) {
          wifiState = CMD_POST_RESPONSE_READY;
          wifiIsResponseReady = true;
          // Otro Timer? Setear las variables de arriba en el callback  y volver a este estado?
        }
      }
      break;

      case CMD_POST_RESPONSE_READY:
      {
        if (!wifiIsResponseReady) {
          wifiState = IDLE;
        } else {
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

  // PRIVADOO

  void WifiCom::_sendCommand(const char* command)
  {
    wifiSerial.enable_output(true);
    wifiSerial.write(command, strlen(command));
    wifiSerial.enable_output(false);
  }

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