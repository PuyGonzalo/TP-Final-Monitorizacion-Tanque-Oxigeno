/****************************************************************************//**
 * @file tank_monitor.cpp
 * @author Gonzalo Puy.
 * @date Jun 2024
 * @brief Tank level monitor module. Implemented with singleton design pattern
          in order to ensure only one instance of the object TankMonitor.
          More info <a href="https://stackoverflow.com/a/1008289">available here</a>.

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

tank_state_t TankMonitor::getTankState()
{
  return TANK_LEVEL_LOW;
}

}; // namespace Module