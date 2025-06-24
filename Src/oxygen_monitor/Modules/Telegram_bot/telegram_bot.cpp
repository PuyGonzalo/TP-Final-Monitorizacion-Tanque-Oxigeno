/****************************************************************************//**
 * @file telegram_bot.cpp
 * @brief Telegram Bot Module.
 * @author Gonzalo Puy.
 * @date Aug 2024
 *******************************************************************************/

#include "telegram_bot.h"

#include <cstdio>
#include <cstdarg>
#include <sstream>
#include "Ticker.h"
#include "arm_book_lib.h"
#include "commands.h"
#include "wifi_com.h"
#include <string>
#include <type_traits>
#include <vector>
#include <ArduinoJson.h>

//=====[Declaration and initialization of private global variables]============

static Timeout tBotTimeout;                   /**<  */
static bool isTimeoutFinished;                /**<  */

static Timeout alertTimeout;                  /**<  */
static bool isAlertTimeoutFinished;           /**<  */

static void onTBotTimeoutFinishedCallback();
static void onAlertTimeoutFinishedCallback();

//=====[Implementations of public functions]===================================

namespace Module {

  TelegramBot::TelegramBot(const char *apiUrl, const char *token) 
  : botUrl(apiUrl)
  , botToken(token)
  , botDelay(0)
  , userId(MAX_USER_COUNT)
  {
  }

  void TelegramBot::init()
  {
    botLastUpdateId = 0;
    std::fill(userId.begin(), userId.end(), "");
    userCount = 0;
    isTimeoutFinished = false;
    isAlertTimeoutFinished = true; //El estado inicial de esta variable DEBE ser true.

    functionsArray[COMMAND_START] = &TelegramBot::_commandStart;
    functionsArray[COMMAND_NEW_TANK] = &TelegramBot::_commandNewTank;
    functionsArray[COMMAND_TANK_STATUS] = &TelegramBot::_commandTankStatus;
    functionsArray[COMMAND_COMMAND_NEW_GAS_FLOW] = &TelegramBot::_commandNewGasFlow;
    functionsArray[COMMAND_END] = &TelegramBot::_commandEnd;
  }

  void TelegramBot::update()
  {
    std::chrono::microseconds remainingTime;
    std::chrono::microseconds remainingAlertTime;

    switch (botState) {
      case INIT:
      {
        if (!Drivers::WifiCom::getInstance().isBusy() ) {
          botState = MONITOR;
        }
      }
      break;

      case MONITOR:
      {
        if (!(Module::TankMonitor::getInstance().getTankState())) {

          if(isAlertTimeoutFinished) {
          isTimeoutFinished = false;
          alertTimeout.detach();
          alertTimeout.attach(&onAlertTimeoutFinishedCallback, 5s);
          botState = SEND_ALERT;
          }
          
        } else {
          botState = REQUEST_LAST_MESSAGE;
          isTimeoutFinished = false;
          tBotTimeout.detach();
          tBotTimeout.attach(&onTBotTimeoutFinishedCallback,2s);
        }
      }
      break;

      case SEND_ALERT:
      {
        if (!Drivers::WifiCom::getInstance().isBusy()) {
          std::string messegeToSend = ALERT_TANK_EMPTY;
          messegeToSend += "\n";
          std::string userId = _getUserId();
          _sendMessage(userId, messegeToSend);
          botState = WAITING_RESPONSE;
        }
      }
      break;

      case REQUEST_LAST_MESSAGE:
      {
        if (isTimeoutFinished && !(Drivers::WifiCom::getInstance().isBusy())) {
          _requestLastMessage();
          isTimeoutFinished = false;
          tBotTimeout.detach();
          tBotTimeout.attach(&onTBotTimeoutFinishedCallback,8000ms);
          botState = WAITING_LAST_MESSAGE;
        }
        
      }
      break;

      case WAITING_LAST_MESSAGE:
      {
        if (isTimeoutFinished) {
          botState = INIT;
        }
        else if (Drivers::WifiCom::getInstance().getPostResponse(&botResponse)) {
          const bool isNewMessage = _getMessageFromResponse(&botLastMessage, botResponse);
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

        if( (_isUserIdValid(botLastMessage.fromId)) || (command.compare(COMMAND_START_STR) == 0 )) {
          command_t command_nb = _findCommand(command); //Agregar verificacion de comando
          if (command_nb != ERROR_INVALID_COMMAND)
          {
            messegeToSend = (this->*functionsArray[command_nb])(params);
          }else {
            messegeToSend = _formatString(ERROR_INVALID_COMMAND_STR, command.c_str());
          }
        } else {
          messegeToSend = _formatString(ERROR_INVALID_USER_STR, botLastMessage.fromName.c_str());
        }

        _sendMessage(botLastMessage.fromId, messegeToSend);
        isTimeoutFinished = false;
        tBotTimeout.detach();
        tBotTimeout.attach(&onTBotTimeoutFinishedCallback,5000ms);
        botState = WAITING_RESPONSE;
      }
      break;

      case WAITING_RESPONSE:
      {
        std::string response;
        if (isTimeoutFinished || (Drivers::WifiCom::getInstance().getPostResponse(&response) && botResponse.compare(RESULT_ERROR) == 0 )) {
          botState = INIT;
        } else if (Drivers::WifiCom::getInstance().getPostResponse(&response)) {
          botState = INIT;
        }
      }
      break;
    }
  }

//=====[Implementations of public functions]===================================

  /**
  * @brief 
  * @note 
  * @param 
  * @return
  */
  std::string TelegramBot::_commandStart(const std::vector<std::string> &params)
  {
    if (params.size() == 1) {
      std::string result;
      printf("Entro aca\r\n");
      bool registerSuccess = _registerUser(botLastMessage.fromId);

      if (registerSuccess)
      {
        result = _formatString("User registered correctly\nHello %s!", botLastMessage.fromName.c_str());
        // Debug:
        for (int i = 0; i < userCount; ++i) { printf("Success! users ID:[ %s ]\r\n", userId[i].c_str()); }
      } else {
        result = _formatString("User '%s' is already registered!", botLastMessage.fromName.c_str());
      }
      return result;
    }

    return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_START_STR);
  }

