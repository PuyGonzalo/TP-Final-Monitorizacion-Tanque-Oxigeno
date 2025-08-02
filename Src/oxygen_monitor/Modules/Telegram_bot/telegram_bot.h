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

#include <cstddef>
#include <map>
#include <string.h>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <stdint.h>
#include "telegram_bot_lib.h"
#include "PinNames.h"
#include "delay.h"
#include "mbed.h"
#include "tank_monitor.h"

//=========================[Module Defines]=====================================

#define NB_COMMANDS 9
#define BOT_API_URL "https://api.telegram.org/bot"
#define BOT_TOKEN   "7713584244:AAGMZfNYBwRIWm1gPhduFv5bhBhdRNhkBcA"
#define MAX_USER_COUNT 10
#define MAX_PARAMS 10
#define BROADCAST_MAX_RETRIES 3

namespace Module {

  class TelegramBot {

    public:

      /**
      * @brief
      */
      static TelegramBot& getInstance(){
        static TelegramBot instance(BOT_API_URL, BOT_TOKEN);

        return instance;
      }

      TelegramBot(const TelegramBot&) = delete;
      TelegramBot& operator=(const TelegramBot&) = delete;

      /**
      * @brief
      */
      static void init();

      /**
      * @brief
      */
      void update();

    private:

      using ParametersArray = std::array<std::string, MAX_PARAMS>;  /**< Aliasing used for array class. */
      using UsersArray = std::array<std::string, MAX_USER_COUNT>;   /**< Aliasing used for array class. */

      /**
      * @brief Type for command function pointers.
      */
      typedef std::string (TelegramBot::*commandFunction)(const ParametersArray &params, const size_t paramCount);

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
        INIT,                       /**< Initial state. */
        MONITOR,                    /**< Monitoring state. */
        SEND_ALERT,                 /**< State to send alerts. */
        REQUEST_LAST_MESSAGE,       /**< State to request last message. */
        WAITING_LAST_MESSAGE,       /**< Waiting for last message. */
        PROCESS_LAST_MESSAGE,       /**< Processing the received message. */
        WAITING_RESPONSE,           /**< Waiting for API response. */
        WAITING_BROADCAST_RESPONSE  /**< Waiting for API response in case of broadcast. */
      } bot_state_t;

      TelegramBot(const char *apiUrl, const char *token)
      : botUrl(apiUrl)
      , botToken(token)
      {};

      ~TelegramBot() = default;

      void _init();

      /**
      * @name Comands Handlers
      * Handler for telegram commands
      * @{
      */
      std::string _commandStart(const ParametersArray &params, size_t paramCount);
      std::string _commandSetUnit(const ParametersArray &params, size_t paramCount);
      std::string _commandUnit(const ParametersArray &params, size_t paramCount);
      std::string _commandNewTank(const ParametersArray &params, size_t paramCount);
      std::string _commandTank(const ParametersArray &params, size_t paramCount);
      std::string _commandTankStatus(const ParametersArray &params, size_t paramCount);
      std::string _commandNewGasFlow(const ParametersArray &params, size_t paramCount);
      std::string _commandGasFlow(const ParametersArray &params, size_t paramCount);
      std::string _commandEnd(const ParametersArray &params, size_t paramCount);
      /** @} */

      bool _registerUser(std::string userId);
      bool _unregisterUser(std::string oldUserId);
      bool _isUserIdValid(std::string tankId);
      std::string _getUserId(std::string user);
      ParametersArray _parseMessage(const std::string &message, size_t &paramCount);
      void _sendMessage(const std::string chatId, const std::string message);
      void _requestLastMessage();
      bool _getMessageFromResponse(telegram_Message *message, const std::string &response);
      command_t _findCommand(const std::string command);
      std::string _formatString(const char* format, ... );
      bool _isStringNumeric(const std::string &str);

      bot_state_t botState;                         /**< Current bot state. */
      const std::string botToken;                   /**< Bot API token. */
      const std::string botUrl;                     /**< Bot API URL. */
      unsigned long botLastUpdateId;                /**< ID of the last processed update. */
      UsersArray userId;                            /**< List of registered user IDs. */
      int userCount;                                /**< Number of registered users. */
      int broadcastIndex;                           /**<  */
      int broadcastRetryCount;                      /**<  */
      telegram_Message botLastMessage;              /**< Last received message. */
      std::string botResponse;                      /**< Last response from API. */
      commandFunction functionsArray[NB_COMMANDS];  /**< Commands function array. */

  }; //TelegramBot class

} // namespace Module

#endif //TELEGRAM_BOT_H