/********************************************************************************
 * @file oxygen_monitor.cpp
 * @brief TODO: Completar
 * @author Gonzalo Puy
 * @date Jun 2024
 *******************************************************************************/

#include "oxygen_monitor.h"

#include "arm_book_lib.h"
#include "delay.h"
#include "mbed.h"
#include "telegram_bot.h"
#include "tank_monitor.h"
#include "wifi_com.h"
#include "util.h"

namespace Module {

//=====[Declaration and initialization of private global variables]============

static Timeout o2MonitorTimeout;
static bool isTimeoutFinished;
static constexpr chrono::seconds O2_MONITOR_TIMEOUT = 120s;

//=====[Implementations of public methods]======================================


void OxygenMonitor::init()
{
  getInstance()._init();
  isTimeoutFinished = true;
}


//-----------------------------------------------------------------------------
void OxygenMonitor::update()
{
    if(isTimeoutFinished) 
    {
      Module::TankMonitor::getInstance().update();
      o2MonitorTimeout.detach();
      o2MonitorTimeout.attach(&onO2MonitorTimeoutFinishedCallback, O2_MONITOR_TIMEOUT);
      isTimeoutFinished = false;
    }
    Drivers::WifiCom::getInstance().update();
    Module::TelegramBot::getInstance().update();
}

//=====[Implementations of private methods]==================================

OxygenMonitor::OxygenMonitor() {}


void Module::OxygenMonitor::_init()
{
  
    printf("\nInit TICK\n");
    Util::Tick::Init();
    printf("\nInit WifiCom\n");
    Drivers::WifiCom::init();
    printf("\nInit Telegram BOT\n");
    Module::TelegramBot::init();
    printf("\nInit TankMonitor\n");
    Module::TankMonitor::init();

}

void Module::OxygenMonitor::onO2MonitorTimeoutFinishedCallback()
{
  isTimeoutFinished = true;
}

} // namespace Module