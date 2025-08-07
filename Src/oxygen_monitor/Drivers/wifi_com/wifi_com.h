/********************************************************************************
 * @file wifi_com.h
 * @brief Wifi communication Driver header file.
 * @author Gonzalo Puy.
 * @date Aug 2024
 *******************************************************************************/
#ifndef WIFI_COM
#define WIFI_COM

#include <string.h>
#include <string>
#include "delay.h"
#include "UnbufferedSerial.h"
#include "mbed.h"

//=========================[Driver Configuration Defines]=========================

/** @brief UART TX pin used for WiFi module. */
#define WIFI_PIN_TX     PA_9

/** @brief UART RX pin used for WiFi module. */
#define WIFI_PIN_RX     PA_10

/** @brief Baud rate for UART communication with WiFi module. */
#define WIFI_BAUD_RATE  115200

/** @brief Default SSID for WiFi connection. */
#define WIFI_SSID       "Royale With Cheese"

/** @brief Default WiFi password. */
#define WIFI_PASSWORD   "Milkra264"

namespace Drivers {
  /**
  * @class WifiCom
  * @brief Handles communication with WiFi module (ESP32) via commands.
  *
  * The WifiCom class implements a singleton-based driver that establishes
  * a WiFi connection, sends GET and POST requests, and manages communication
  * state using a basic FSM. This class communicates with an ESP32 as Wi-Fi a
  * module over UART.
  */
  class WifiCom
  {
    public:

      /**
      * @brief Get singleton instance of WifiCom.
      * 
      * @return Reference to the single instance of WifiCom.
      */
      static WifiCom& getInstance(){
        static WifiCom instance(WIFI_PIN_TX, WIFI_PIN_RX, WIFI_BAUD_RATE);

        return instance;
      }

      WifiCom(const WifiCom&) = delete;
      WifiCom& operator=(const WifiCom&) = delete;

      /**
      * @brief Initializes the WiFi communication module.
      */
      static void init();

      /**
      * @brief Updates the internal FSM of the WiFi communication module.
      * 
      * Should be called periodically to drive the state machine and process
      * responses from the WiFi module.
      */
      void update();

      /**
      * @brief Checks if the module is currently processing a command.
      * 
      * @return true if busy; false if ready to receive a new command.
      */
      bool isBusy();

      /**
      * @brief Sends a POST request to a remote server.
      * 
      * @param server Server URL or IP address.
      * @param request Complete HTTP request payload.
      */
      void post(const std::string& server, const std::string& request);

      /**
      * @brief Sends a GET request to a specific URL.
      * 
      * @param url Full URL for the GET request.
      */
      void request(const std::string& url);

      /**
      * @brief Retrieves the server response from the last POST request.
      * 
      * @param response Pointer to store the response string.
      * @return true if response is available; false otherwise.
      */
      bool getPostResponse(std::string* response);

      /**
      * @brief Retrieves the server response from the last GET request.
      * 
      * @param response Pointer to store the response string.
      * @return true if response is available; false otherwise.
      */
      bool getGetResponse(std::string* response);

    private:

      WifiCom(PinName txPin, PinName rxPin, const int baudRate);
      ~WifiCom() = default;

      void _init();

      /**
      * @enum wifi_state_t
      * @brief FSM states for WiFi communication.
      */
      typedef enum wifi_state {
        INIT,                       /**< Initialization state. */
        CMD_STATUS_SEND,            /**< Sending status command. */
        CMD_STATUS_WAIT_RESPONSE,   /**< Waiting for status command response. */
        CMD_CONNECT_SEND,           /**< Sending connect command with SSID/PASSWORD. */
        CMD_CONNECT_WAIT_RESPONSE,  /**< Waiting for WiFi connection response. */
        CMD_GET_SEND,               /**< Sending GET request. */
        CMD_GET_WAIT_RESPONSE,      /**< Waiting for GET response. */
        CMD_GET_RESPONSE_READY,     /**< GET response is ready. */
        CMD_POST_SEND,              /**< Sending POST request. */
        CMD_POST_WAIT_RESPONSE,     /**< Waiting for POST response. */
        CMD_POST_RESPONSE_READY,    /**< POST response is ready. */
        IDLE,                       /**< Idle state (ready). */
        ERROR                       /**< Error state. */
      } wifi_state_t;

      void _sendCommand(const char* command);

      bool _isResponseCompleted(std::string* response);

      bool _readCom(char* receivedChar);

      wifi_state_t   wifiState;               /**< Current FSM state. */
      UnbufferedSerial wifiSerial;            /**< Serial interface for WiFi communication. */
      Util::Delay    wifiComDelay;            /**< Delay helper for timing between states. */
      std::string    wifiSsid;                /**< SSID of the WiFi network. */
      std::string    wifiPassword;            /**< Password for the WiFi network. */
      std::string    wifiResponse;            /**< Full response buffer. */
      std::string    wifiCommandGetResponse;  /**< Temporary response buffer for GET. */
      std::string    wifiServer;              /**< Server URL for POST requests. */
      std::string    wifiRequest;             /**< HTTP payload for POST requests. */
      bool           wifiIsResponseReady;     /**< Flag indicating response to POST is ready. */
      bool           wifiIsGetResponseReady;  /**< Flag indicating response to GET is ready. */
  };
} // namespace Drivers

#endif // WIFI_COM