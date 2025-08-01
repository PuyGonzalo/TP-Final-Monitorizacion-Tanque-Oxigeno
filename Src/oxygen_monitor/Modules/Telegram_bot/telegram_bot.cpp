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
#include <ArduinoJson.h>

//=====[Declaration and initialization of private global variables]============

static Timeout tBotTimeout;                         /**<  */
static bool isTimeoutFinished;                      /**<  */

static Timeout alertTimeout;                        /**<  */
static bool isAlertTimeoutFinished;                 /**<  */

static constexpr chrono::seconds alertDelay = 60s;  /**<  */

/**
* @brief 
*/
static void onTBotTimeoutFinishedCallback();

/**
* @brief 
*/
static void onAlertTimeoutFinishedCallback();

//=====[Implementations of public functions]===================================

namespace Module {

  void TelegramBot::init()
  {
    getInstance()._init();
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
          botState = userCount > 1 ? WAITING_BROADCAST_RESPONSE : WAITING_RESPONSE;
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
          command_t command_nb = _findCommand(command);
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
        tBotTimeout.attach(&onTBotTimeoutFinishedCallback, 5s);
        botState = WAITING_RESPONSE;
      }
      break;

      case WAITING_RESPONSE:
      {
        const bool isResponseReady = Drivers::WifiCom::getInstance().getPostResponse(&botResponse);
        if (isTimeoutFinished || ( isResponseReady && botResponse.compare(RESULT_ERROR) == 0 ))
        {
          botState = INIT;
        }
        else if (Drivers::WifiCom::getInstance().getPostResponse(&botResponse))
        {
          botState = INIT;
        }
      }
      break;

      case WAITING_BROADCAST_RESPONSE:
      {
        const bool isResponseReady = Drivers::WifiCom::getInstance().getPostResponse(&botResponse);
        if (isTimeoutFinished || ( isResponseReady && botResponse.compare(RESULT_ERROR) == 0 ))
        {
          if (broadcastRetryCount < BROADCAST_MAX_RETRIES)
          {
            broadcastRetryCount++;
            tBotTimeout.detach();
            tBotTimeout.attach(&onTBotTimeoutFinishedCallback, 8s);
            botState = SEND_ALERT;
          } else {
            broadcastIndex = 0;
            broadcastRetryCount = 0;
            botState = INIT;
          }
        } else if (Drivers::WifiCom::getInstance().getPostResponse(&botResponse)) {
          broadcastIndex++;
          if (broadcastIndex >= userCount)
          {
            broadcastIndex = 0;
            broadcastRetryCount = 0;
            botState = INIT;
          } else {
            broadcastRetryCount = 0;
            botState = SEND_ALERT;
          }
        }
      }
      break;
    }
  } // TelegramBot::update()

