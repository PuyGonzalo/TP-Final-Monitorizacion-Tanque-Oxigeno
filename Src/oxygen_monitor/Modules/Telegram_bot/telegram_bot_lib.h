/******************************************************************************//**
 * @file telegram_bot_lib.h
 * @author Gonzalo Puy.
 * @date Aug 2024
 * @brief Common constants and enumerations used by the Telegram Bot module.
 *
 * This file defines command identifiers, associated command strings, and common
 * error or alert messages used throughout the Telegram Bot system.
 *******************************************************************************/

#ifndef TELEGRAM_BOT_LIB_H
#define TELEGRAM_BOT_LIB_H

/**
 * @enum command_t
 * @brief Enumerates all supported commands for the Telegram Bot.
 */
typedef enum COMMANDS {
    COMMAND_START,                  /**< Command to start bot interaction and register new user(/start). */
    COMMAND_NEW_TANK,               /**< Command to register a new tank (/newTank). */
    COMMAND_TANK,                   /**< Command to set new tank information (/tank). */
    COMMAND_TANK_STATUS,            /**< Command to query the tank's current status (/status). */
    COMMAND_NEW_GAS_FLOW,           /**< Command to configure new gas flow (/newgf). */
    COMMAND_GAS_FLOW,               /**< Command to set new gas flow (/gasflow). */
    COMMAND_END,                    /**< Command to unregister user (/end). */
    ERROR_INVALID_COMMAND           /**< Returned when a command is not recognized. */
} command_t;

/**
 * @brief Command string for starting the bot session.
 */
const char COMMAND_START_STR[]              = "/start";

/**
 * @brief Command string for registering a new tank.
 */
const char COMMAND_NEW_TANK_STR[]           = "/newtank";

/**
 * @brief Command string for registering a new tank.
 */
const char COMMAND_TANK_STR[]           = "/tank";

/**
 * @brief Command string for checking tank status.
 */
const char COMMAND_TANK_STATUS_STR[]        = "/status";

/**
 * @brief Command string for setting new gas flow.
 */
const char COMMAND_NEW_GAS_FLOW_STR[]       = "/newgf";

/**
 * @brief Command string for setting new gas flow.
 */
const char COMMAND_GAS_FLOW_STR[]       = "/gasflow";

/**
 * @brief Command string for setting new gas flow.
 */
const char COMMAND_END_STR[]       = "/end";

/**
 * @brief Error message for invalid commands.
 */
const char ERROR_INVALID_COMMAND_STR[]      = "[ERROR]\nInvalid command [%s].";

/**
 * @brief Error message for unregistered or unauthorized users.
 */
const char ERROR_INVALID_USER_STR[]         = "[ERROR]\nInvalid user [%s].";

/**
 * @brief Error message for invalid or missing parameters.
 */
const char ERROR_INVALID_PARAMETERS_STR[]   = "[ERROR]\nInvalid parameters for  [%s] command.";

/**
 * @brief Error message for invalid tank IDs.
 */
const char ERROR_NO_TANK_STR[]      = "[Error]\nThere is no tank regitered yet.\nPlease use '/newTank' command first.";

/**
 * @brief Alert message indicating the tank is low or empty.
 */
const char ALERT_TANK_EMPTY[]               = "[ALERT]\nTank is low!";

#endif // TELEGRAM_BOT_LIB_H