  /**
  * @brief 
  * @note 
  * @param 
  * @return
  */
  std::string TelegramBot::_commandNewTank(const std::vector<std::string> &params)
  {
    if (params.size() == 3) {
      std::string numTankId = params[1];
      std::string numTankCapacity = params[2];

      if (_isStringNumeric(numTankId) && _isStringNumeric(numTankCapacity)) {
        int tankId = std::stoi(numTankId);
        int tankCapacity = std::stoi(numTankCapacity);
        //Verificaciones de los numeros?
        //calback para cargar nuevo tanque.
        return _formatString("[Success!]\nNew Oxygen Tank with ID: [%d]\nAnd Capacity of [%d] lts.\nWas seted up.", tankId, tankCapacity);
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

    return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_NEW_TANK_STR);
  }

  /**
  * @brief 
  * @note 
  * @param 
  * @return
  */
  std::string TelegramBot::_commandTankStatus(const std::vector<std::string> &params)
  {
    return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_NEW_TANK_STR);
  }

  /**
  * @brief 
  * @note 
  * @param 
  * @return
  */
  std::string TelegramBot::_commandNewGasFlow(const std::vector<std::string> &params)
  {
    return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_NEW_TANK_STR);
  }

  /**
  * @brief 
  * @note 
  * @param 
  * @return
  */
  std::string TelegramBot::_commandEnd(const std::vector<std::string> &params)
  {
    if (params.size() == 1) {
      std::string result;
      bool unregisterSuccess = _unregisterUser(botLastMessage.fromId);

      if (unregisterSuccess)
      {
        result = _formatString("User removed correctly\nGoodbye %s!", botLastMessage.fromName.c_str());
      } else {
        result = _formatString("User '%s' is not registered!\nUse '/start' command if you want to register", botLastMessage.fromName.c_str());
      }
      return result;
    }

    return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_START_STR);
  }

  /**
  * @brief 
  * @note 
  * @param 
  * @return
  */
  bool TelegramBot::_registerUser(std::string newUserId)
  {
    bool userFound = _isUserIdValid(newUserId);
    
    if (!userFound)
    {
      if (userCount == MAX_USER_COUNT){
        return false;
      }
      userId.insert(userId.begin(), newUserId);  // Insert at beginning
      userCount++;
      return true;
    }
    return false;
  }

  /**
  * @brief 
  * @note 
  * @param 
  * @return
  */
  bool TelegramBot::_unregisterUser(std::string oldUserId)
  {
    std::vector<std::string>::iterator it = std::find(userId.begin(), userId.end(), oldUserId);
    
    if (it != userId.end()) {
      userId.erase(it);
      userCount--;
      return true;
    } else {
      return false;
    }
    
  }

  /**
  * @brief 
  * @note 
  * @param 
  * @return
  */
  bool TelegramBot::_isUserIdValid(std::string fUserId)
  {
    std::vector<std::string>::iterator it = std::find(userId.begin(), userId.end(), fUserId);

    if (it != userId.end())
    {
      return true;
    }

    return false;
  }

  std::string TelegramBot::_getUserId()
  {
    return USER_ID;
  }

  /**
  * @brief 
  * @note 
  * @param 
  * @return
  */
  std::vector<std::string> TelegramBot::_parseMessage(const std::string &message)
  {
    std::vector<std::string> params;
    size_t start = 0;
    size_t end = message.find(' ');

    while (end != std::string::npos)
    {
        params.push_back(message.substr(start, end - start));
        start = end + 1;
        end = message.find(' ', start);
    }

    // Agregar el último parámetro (si existe)
    if (start < message.length())
    {
        params.push_back(message.substr(start));
    }
    for (int i=0; i<params.size(); ++i) {
      printf("params[%zu] = %s\n", i, params[i].c_str());
    }
    
    return params;
  }

  /**
  * @brief 
  * @note 
  * @param 
  * @return
  */
  void TelegramBot::_sendMessage(const std::string chatId, const std::string message)
  {
    std::string server = botUrl + botToken + "/sendmessage";
    std::string request = "chat_id=" + chatId + "&text=" + message;

    Drivers::WifiCom::getInstance().post(server, request);
  }

  /**
  * @brief 
  * @note 
  * @param 
  * @return
  */
  void TelegramBot::_requestLastMessage()
  {
    std::string server = botUrl + botToken + "/getUpdates";
    std::string request = "offset=-1";
    Drivers::WifiCom::getInstance().post(server, request);
  }

  /**
  * @brief 
  * @note 
  * @param 
  * @return
  */
  bool TelegramBot::_getMessageFromResponse(telegram_Message *message, const std::string &response)
  {
    std::string fixedResponse = response;

    // Si empieza con doble llave, eliminamos una
    if (fixedResponse[0] == '{' && fixedResponse[1] == '{') {
        fixedResponse.erase(0, 1);  // Eliminar el primer '{'
    }

    JsonDocument doc;

    // Parse the JSON payload
    DeserializationError error = deserializeJson(doc, fixedResponse);
    if (error)
    {
      return false;
    }
    
    // Check if "ok" is true
    if (!doc["ok"])
    {
      return false;
    }

    JsonArray results = doc["result"];
    if (results.size() == 0)
    {
      return false;
    }
    
    JsonObject firstResult = results[0];
    unsigned long updateId = firstResult["update_id"] | 0;

    if (botLastUpdateId == 0)
    {
      botLastUpdateId = updateId;
      return false;  // Ignorar primer mensaje recibido siempre
    }

    if (updateId <= botLastUpdateId)
    {
      return false; // Ignorar mensajes repetidos o viejos
    }

    botLastUpdateId = updateId;

    JsonObject messageObj = firstResult["message"];
    JsonObject from = messageObj["from"];

    const char* result_0_message_text = messageObj["text"];
    unsigned long long result_0_message_from_id = from["id"];
    const char* result_0_message_from_first_name = from["first_name"];
    const char* result_0_message_from_username = !from["username"].isNull() ? from["username"] : from["id"].as<const char*>(); //TODO: Esto esta medio al pepe, ver como lo puedo cambiar.

    message->updateId = updateId;
    message->fromId = std::to_string(result_0_message_from_id);
    message->fromName = std::string(result_0_message_from_first_name);
    message->fromUserName = std::string(result_0_message_from_username);
    message->message = std::string(result_0_message_text);


    return true;
  }

  /**
  * @brief 
  * @note 
  * @param 
  * @return
  */
  command_t TelegramBot::_findCommand(const std::string command)
  {
    if (command == "/start") {
      return COMMAND_START;
    } else if (command == "/new_tank") {
      return COMMAND_NEW_TANK;
    } else if (command == "/tank_status") {
      return COMMAND_TANK_STATUS;
    } else if (command == "/new_gas_flow") {
      return COMMAND_COMMAND_NEW_GAS_FLOW;
    } else if (command == "/end") {
      return COMMAND_END;
    }

    return ERROR_INVALID_COMMAND;
  }

  /**
  * @brief Format a string according to the provided format.
  * @note This implementation was obtained from <a href="https://stackoverflow.com/a/49812018">here</a>.
  * @param format The format string.
  * @param ... Additional arguments for formatting.
  * @return The formatted string.
  */
  std::string TelegramBot::_formatString(const char* format, ...)
  {
    va_list args;
    va_start(args,format);

    int size = std::vsnprintf(nullptr, 0, format, args);
    va_end(args);

    std::string result(size + 1, '\0');
    
    va_start(args, format);
    std::vsnprintf(&result[0], result.size(), format, args);
    va_end(args);

    result.pop_back();

    return result;
  }

  /**
  * @brief 
  * @note 
  * @param 
  * @return
  */
  bool TelegramBot::_isStringNumeric(const std::string &str)
  {
    return str.find_first_not_of( "0123456789" ) == string::npos;
  }


} // namespace Module

/**
* @brief 
*/
static void onTBotTimeoutFinishedCallback()
{
  isTimeoutFinished = true;
}

/**
* @brief 
*/
static void onAlertTimeoutFinishedCallback()
{
  isTimeoutFinished = true;
}