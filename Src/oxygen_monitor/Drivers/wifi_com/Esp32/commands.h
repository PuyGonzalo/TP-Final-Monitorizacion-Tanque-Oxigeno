/********************************************************************************
 * @file commands.h
 * @brief Wifi communication commands for ESP32.
 * @author Gonzalo Puy.
 * @date Aug 2024
 *******************************************************************************/

 #ifndef COMMANDS_H
 #define COMMANDS_H

const char COMMAND_CONNECT_STR[]      = "connect";
const char COMMAND_POST_STR[]         = "post";
const char COMMAND_GET_STR[]          = "get";
const char COMMAND_STATUS_STR[]       = "status";
const char COMMAND_ACCESSPOINT_STR[]  = "accesspoint";

const char RESULT_ERROR[]             = "ERROR";
const char RESULT_OK[]                = "OK";
const char RESULT_CONNECTED[]         = "CONNECTED";
const char RESULT_NOT_CONNECTED[]     = "NOT_CONNECTED";
const char RESULT_AP_WAITING[]        = "AP_WAITING";

const char PARAM_SEPARATOR_CHAR       = '|';
const char STOP_CHAR                  = '~';

#endif // COMMANDS_H