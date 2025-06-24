/******************************************************************************//**
 * @file telegram_bot.h
 * @author Gonzalo Puy.
 * @date Aug 2024
 * @brief Telegram Bot Module header file.
 *
 *******************************************************************************/

#ifndef TELEGRAM_BOT_H
#define TELEGRAM_BOT_H

#include <map>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include <stdint.h>
#include "telegram_bot_lib.h"
#include "PinNames.h"
#include "delay.h"
#include "mbed.h"
#include "tank_monitor.h"

#define NB_COMMANDS 5

#define BOT_API_URL "https://api.telegram.org/bot"
#define BOT_TOKEN   "7713584244:AAGMZfNYBwRIWm1gPhduFv5bhBhdRNhkBcA"
#define USER_ID     "GP96_testBot"
#define MAX_USER_COUNT 10
#define MAX_ALERT_LIMIT 10

//typedef void (*botFunctionCallback)();

namespace Module {

  class TelegramBot {

    public:

      TelegramBot(const char *apiUrl, const char *token);

      ~TelegramBot() = default;

      void init();

      void update();

      //void send_alert(); Funcion que simplemente ponga botState = SEND_ALERT, llame a update(); y setee un timer del RTC para reenviar la alerta despues de x minutos

    private:

      typedef std::string (TelegramBot::*commandFunction)(const std::vector<std::string> &params);

      struct telegram_Message {
        unsigned long updateId;
        std::string fromId;
        std::string fromUserName;
        std::string fromName;
        std::string message;
      };

      typedef enum BOT_STATE {
        INIT,
        MONITOR,
        SEND_ALERT,
        REQUEST_LAST_MESSAGE,
        WAITING_LAST_MESSAGE,
        PROCESS_LAST_MESSAGE,
        WAITING_RESPONSE,
      } bot_state_t;

      std::string _commandStart(const std::vector<std::string> &params);
      std::string _commandNewTank(const std::vector<std::string> &params);
      std::string _commandTankStatus(const std::vector<std::string> &params);
      std::string _commandNewGasFlow(const std::vector<std::string> &params);
      std::string _commandEnd(const std::vector<std::string> &params);

      bool _registerUser(std::string userId);

      bool _unregisterUser(std::string oldUserId);

      bool _isUserIdValid(std::string tankId);

      std::string _getUserId();

      std::vector<std::string> _parseMessage(const std::string &message);

      void _sendMessage(const std::string chatId, const std::string message);

      void _requestLastMessage();

      bool _getMessageFromResponse(telegram_Message *message, const std::string &response);

      command_t _findCommand(const std::string command);

      /**
      * @brief Format a string according to the provided format.
      * @note This implementation was obtained from <a href="https://stackoverflow.com/a/49812018">here</a>.
      * @param format The format string.
      * @param ... Additional arguments for formatting.
      * @return The formatted string.
      */
      std::string _formatString(const char* format, ... );

      bool _isStringNumeric(const std::string &str);

      bot_state_t botState;
      const std::string botToken;
      const std::string botUrl;
      unsigned long botLastUpdateId;
      std::vector<std::string> userId;
      int userCount;
      telegram_Message botLastMessage;
      std::string botResponse;
      commandFunction functionsArray[NB_COMMANDS];
      Util::Delay         botDelay;

  }; //TelegramBot class

} // namespace Module

#endif //TELEGRAM_BOT_H