/****************************************************************************//**
 * @file telegram_bot.cpp
 * @brief Telegram Bot Module.
 * @author Gonzalo Puy.
 * @date Aug 2024
 *******************************************************************************/

#include "telegram_bot.h"

#include <cstddef>
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
  {
  }

  void TelegramBot::init()
  {
    botLastUpdateId = 0;
    // std::fill(userId.begin(), userId.end(), "");
    userId.fill("");
    userCount = 0;
    broadcastIndex = 0;
    isTimeoutFinished = false;
    isAlertTimeoutFinished = true; //El estado inicial de esta variable DEBE ser true.
    isBroadcastInProgress = false;

    functionsArray[COMMAND_START] = &TelegramBot::_commandStart;
    functionsArray[COMMAND_NEW_TANK] = &TelegramBot::_commandNewTank;
    functionsArray[COMMAND_TANK] = &TelegramBot::_commandTank;
    functionsArray[COMMAND_TANK_STATUS] = &TelegramBot::_commandTankStatus;
    functionsArray[COMMAND_NEW_GAS_FLOW] = &TelegramBot::_commandNewGasFlow;
    functionsArray[COMMAND_GAS_FLOW] =  &TelegramBot::_commandGasFlow;
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
        printf("\n\r[DEBUG] Tank State: %d\n\r", Module::TankMonitor::getInstance().getTankState());
        if ( isAlertTimeoutFinished && (Module::TankMonitor::getInstance().getTankState() == TANK_LEVEL_LOW) ) {

          isAlertTimeoutFinished = false;
          alertTimeout.detach();
          alertTimeout.attach(&onAlertTimeoutFinishedCallback, 60s);
          botState = SEND_ALERT;
          
        } else {
          botState = REQUEST_LAST_MESSAGE;
          isTimeoutFinished = false;
          tBotTimeout.detach();
          tBotTimeout.attach(&onTBotTimeoutFinishedCallback, 2s);
        }
      }
      break;

      case SEND_ALERT:
      {
        if (!Drivers::WifiCom::getInstance().isBusy()) {
          std::string messegeToSend = ALERT_TANK_EMPTY;
          messegeToSend += "\n";
          _sendMessage(userId[broadcastIndex], messegeToSend);
          isBroadcastInProgress = userCount > 1 ? true : false;
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
        size_t paramCount;
        std::array<std::string, MAX_PARAMS> params = _parseMessage(botLastMessage.message, paramCount);
        std::string command = params[0];

        if( (_isUserIdValid(botLastMessage.fromId)) || (command.compare(COMMAND_START_STR) == 0 )) {
          command_t command_nb = _findCommand(command); //Agregar verificacion de comando
          if (command_nb != ERROR_INVALID_COMMAND)
          {
            printf("paramCount: %d\n\r", paramCount);
            messegeToSend = (this->*functionsArray[command_nb])(params, paramCount);
          }else {
            messegeToSend = _formatString(ERROR_INVALID_COMMAND_STR, command.c_str());
          }
        } else {
          messegeToSend = _formatString(ERROR_INVALID_USER_STR, botLastMessage.fromName.c_str());
        }

        _sendMessage(botLastMessage.fromId, messegeToSend);
        isTimeoutFinished = false;
        tBotTimeout.detach();
        tBotTimeout.attach(&onTBotTimeoutFinishedCallback,5s);
        botState = WAITING_RESPONSE;
      }
      break;

      case WAITING_RESPONSE:
      {
        std::string response;
        if (isTimeoutFinished || (Drivers::WifiCom::getInstance().getPostResponse(&response) && botResponse.compare(RESULT_ERROR) == 0 )) {
          if (isBroadcastInProgress)
          {
            // SI falló y hay un broadcast en progreso, reintentar
            isBroadcastInProgress = true;
            botState = SEND_ALERT;
          } else {
            broadcastIndex = 0;
            isBroadcastInProgress = false;
            botState = INIT;
          }
        } else if (Drivers::WifiCom::getInstance().getPostResponse(&response)) {
          if (isBroadcastInProgress){
            broadcastIndex++;
            if (broadcastIndex >= userCount)
            {
              broadcastIndex = 0;
              isBroadcastInProgress = false;
              botState = INIT;
            } else {
              botState = SEND_ALERT;
            }
          }else {
            botState = INIT;
          }
        }
      }
      break;
    }
  }

