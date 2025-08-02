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
    COMMAND_SET_UNIT,               /**< Command to register a new unit (/setunit). */
    COMMAND_UNIT,                   /**< Command to set or check unit (/unit). */
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
 * @brief Command string for starting the bot session.
 */
const char COMMAND_SET_UNIT_STR[]           = "/setunit";

/**
 * @brief Command string for starting the bot session.
 */
const char COMMAND_UNIT_STR[]               = "/unit";

/**
 * @brief Command string for registering a new tank.
 */
const char COMMAND_NEW_TANK_STR[]           = "/newtank";

/**
 * @brief Command string for seting up a new tank.
 */
const char COMMAND_TANK_STR[]               = "/tank";

/**
 * @brief Command string for checking tank status.
 */
const char COMMAND_TANK_STATUS_STR[]        = "/status";

/**
 * @brief Command string for new gas flow.
 */
const char COMMAND_NEW_GAS_FLOW_STR[]       = "/newgf";

/**
 * @brief Command string for setting the new gas flow.
 */
const char COMMAND_GAS_FLOW_STR[]           = "/gasflow";

/**
 * @brief Command string for ending the bot session.
 */
const char COMMAND_END_STR[]                = "/end";

/**
 * @brief 
 */
const char NEW_TANK_COMMAND_RESPONSE_STR[]                        = "To register a new tank please use '/tank' command as follows:\
                                                                  \n\n/tank type <tank type> gflow <gas flow [L/min]>\
                                                                  \n\nOr if you don't know the tank type:\
                                                                  \n(Warning: this will only work if you seted the measure unit to bar)\
                                                                  \n\n/tank vol <tank volume [L]> gflow <gas flow [L/min]>\
                                                                  \n\nExamples:\
                                                                  \n/tank type H gflow 1.25\
                                                                  \n/tank vol 50 gflow 2";

/**
 * @brief 
 */
const char UNIT_COMMAND_RESPONSE_STR[]                            = "The current unit is: %s\
                                                                  \n\nTo set the unit please use '/setunit' command as follows:\
                                                                  \n\n/setunit <unit>\
                                                                  \n\nExamples:\
                                                                  \n/unit PSI\
                                                                  \n/unit BAR";

/**
 * @brief 
 */
const char SET_UNIT_COMMAND_RESPONSE_STR[]                        = "[Success!]\
                                                                    \n\nCorrectly set new unit.";

/**
 * @brief 
 */
const char START_COMMAND_USER_REGISTERED_RESPONSE_STR[]           = "User registered correctly\nHello %s!";

/**
 * @brief 
 */
const char START_COMMAND_USER_ALREADY_REGISTERED_RESPONSE_STR[]   = "User '%s' is already registered!";

/**
 * @brief 
 */
const char NEW_GAS_FLOW_COMMAND_RESPONSE_STR[]                    = "To set a new gas flow value for the current tank please use '/gasflow' command as follows:\
                                                                    \n\n/gasflow <gas flow [L/min]>\
                                                                    \n\nExample:\
                                                                    \n/gasflow 2";

/**
 * @brief 
 */
const char GAS_FLOW_COMMAND_RESPONSE_STR[]                        = "[Success!]\
                                                                    \nNew gas flow seted up with the value: %d [L/min]";

/**
 * @brief 
 */
const char STATUS_COMMAND_RESPONSE_HOURS_STR[]                    = "[Tank Status]\
                                                                    \nThe tank will go low in approximately %d hs. and %d min.";

/**
 * @brief 
 */
const char STATUS_COMMAND_RESPONSE_MINUTES_STR[]                  = "[Tank Status]\
                                                                    \nThe tank will go low in approximately %d min.";

const char TANK_COMMAND_NO_UNIT_RESPONSE[]                        = "[Warning!]\
                                                                    \n\nNo measure unit is set for the system.\
                                                                    \nPlease use /unit command to know how to set the measure unit.";

/**
 * @brief 
 */
const char TANK_COMMAND_TYPE_RESPONSE_STR[]                       = "[Success!]\
                                                                    \nNew Oxygen Tank registered:\
                                                                    \nType: %s\
                                                                    \nGas Flow: %d [L/min].\n";

/**
 * @brief 
 */
const char TANK_COMMAND_VOL_RESPONSE_STR[]                        = "[Success!]\
                                                                    \nNew Oxygen Tank registered:\
                                                                    \nCapacity: %d [L]\
                                                                    \nGas Flow: %d [L/min]";

/**
 * @brief 
 */
const char COMMAND_TANK_UNIT_ERROR[]                              = "[ERROR]\
                                                                    \n\nParameters used are not valid with the current unit.";

/**
 * @brief 
 */
const char END_COMMAND_USR_REMOVED_RESPONSE_STR[]                 = "User removed correctly\nGoodbye %s!";

/**
 * @brief 
 */
const char END_COMMAND_USR_NOTFOUND_RESPONSE_STR[]                = "User '%s' is not registered!\nUse '/start' command if you want to register";

/**
 * @brief Error message for invalid commands.
 */
const char ERROR_INVALID_COMMAND_STR[]                            = "[ERROR]\nInvalid command [%s].";

/**
 * @brief Error message for unregistered or unauthorized users.
 */
const char ERROR_INVALID_USER_STR[]                               = "[ERROR]\nInvalid user [%s].";

/**
 * @brief Error message for invalid or missing parameters.
 */
const char ERROR_INVALID_PARAMETERS_STR[]                         = "[ERROR]\nInvalid parameters for  [%s] command.";

/**
 * @brief Error message for unregistered tank.
 */
const char ERROR_NO_TANK_STR[]                                    = "[Error]\nThere is no tank regitered yet.\nPlease use '/newTank' command first.";

/**
 * @brief 
 */
const char ERROR_STATUS_COMMAND_STR[]                             = "[Tank Status Error]\
                                                                    \nCan't get tank status.\
                                                                    \nPlease try again.";

/**
 * @brief Alert message indicating the tank is low or empty.
 */
const char ALERT_TANK_EMPTY[]                                     = "[ALERT]\nTank is low!";

#endif // TELEGRAM_BOT_LIB_H