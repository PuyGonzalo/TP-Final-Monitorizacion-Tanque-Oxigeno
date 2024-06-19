/********************************************************************************
 * @file pressure_monitor.cpp
 * @brief Pressure monitor module.
 * @author Gonzalo Puy.
 * @date Jun 2024
 *******************************************************************************/

#include "tank_monitor.h"

//===========================[Private Variables]=================================

static Drivers::PressureGauge pressure_sensor(PRESS_SENSOR_PIN); ///< PressureGauge instance.

namespace Module {

void TankMonitor::init(const float gas_flow, const float tank_capacity, tm_function_callback fp) 
{
  pressure_sensor.init();

  this->gas_flow = gas_flow;
  this->tank_capacity = tank_capacity;
  callback = fp;
}

TankMonitor::TankMonitor()
{
  tank_state = TANK_LEVEL_OK;
}

}; // namespace Module