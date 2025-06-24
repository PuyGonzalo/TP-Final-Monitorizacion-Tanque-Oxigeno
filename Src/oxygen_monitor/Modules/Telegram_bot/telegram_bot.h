/******************************************************************************//**
 * @file telegram_bot.h
 * @author Gonzalo Puy.
 * @date Aug 2024
 * @brief Telegram Bot Module header file.
 *
 * This file defines the TelegramBot class used to interact with the Telegram Bot API.
 * It handles sending/receiving messages, managing users, and processing commands.
 ******************************************************************************/

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

namespace Module {

  class TelegramBot {

    public:

      /**
      * @brief
      * @note
      * @param
      * @return
      */
      TelegramBot(const char *apiUrl, const char *token);

      /**
      * @brief
      * @note
      * @param
      * @return
      */
      ~TelegramBot() = default;

      /**
      * @brief
      * @note
      * @param
      * @return
      */
      void init();

      /**
      * @brief
      * @note
      * @param
      * @return
      */
      void update();

    private:

      /**
      * @brief Type for command function pointers.
      */
      typedef std::string (TelegramBot::*commandFunction)(const std::vector<std::string> &params);

      /**
       * @struct telegram_Message
       * @brief Represents a message received from Telegram.
       */
      struct telegram_Message {
        unsigned long updateId;     /**< The update ID from Telegram. */
        std::string fromId;         /**< The sender's Telegram user ID. */
        std::string fromUserName;   /**< The sender's username. */
        std::string fromName;       /**< The sender's first name. */
        std::string message;        /**< The message text. */
      };

      /**
       * @enum BOT_STATE
       * @brief Possible states of the bot's internal state machine.
       */
      typedef enum BOT_STATE {
        INIT,                   /**< Initial state. */
        MONITOR,                /**< Monitoring state. */
        SEND_ALERT,             /**< State to send alerts. */
        REQUEST_LAST_MESSAGE,   /**< State to request last message. */
        WAITING_LAST_MESSAGE,   /**< Waiting for last message. */
        PROCESS_LAST_MESSAGE,   /**< Processing the received message. */
        WAITING_RESPONSE,       /**< Waiting for API response. */
      } bot_state_t;

      // Command Handlers
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
      std::string _formatString(const char* format, ... );
      bool _isStringNumeric(const std::string &str);

      bot_state_t botState;                           /**< Current bot state. */
      const std::string botToken;                     /**< Bot API token. */
      const std::string botUrl;                       /**< Bot API URL. */
      unsigned long botLastUpdateId;                  /**< ID of the last processed update. */
      std::vector<std::string> userId;                /**< List of registered user IDs. */
      int userCount;                                  /**< Number of registered users. */
      telegram_Message botLastMessage;                /**< Last received message. */
      std::string botResponse;                        /**< Last response from API. */
      commandFunction functionsArray[NB_COMMANDS];    /**< Command dispatch table. */
      Util::Delay botDelay;                           /**< Delay utility for timing. */

  }; //TelegramBot class

} // namespace Module

#endif //TELEGRAM_BOT_H