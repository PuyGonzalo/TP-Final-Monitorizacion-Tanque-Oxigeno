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
//#include "delay.h"
#include "UnbufferedSerial.h"
#include "mbed.h"

#define WIFI_PIN_TX     PR_8
#define WIFI_PIN_RX     PE_7
#define WIFI_BAUD_RATE  115200
#define WIFI_SSID "Nombre-WIFI"
#define WIFI_PASSWORD "Password123-WIFI"

namespace Drivers {
  class WifiCom
  {
    public:

      WifiCom(PinName txPin, PinName rx_pin, const int baudRate);

      ~WifiCom() = default;

      void init();

      void update();

      bool isBusy();

      void post(const std::string& server, const std::string& request);

      void request(const std::string& url);

      bool getPostResponse(std::string* response);

      bool getGetResponse(std::string* response);

    private:

      typedef enum wifi_state {
        INIT,
        CMD_STATUS_SEND,
        CMD_STATUS_WAIT_RESPONSE,
        CMD_CONNECT_SEND,
        CMD_CONNECT_WAIT_RESPONSE,
        CMD_ACCESSPOINT_SEND,
        CMD_ACCESSPOINT_WAIT_RESPONSE,
        CMD_GET_SEND,
        CMD_GET_WAIT_RESPONSE,
        CMD_GET_RESPONSE_READY,
        CMD_POST_SEND,
        CMD_POST_WAIT_RESPONSE,
        CMD_POST_RESPONSE_READY,
        IDLE,
        ERROR
      } wifi_state_t;

      void _sendCommand(const char* command);

      bool _isResponseCompleted(std::string* response);

      bool _readCom(char* receivedChar);

      wifi_state_t  wifiState;
      UnbufferedSerial  wifiSerial;
      std::string       wifiSsid;
      std::string       wifiPassword;
      std::string       wifiResponse;
      std::string       wifiCommandGetResponse;
      std::string       wifiServer;
      std::string       wifiRequest;
      bool              wifiIsResponseReady;
      bool              wifiIsGetResponseReady;
  };
} // namespace Drivers

#endif // WIFI_COM