//=====[Implementations of private functions]===================================

  void TelegramBot::_init()
  {
    botLastUpdateId = 0;
    userId.fill("");
    userCount = 0;
    broadcastIndex = 0;
    isTimeoutFinished = false;
    isAlertTimeoutFinished = true; //Initial state of this variable MUST be true.
    broadcastRetryCount = 0;

    functionsArray[COMMAND_START] = &TelegramBot::_commandStart;
    functionsArray[COMMAND_SET_UNIT] = &TelegramBot::_commandSetUnit;
    functionsArray[COMMAND_UNIT] = &TelegramBot::_commandUnit;
    functionsArray[COMMAND_NEW_TANK] = &TelegramBot::_commandNewTank;
    functionsArray[COMMAND_TANK] = &TelegramBot::_commandTank;
    functionsArray[COMMAND_TANK_STATUS] = &TelegramBot::_commandTankStatus;
    functionsArray[COMMAND_NEW_GAS_FLOW] = &TelegramBot::_commandNewGasFlow;
    functionsArray[COMMAND_GAS_FLOW] =  &TelegramBot::_commandGasFlow;
    functionsArray[COMMAND_END] = &TelegramBot::_commandEnd;
  }

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
        result = _formatString(START_COMMAND_USER_REGISTERED_RESPONSE_STR, botLastMessage.fromName.c_str());
        // Debug:
        for (int i = 0; i < userCount; ++i) { printf("Success! users ID:[ %s ]\r\n", userId[i].c_str()); }
      } else {
        result = _formatString(START_COMMAND_USER_ALREADY_REGISTERED_RESPONSE_STR, botLastMessage.fromName.c_str());
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
  std::string TelegramBot::_commandSetUnit(const ParametersArray &params, size_t paramCount)
  {
    if (paramCount == 2) {
      std::string unit = params[1];

      if(Module::TankMonitor::getInstance().setPressureGaugeUnit(unit))
      {
        return SET_UNIT_COMMAND_RESPONSE_STR;
      } 

    }

    return _formatString(ERROR_INVALID_PARAMETERS_STR, COMMAND_START_STR);
  }

  /**
  * @brief 
  * @param
  * @param 
  * @return
  */
  std::string TelegramBot::_commandUnit(const ParametersArray &params, size_t paramCount)
  {
    if (paramCount == 1) {
      std::string result;
      
      std::string unit = Module::TankMonitor::getInstance().getPressureGaugeUnitStr();
      if (unit != "Unknown")
      {
        result = _formatString(UNIT_COMMAND_RESPONSE_STR, unit.c_str());
      } else {
        result = _formatString(UNIT_COMMAND_RESPONSE_STR, "unit not set");
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
      return NEW_TANK_COMMAND_RESPONSE_STR;
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
      
      if (!Module::TankMonitor::getInstance().isUnitSet())
      {
        return TANK_COMMAND_NO_UNIT_RESPONSE;
      }

      std::string unit = Module::TankMonitor::getInstance().getPressureGaugeUnitStr();

      if (firstParam == "type") {

        if (_isStringNumeric(numTankGasFlow)) {
          bool isTypeValid = Module::TankMonitor::getInstance().isTankTypeValid(tankType);

          if (isTypeValid) {
            float tankGasFlow = std::stof(numTankGasFlow);
            int tankCapacity = 0;
            Module::TankMonitor::getInstance().setNewTank(tankType, tankCapacity, tankGasFlow);
            return _formatString(TANK_COMMAND_TYPE_RESPONSE_STR, tankType.c_str(), tankGasFlow);
          }
        }

      } else if (firstParam == "vol") {
        if (unit != "BAR") return COMMAND_TANK_UNIT_ERROR;

        if (_isStringNumeric(numTankGasFlow) && _isStringNumeric(numTankCapacity)) {
          int tankGasFlow = std::stof(numTankGasFlow);
          int tankCapacity = std::stoi(numTankCapacity);
          tankType = "None";
          Module::TankMonitor::getInstance().setNewTank(tankType, tankCapacity, tankGasFlow);
          return _formatString(TANK_COMMAND_VOL_RESPONSE_STR, tankCapacity, tankGasFlow);
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
          return _formatString(STATUS_COMMAND_RESPONSE_HOURS_STR, hours, minutes);
        } else if (time < 60.0){
          int timeLeft = (int) time;
          return _formatString(STATUS_COMMAND_RESPONSE_MINUTES_STR, timeLeft);
        } else if (time == -1){
          return ERROR_STATUS_COMMAND_STR;
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
      return NEW_GAS_FLOW_COMMAND_RESPONSE_STR;
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
          return _formatString(GAS_FLOW_COMMAND_RESPONSE_STR, tankGasFlow);

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
        result = _formatString(END_COMMAND_USR_REMOVED_RESPONSE_STR, botLastMessage.fromName.c_str());
      } else {
        result = _formatString(END_COMMAND_USR_NOTFOUND_RESPONSE_STR, botLastMessage.fromName.c_str());
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
      userId[userCount] = "";
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
    UsersArray::iterator it = std::find(userId.begin(), userId.begin() + userCount, fUserId);

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
    UsersArray::iterator it = std::find(userId.begin(), userId.begin() + userCount, user);

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

    // Erase first '{'
    // Don't know why but sometimes JSON's comes with an extra '{' at the start
    // that it's not supposed to be there (Everything on th ESP32 side seems OK).
    if (fixedResponse[0] == '{' && fixedResponse[1] == '{') {
        fixedResponse.erase(0, 1);
    }

    JsonDocument doc;

    // Parse the JSON payload
    DeserializationError error = deserializeJson(doc, fixedResponse);
    if (error)
    {
      return false;
    }
    
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
      return false;
    }

    if (updateId <= botLastUpdateId)
    {
      return false;
    }

    botLastUpdateId = updateId;

    JsonObject messageObj = firstResult["message"];
    JsonObject from = messageObj["from"];

    const char* result_0_message_text = messageObj["text"];
    unsigned long long result_0_message_from_id = from["id"];
    const char* result_0_message_from_first_name = from["first_name"];
    const char* result_0_message_from_username = !from["username"].isNull() ? from["username"] : from["id"].as<const char*>();

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
    } else if (command == COMMAND_UNIT_STR) {
      return COMMAND_UNIT;
    } else if (command == COMMAND_SET_UNIT_STR) {
      return COMMAND_SET_UNIT;
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

static void onTBotTimeoutFinishedCallback()
{
  isTimeoutFinished = true;
}

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