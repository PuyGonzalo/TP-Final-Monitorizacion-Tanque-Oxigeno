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
    COMMAND_START,                  /**< Command to start bot interaction (/start). */
    COMMAND_NEW_TANK,               /**< Command to register a new tank (/newTank). */
    COMMAND_TANK_STATUS,            /**< Command to query the tank's current status (/status). */
    COMMAND_COMMAND_NEW_GAS_FLOW,   /**< Command to configure new gas flow (/newGFlow). */
    COMMAND_END,                    /**< TODO agregar */
    ERROR_INVALID_COMMAND           /**< Returned when a command is not recognized. */
} command_t;

/**
 * @brief Command string for starting the bot session.
 */
const char COMMAND_START_STR[]              = "/start";

/**
 * @brief Command string for registering a new tank.
 */
const char COMMAND_NEW_TANK_STR[]           = "/newTank";

/**
 * @brief Command string for checking tank status.
 */
const char COMMAND_TANK_STATUS_STR[]        = "/status";

/**
 * @brief Command string for setting new gas flow.
 */
const char COMMAND_NEW_GAS_FLOW_STR[]       = "/newGFlow";

/**
 * @brief Error message for invalid commands.
 */
const char ERROR_INVALID_COMMAND_STR[]      = "ERROR\nInvalid command [%s].";

/**
 * @brief Error message for unregistered or unauthorized users.
 */
const char ERROR_INVALID_USER_STR[]         = "ERROR\nInvalid user [%s].";

/**
 * @brief Error message for invalid or missing parameters.
 */
const char ERROR_INVALID_PARAMETERS_STR[]   = "ERROR\nInvalid parameters for  [%s] command.";

/**
 * @brief Error message for invalid tank IDs.
 */
const char ERROR_INVALID_TANK_ID_STR[]      = "ERROR\nInvalid Tank ID";

/**
 * @brief Alert message indicating the tank is low or empty.
 */
const char ALERT_TANK_EMPTY[]               = "[ALERT]\nTank is low!";

#endif // TELEGRAM_BOT_LIB_H