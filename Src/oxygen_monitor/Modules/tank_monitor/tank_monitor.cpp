/****************************************************************************//**
 * @file tank_monitor.cpp
 * @author Gonzalo Puy.
 * @date Jun 2024
 * @brief Tank level monitor module. Implemented with singleton design pattern
          in order to ensure only one instance of the object TankMonitor.

 *******************************************************************************/

#include <cstdio>
#include "tank_monitor.h"

//=====[Declaration and initialization of private global variables]==============

static Drivers::PressureGauge pressure_sensor(PRESS_SENSOR_PIN); /**< PressureGauge instance. */

//=====[Implementations of public methods]============+++=========================

namespace Module {

  void TankMonitor::init() 
  {
    getInstance()._init();
  }

  void TankMonitor::update()
  {
    pressure_sensor.update();
    float last_reading = pressure_sensor.get_last_reading();
    printf("Last reading: %.2f bar\n\r", last_reading);
    if (last_reading < PRESSURE_THRESHOLD) {

      tank_state = TANK_LEVEL_LOW;
    }
    tank_state = TANK_LEVEL_OK;
  }

  void TankMonitor::setNewTank(const std::string tankType, const int tankCapacity, const int tankGasFlow)
  {
    tank_type = _findType(tankType);
    tank_capacity = (float) tankCapacity;
    gas_flow = (float) tankGasFlow;
    tankRegistered = true;
  }

  void TankMonitor::setNewGasFlow(const int tankGasFlow)
  {
    gas_flow = (float) tankGasFlow;
  }

  tank_state_t TankMonitor::getTankState()
  {
    return TANK_LEVEL_OK;
  }

  float TankMonitor::getTankStatus()
  {

    if (!tankRegistered) return 0.0;

    pressure_sensor.update();
    float last_reading = pressure_sensor.get_last_reading();

    if (tank_type == TANK_TYPE_NONE) {
      float availabeVolume = (last_reading / TANK_NOMINAL_PRESS) * tank_capacity;
      float time = availabeVolume / gas_flow;

      return time; // [min]
    } else {
      float factor = _getTypeFactor();
      float availabeVolume = (last_reading - TANK_RESERVE) * factor;
      float time = availabeVolume / gas_flow;

      return time; // [min]
    }
  }

  bool TankMonitor::isTankTypeValid(const std::string fTankType)
  {
    return _findType(fTankType) != TANK_TYPE_NONE;
  }

  bool TankMonitor::isTankRegistered()
  {
    return tankRegistered;
  }

  //=====[Implementations of private methods]===================================

  /**
  * @brief TODO: Completar
  */
  void TankMonitor::_init()
  {
    pressure_sensor.init();

    tank_state = TANK_LEVEL_OK;
    gas_flow = 0;
    tank_capacity = 0;
    tank_type = TANK_TYPE_NONE;
    tankRegistered = false;
  }

  /**
  * @brief TODO: Completar
  */
  tank_type_t TankMonitor::_findType(const std::string fTankType)
  {
    if (fTankType == TANK_D_STR || fTankType == "d") {
      return TANK_D;
    } else if (fTankType == TANK_E_STR || fTankType == "e") {
      return TANK_E;
    } else if (fTankType == TANK_M_STR || fTankType == "m") {
      return TANK_M;
    } else if (fTankType == TANK_H_STR || fTankType == "h") {
      return TANK_H;
    } 

    return TANK_TYPE_NONE;
  }

  /**
  * @brief TODO: Completar
  */
  float TankMonitor::_getTypeFactor()
  {
    if (tank_type == TANK_D ) {
      return TANK_D_FACTOR;
    } else if (tank_type == TANK_E) {
      return TANK_E_FACTOR;
    } else if (tank_type == TANK_M) {
      return TANK_M_FACTOR;
    } else if (tank_type == TANK_H) {
      return TANK_H_FACTOR;
    }

    return 0.0;
  }

}; // namespace Module