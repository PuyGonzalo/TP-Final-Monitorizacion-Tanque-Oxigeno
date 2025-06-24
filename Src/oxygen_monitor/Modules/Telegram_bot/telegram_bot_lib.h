/******************************************************************************//**
 * @file telegram_bot_lib.h
 * @author Gonzalo Puy.
 * @date Aug 2024
 * @brief Telegram Bot Lib.
 *
 *******************************************************************************/

#ifndef TELEGRAM_BOT_LIB_H
#define TELEGRAM_BOT_LIB_H

typedef enum COMMANDS {
    COMMAND_START,
    COMMAND_NEW_TANK,
    COMMAND_TANK_STATUS,
    COMMAND_COMMAND_NEW_GAS_FLOW,
    COMMAND_END,
    ERROR_INVALID_COMMAND
} command_t;

const char COMMAND_START_STR[]              = "/start";
const char COMMAND_NEW_TANK_STR[]           = "/newTank";
const char COMMAND_TANK_STATUS_STR[]        = "/status";
const char COMMAND_NEW_GAS_FLOW_STR[]       = "/newGFlow";

const char ERROR_INVALID_COMMAND_STR[]      = "ERROR\nInvalid command [%s].";
const char ERROR_INVALID_USER_STR[]         = "ERROR\nInvalid user [%s].";
const char ERROR_INVALID_PARAMETERS_STR[]   = "ERROR\nInvalid parameters for  [%s] command.";
const char ERROR_INVALID_TANK_ID_STR[]      = "ERROR\nInvalid Tank ID";

const char ALERT_TANK_EMPTY[]               = "[ALERT]\nTank is low!";

#endif // TELEGRAM_BOT_LIB_H