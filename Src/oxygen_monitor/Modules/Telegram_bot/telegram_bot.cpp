/********************************************************************************
 * @file telegram_bot.cpp
 * @brief Telegram Bot Module.
 * @author Gonzalo Puy.
 * @date Aug 2024
 *******************************************************************************/

#include "telegram_bot.h"

#include <cstdio>
#include <cstdarg>
#include <sstream>
#include "arm_book_lib.h"
#include "commands.h"
#include "wifi_com.h"
#include <string>
#include <type_traits>
#include <vector>

static Drivers::WifiCom wifiModule(WIFI_PIN_TX, WIFI_PIN_RX, WIFI_BAUD_RATE);

namespace Module {

  TelegramBot::TelegramBot(const char *apiUrl, const char *token) 
  : botUrl(apiUrl)
  , botToken(token)
  {
  }

  void TelegramBot::init()
  {
    botTankId = 0;

    functionsArray[COMMAND_START] = &TelegramBot::_commandStart;
    functionsArray[COMMAND_NEW_TANK] = &TelegramBot::_commandNewTank;
    functionsArray[COMMAND_TANK_STATUS] = &TelegramBot::_commandTankStatus;
    functionsArray[COMMAND_COMMAND_NEW_GAS_FLOW] = &TelegramBot::_commandNewGasFlow;
  }

  void TelegramBot::update()
  {
    switch (botState) {
      case INIT:
      {
        if (!wifiModule.isBusy() ) {
          botState = MONITOR;
        }
      }
      break;

      case MONITOR:
      {

      }
      break;

      case SEND_ALERT:
      {
        if (!wifiModule.isBusy()) {
          std::string messegeToSend = ALERT_TANK_EMPTY;
          messegeToSend += "\n";
          std::string tankId = _getTankId();
          _sendMessage(tankId, messegeToSend);
          botState = WAITING_RESPONSE;
        }
      }
      break;

      case REQUEST_LAST_MESSAGE:
      {
        _requestLastMessage();
        botState = WAITING_LAST_MESSAGE;
        //Timer RTC
      }
      break;

      case WAITING_LAST_MESSAGE:
      {
        std::string response;
        if (wifiModule.getPostResponse(&response)) {
          const bool isNewMessage = _getMessageFromResponse(&botLastMessage, response);
          if (isNewMessage) {
            botState = PROCESS_LAST_MESSAGE;
          } else {
            botState = INIT;
          }
        }
      }
      break;

      case PROCESS_LAST_MESSAGE:
      {
        std::string messegeToSend;
        std::vector<std::string> params = _parseMessage(botLastMessage.message);
        std::string command = params[0];

        if( (_isTankIdValid(botLastMessage.fromId)) || (command.compare(COMMAND_START_STR) == 0 )) {
          command_t command_nb = _findCommand(command);
          messegeToSend = (this->*functionsArray[command_nb])(params);
        } else {
          //messegeToSend = (this->*functionsARRAY[ERROR_INVALID_COMMAND](params));
        }

        _sendMessage(botLastMessage.fromId, messegeToSend);
        botState = WAITING_RESPONSE;
      }
      break;

      case WAITING_RESPONSE:
      {
        std::string response;
        if ((wifiModule.getPostResponse(&response) && botResponse.compare(RESULT_ERROR) == 0 )) {
          botState = INIT;
        } else {
          botState = INIT;
        }
      }
      break;
    }
  }

  // PRIVADO

  std::string _formatString(const char* format, ...)
  {
    va_list args;
    va_list ap;
    va_start(ap,format);

    int size = std::vsnprintf(nullptr, 0, format, args);

    std::string result(size + 1, '\0');
    std::vsnprintf(&result[0], result.size(), format, args);

    va_end(args);
    result.pop_back();

    return result;
  }

  bool _isStringNumeric(const std::string &str)
  {
    return str.find_first_not_of( "0123456789" ) == string::npos;
  }


  std::string TelegramBot::_commandStart(const std::vector<std::string> &params)
  {
    if (params.size() == 1) {
      _registerTankId(botLastMessage.fromId);
      return _formatString("Tank with ID: %s\nregistered correctly\n Please enter the /newTank command to continue.", botLastMessage.fromId.c_str());
    }

    return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_START_STR);
  }

  std::string _commandNewTank(const std::vector<std::string> &params)
  {
    if (params.size() == 3) {
      std::string numTankId = params[1];
      std::string numTankCapacity = params[2];

      if (_isStringNumeric(numTankId) && _isStringNumeric(numTankCapacity)) {
        int tankId = std::stoi(numTankId);
        int tankCapacity = std::stoi(numTankCapacity);
        //Verificaciones de los numeros?
        //calback para cargar nuevo tanque.
        return _formatString("[Success!]\nNew Tank with ID: [%d]\nAnd Capacity of [%d] lts.\nWas seted up.", tankId, tankCapacity);
      }

    } else if (params.size() == 4) {
      std::string numTankId = params[1];
      std::string numTankCapacity = params[2];
      std::string numTankGasFlow = params[3];

      if (_isStringNumeric(numTankId) && _isStringNumeric(numTankCapacity) && _isStringNumeric(numTankGasFlow)) {
        int tankId = std::stoi(numTankId);
        int tankCapacity = std::stoi(numTankCapacity);
        int gasFlow = std::stoi(numTankGasFlow);
        //Verificaciones de los numeros?
        //calback para cargar nuevo tanque. Con los parametros necesarios.
        return _formatString("[Success!]\nNew Tank with ID: [%d]\nCapacity of [%d] lts.\nGas flow of [%d] unidad\nWas seted up.", tankId, tankCapacity, gasFlow);
      }
    } else {
      return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_NEW_TANK_STR);
    }
  }

  std::string _commandTankStatus(const std::vector<std::string> &params)
  {

  }

  std::string _commandNewGasFlow(const std::vector<std::string> &params)
  {

  }


} // namespace Module