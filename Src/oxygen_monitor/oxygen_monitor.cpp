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
// #include "tank_monitor.h"
#include "wifi_com.h"
#include "util.h"
#include <cstdio>

namespace Module {

//=====[Declaration and initialization of private global variables]============

static Module::TelegramBot telegramBot(BOT_API_URL, BOT_TOKEN);
static Timeout o2MonitorTimeout;
static bool isTimeoutFinished;

//=====[Implementations of public methods]======================================


void OxygenMonitor::init()
{
  getInstance()._init();
  isTimeoutFinished = true;
}

//----static-------------------------------------------------------------------
// FishTankGuardian* FishTankGuardian::GetInstance()
// {
//     return mInstance;
// }

//-----------------------------------------------------------------------------
void OxygenMonitor::update()
{
    if(isTimeoutFinished) 
    {
      Module::TankMonitor::getInstance().update();
      o2MonitorTimeout.detach();
      o2MonitorTimeout.attach(&onO2MonitorTimeoutFinishedCallback, 2s); //TODO: Cambiar por 60 segundos
      isTimeoutFinished = false;
    }
    Drivers::WifiCom::getInstance().update();
    telegramBot.update();
}

//=====[Implementations of private methods]==================================

OxygenMonitor::OxygenMonitor() 
    : mDelay(SYSTEM_TIME_INCREMENT_MS)
{}

void Module::OxygenMonitor::_init()
{
  
    printf("\nInit TICK\n");
    Util::Tick::Init();
    printf("\nInit WifiCom\n");
    Drivers::WifiCom::init();
    printf("\nInit Telegram BOT\n");
    telegramBot.init();
    printf("\nInit TankMonitor\n");
    Module::TankMonitor::init();

}

void Module::OxygenMonitor::onO2MonitorTimeoutFinishedCallback()
{
  isTimeoutFinished = true;
}

} // namespace Module