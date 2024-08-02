/********************************************************************************
 * @file pressure_monitor.cpp
 * @brief Pressure monitor module.
 * @author Gonzalo Puy.
 * @date Jun 2024
 *******************************************************************************/

#include "tank_monitor.h"
#include <cstdio>

//===========================[Private Variables]=================================

static Drivers::PressureGauge pressure_sensor(PRESS_SENSOR_PIN); ///< PressureGauge instance.

namespace Module {

void TankMonitor::init(const float gas_flow, const float tank_capacity, tm_function_callback fp) 
{
  pressure_sensor.init();

  this->gas_flow = gas_flow;
  this->tank_capacity = tank_capacity;
  this->callback = fp;

}

TankMonitor::TankMonitor()
{
  this->tank_state = NO_TANK_SETTINGS;
}

tank_state_t TankMonitor::update()
{
  pressure_sensor.update();
  float last_reading = pressure_sensor.get_last_reading();
  HAL_Delay(2000);
  printf("\r\n Esto llega aca jojo\r\n");
  printf("\r\n El valor actual es: %f", last_reading);
  return TANK_LEVEL_OK;
}

}; // namespace Module