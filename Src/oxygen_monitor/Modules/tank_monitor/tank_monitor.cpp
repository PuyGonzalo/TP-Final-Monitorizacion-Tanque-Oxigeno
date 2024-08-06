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
  this->tank_state = TANK_LEVEL_OK;
  this->gas_flow = 0;
  this->tank_capacity = 0;
  this->callback = nullptr;
}

tank_state_t TankMonitor::update()
{
  pressure_sensor.update();
  float last_reading = pressure_sensor.get_last_reading();

  if (last_reading < TEMP_THRESHOLD) {

    callback();
  }
  return TANK_LEVEL_OK;
}

}; // namespace Module