//=====[Implementations of private functions]===================================

  /**
  * @brief 
  * @param
  * @param 
  * @return
  */
  std::string TelegramBot::_commandStart(const ParametersArray &params, size_t paramCount)
  {
    if (paramCount == 1) {
      std::string result;
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
  * @param 
  * @param 
  * @return
  */
  std::string TelegramBot::_commandNewTank(const ParametersArray &params, size_t paramCount)
  {
    if (paramCount == 1) {
      return _formatString("To register a new tank please use '/tank' command as follows:\
      \n\n/tank type <tank type> gflow <gas flow [L/min]>\
      \n\nOr if you don't know the tank type:\
      \n\n/tank vol <tank volume [L]> gflow <gas flow [L/min]>\
      \n\nExamples:\
      \n/tank type H gflow 2\
      \n/tank vol 30 gflow 3");
    } else {
      return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_NEW_TANK_STR);
    }

    return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_NEW_TANK_STR);
  }

  /**
  * @brief 
  * @param 
  * @param 
  * @return
  */
  std::string TelegramBot::_commandTank(const ParametersArray &params, size_t paramCount)
  {
    if (paramCount == 5) {
      std::string firstParam = params[1];
      std::string tankType = params[2];
      std::string numTankCapacity = params[2];
      std::string numTankGasFlow = params[4];
      
      if (firstParam == "type") {
        
        if (_isStringNumeric(numTankGasFlow)) {
          bool isTypeValid = Module::TankMonitor::getInstance().isTankTypeValid(tankType);

          if (isTypeValid) {
            float tankGasFlow = std::stof(numTankGasFlow);
            int tankCapacity = 0;
            Module::TankMonitor::getInstance().setNewTank(tankType, tankCapacity, tankGasFlow);
            return _formatString("[Success!]\
                   \nNew Oxygen Tank registered:\
                   \nType: %s\
                   \nGas Flow: %d [L/min].\n",
                   tankType.c_str(), tankGasFlow);
          }
        }

      } else if (firstParam == "vol") {
        if (_isStringNumeric(numTankGasFlow) && _isStringNumeric(numTankCapacity)) {
          int tankGasFlow = std::stoi(numTankGasFlow);
          int tankCapacity = std::stoi(numTankCapacity);
          tankType = "None";
          Module::TankMonitor::getInstance().setNewTank(tankType, tankCapacity, tankGasFlow);
          return _formatString("[Success!]\
                 \nNew Oxygen Tank registered:\
                 \nCapacity: %d [L]\
                 \nGas Flow: %d [L/min]",
                 tankCapacity, tankGasFlow);
        }
        
      }

    } else {
      return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_TANK_STR);
    }

    return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_TANK_STR);
  }

  /**
  * @brief 
  * @param
  * @param 
  * @return
  */
  std::string TelegramBot::_commandTankStatus(const ParametersArray &params, size_t paramCount)
  {
    if (paramCount == 1) {
      if (Module::TankMonitor::getInstance().isTankRegistered()) {
        float time = Module::TankMonitor::getInstance().getTankStatus();

        if (time > 60.0) {
          int hours = (int) (time / 60.0);
          float minutesLeft = time - (hours * 60.0);
          int minutes = (int) (minutesLeft + 0.5);
          return _formatString("[Tank Status]\
                 \nThe tank will go low in approximately %d hs. and %d min.",
                 hours, minutes);
        } else if (time < 60.0){
          int timeLeft = (int) time;
          return _formatString("[Tank Status]\
                 \nThe tank will go low in approximately %d min.",
                 timeLeft);
        } else {
          return _formatString("[Tank Status Error]\
                 \nCan't get tank status.\
                 \nPlease try again.");
        }

      } else {
        return _formatString(ERROR_NO_TANK_STR);
      }
      
    } else {
      return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_TANK_STATUS_STR);
    }

    return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_TANK_STATUS_STR);
  }

  /**
  * @brief 
  * @param 
  * @param 
  * @return
  */
  std::string TelegramBot::_commandNewGasFlow(const ParametersArray &params, size_t paramCount)
  {
    if (paramCount == 1) {
      return _formatString("To set a new gas flow value for the current tank please use '/gasflow' command as follows:\
      \n\n/gasflow <gas flow [L/min]>\
      \n\nExample:\
      \n/gasflow 2");
    } else {
      return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_NEW_TANK_STR);
    }

    return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_NEW_TANK_STR);
  }

  /**
  * @brief 
  * @param
  * @param 
  * @return
  */
  std::string TelegramBot::_commandGasFlow(const ParametersArray &params, size_t paramCount)
  {
    if (paramCount == 2) {
      if (Module::TankMonitor::getInstance().isTankRegistered()) {
        std::string numTankGasFlow = params[1];

        if (_isStringNumeric(numTankGasFlow)) {
          float tankGasFlow = std::stof(numTankGasFlow);
          Module::TankMonitor::getInstance().setNewGasFlow(tankGasFlow);
          return _formatString("[Success!]\
                 \nNew gas flow seted up with the value: %d [L/min]",
                 tankGasFlow);

        }  
      } else {
        return _formatString(ERROR_NO_TANK_STR);
      }
    } else {
      return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_NEW_GAS_FLOW_STR);
    }

    return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_NEW_GAS_FLOW_STR);
  }

  /**
  * @brief 
  * @param params
  * @param paramCount
  * @return
  */
  std::string TelegramBot::_commandEnd(const ParametersArray &params, size_t paramCount)
  {
    if (paramCount == 1) {
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

    return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_END_STR);
  }

  /**
  * @brief 
  * @param 
  * @return
  */
  bool TelegramBot::_registerUser(std::string newUserId)
  {
    bool userFound = _isUserIdValid(newUserId);
    printf("userFound: %d\r\n", userFound);

    if (!userFound)
    {
      if (userCount == MAX_USER_COUNT){
        return false;
      }
      //userId.insert(userId.begin(), newUserId);  // Insert at beginning
      userId[userCount] = newUserId;
      userCount++;
      return true;
    }
    return false;
  }

  /**
  * @brief 
  * @param 
  * @return
  */
  bool TelegramBot::_unregisterUser(std::string oldUserId)
  {
    std::array<std::string, MAX_USER_COUNT>::iterator it = std::find(userId.begin(), userId.end(), oldUserId);
    std::array<std::string, MAX_USER_COUNT>::iterator end = userId.begin() + userCount;

    if (it != userId.end()) {
      std::array<std::string, MAX_USER_COUNT>::iterator next;

      for (next = it + 1; next != end; ++it, ++next)
      {
          *it = *next;
      }

      userCount--;
      userId[userCount] = "";  // Limpiar el último valor válido
      return true;
    } else {
      return false;
    }
    
  }

  /**
  * @brief 
  * @param 
  * @return
  */
  bool TelegramBot::_isUserIdValid(std::string fUserId)
  {
    std::array<std::string, MAX_USER_COUNT>::iterator it = std::find(userId.begin(), userId.begin() + userCount, fUserId);

    if (it != userId.begin() + userCount)
    {
      return true;
    }

    return false;
  }

  /**
  * @brief 
  * @param 
  * @return
  */
  std::string TelegramBot::_getUserId(std::string user) //TODO: Esta funcion esta al pedo?
  {
    std::array<std::string, MAX_USER_COUNT>::iterator it = std::find(userId.begin(), userId.begin() + userCount, user);

    if (it != userId.begin() + userCount)
    {
      return *it;
    }

    return "User not found";
  }

  /**
  * @brief 
  * @param 
  * @param
  * @return
  */
  Module::TelegramBot::ParametersArray TelegramBot::_parseMessage(const std::string &message, size_t &paramCount)
  {
    ParametersArray params;
    paramCount = 0;

    size_t start = 0;
    size_t end = message.find(' ');

    while (end != std::string::npos && paramCount < MAX_PARAMS)
    {
        params[paramCount++] = message.substr(start, end - start);
        start = end + 1;
        end = message.find(' ', start);
    }

    // Agregar el último parámetro (si existe y hay espacio)
    if (start < message.length() && paramCount < MAX_PARAMS)
    {
        params[paramCount++] = message.substr(start);
    }

    //For Debug:
    for (size_t i = 0; i < paramCount; ++i)
    {
        printf("params[%zu] = %s\n", i, params[i].c_str());
    }

    return params;
  }

  /**
  * @brief 
  * @param 
  * @param 
  */
  void TelegramBot::_sendMessage(const std::string chatId, const std::string message)
  {
    std::string server = botUrl + botToken + "/sendmessage";
    std::string request = "chat_id=" + chatId + "&text=" + message;

    Drivers::WifiCom::getInstance().post(server, request);
  }

  /**
  * @brief 
  */
  void TelegramBot::_requestLastMessage()
  {
    std::string server = botUrl + botToken + "/getUpdates";
    std::string request = "offset=-1";
    Drivers::WifiCom::getInstance().post(server, request);
  }

  /**
  * @brief 
  * @param
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
  * @param 
  * @return
  */
  command_t TelegramBot::_findCommand(const std::string command)
  {
    if (command == COMMAND_START_STR) {
      return COMMAND_START;
    } else if (command == COMMAND_NEW_TANK_STR) {
      return COMMAND_NEW_TANK;
    } else if (command == COMMAND_TANK_STR) {
      return COMMAND_TANK;
    } else if (command == COMMAND_TANK_STATUS_STR) {
      return COMMAND_TANK_STATUS;
    } else if (command == COMMAND_NEW_GAS_FLOW_STR) {
      return COMMAND_NEW_GAS_FLOW;
    } else if (command == COMMAND_GAS_FLOW_STR) {
      return COMMAND_GAS_FLOW;
    } else if (command == COMMAND_END_STR) {
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
  * @param 
  * @return
  */
  bool TelegramBot::_isStringNumeric(const std::string &str)
  {
    if (str.empty()) return false;

    bool dot_found = false;
    bool digit_found = false;

    size_t i = 0;

    for (; i < str.size(); ++i) {
      char c = str[i];
      if (isdigit(c)) {
        digit_found = true;
      } else if (c == '.') {
          if (dot_found) return false;  // more than 1 '.' is not valid
          dot_found = true;
      } else {
          return false; // invalid character
      }
    }
    return digit_found;
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
  isAlertTimeoutFinished = true;
}

  // Backup de funcion _parseMessage con Vector
  // std::vector<std::string> TelegramBot::_parseMessage(const std::string &message)
  // {
  //   std::vector<std::string> params;
  //   size_t start = 0;
  //   size_t end = message.find(' ');

  //   while (end != std::string::npos)
  //   {
  //       params.push_back(message.substr(start, end - start));
  //       start = end + 1;
  //       end = message.find(' ', start);
  //   }

  //   // Agregar el último parámetro (si existe)
  //   if (start < message.length())
  //   {
  //       params.push_back(message.substr(start));
  //   }
  //   for (int i=0; i<params.size(); ++i) {
  //     printf("params[%zu] = %s\n", i, params[i].c_str());
  //   }
    
  //   return params;
  // }