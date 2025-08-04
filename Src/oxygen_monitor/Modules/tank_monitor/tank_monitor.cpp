/****************************************************************************//**
 * @file tank_monitor.cpp
 * @author Gonzalo Puy.
 * @date Jun 2024
 * @brief Tank level monitor module. Implemented with singleton design pattern
          in order to ensure only one instance of the object TankMonitor.

 *******************************************************************************/

#include <cstdio>
#include <string>
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
    float threshold = pressure_sensor.get_unit() == Drivers::PressureGauge::UNIT_BAR ? PRESSURE_THRESHOLD_BAR : PRESSURE_THRESHOLD_PSI;
    printf("Last reading: %.2f\n\r", last_reading);
    if (last_reading < threshold) {

      tank_state = TANK_LEVEL_LOW;
    } else {
      tank_state = TANK_LEVEL_OK;
    }
    
    // Check if sensor is disconnected:
    if (last_reading == 0) {
      tank_state = TANK_LEVEL_UNKNOWN;
    }
  }

  void TankMonitor::setNewTank(const std::string tankType, const int tankCapacity, const float tankGasFlow)
  {
    tank_type = _findType(tankType);
    tank_capacity = (float) tankCapacity;
    gas_flow = tankGasFlow;
    tankRegistered = true;
  }

  void TankMonitor::setNewGasFlow(const float tankGasFlow)
  {
    gas_flow = tankGasFlow;
  }

  tank_state_t TankMonitor::getTankState()
  {
    return tank_state;
  }

  float TankMonitor::getTankStatus(float &lastReading, float &currentGasFlow)
  {
    if (!tankRegistered) return -1;
    if (gas_flow == 0) return -1;
    if (!pressure_sensor.isUnitSet()) return -1;

    Drivers::PressureGauge::unit_t unit = pressure_sensor.get_unit();
    pressure_sensor.update();
    lastReading = pressure_sensor.get_last_reading();
    currentGasFlow = gas_flow;

    if (unit == Drivers::PressureGauge::UNIT_BAR && tank_type == TANK_TYPE_NONE) {
      float count = tank_capacity > 20 ? (lastReading - BIG_TANK_RESIDUAL_BAR) : (lastReading - SMALL_TANK_RESIDUAL_BAR);
      float availabeVolume = count * tank_capacity;
      float time = availabeVolume / gas_flow;

      return time;
    } else if (tank_type != TANK_TYPE_NONE){
      float factor = _getTypeFactor(unit);
      float count = unit == Drivers::PressureGauge::UNIT_PSI ? (lastReading - TANK_RESIDUAL_PSI) : (lastReading - TANK_RESIDUAL_BAR);
      float availabeVolume = count * factor;
      float time = availabeVolume / gas_flow;

      return time;
    } else {
      return -1;
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

  bool TankMonitor::isUnitSet()
  {
    return pressure_sensor.isUnitSet();
  }

  bool TankMonitor::setPressureGaugeUnit(std::string unitStr)
  {
    if (unitStr == "bar" || unitStr == "BAR"){
      pressure_sensor.setUnit(Drivers::PressureGauge::UNIT_BAR);
      return true;
    } else if (unitStr == "psi" || unitStr == "PSI") {
      pressure_sensor.setUnit(Drivers::PressureGauge::UNIT_PSI);
      return true;
    } else {
      return false;
    }
  }

  std::string TankMonitor::getPressureGaugeUnitStr()
  {
    Drivers::PressureGauge::unit_t currentUnit;
    std::string result;

    currentUnit = pressure_sensor.get_unit();

    if (currentUnit == Drivers::PressureGauge::UNIT_BAR){
      result = "BAR";
    } else if (currentUnit == Drivers::PressureGauge::UNIT_PSI) {
      result = "PSI";
    }else {
      result = "Unknown";
    }

    return result;
  }

  //=====[Implementations of private methods]===================================

  /**
  * @brief TODO: Completar
  */
  void TankMonitor::_init()
  {
    pressure_sensor.init();

    tank_state = TANK_LEVEL_UNKNOWN;
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
    } else if (fTankType == TANK_G_STR || fTankType == "g") {
      return TANK_G;
    } else if (fTankType == TANK_H_STR || fTankType == "h") {
      return TANK_H;
    } 

    return TANK_TYPE_NONE;
  }

  /**
  * @brief TODO: Completar
  */
  float TankMonitor::_getTypeFactor(Drivers::PressureGauge::unit_t unit)
  {
    if (unit == Drivers::PressureGauge::UNIT_BAR)
    {
      if (tank_type == TANK_D ) {
        return TANK_D_FACTOR_BAR;
      } else if (tank_type == TANK_E) {
        return TANK_E_FACTOR_BAR;
      } else if (tank_type == TANK_M) {
        return TANK_M_FACTOR_BAR;
      } else if (tank_type == TANK_G) {
        return TANK_G_FACTOR_BAR;
      } else if (tank_type == TANK_H) {
        return TANK_H_FACTOR_BAR;
      }
    }

    if (unit == Drivers::PressureGauge::UNIT_PSI)
    {
      if (tank_type == TANK_D ) {
        return TANK_D_FACTOR_PSI;
      } else if (tank_type == TANK_E) {
        return TANK_E_FACTOR_PSI;
      } else if (tank_type == TANK_M) {
        return TANK_M_FACTOR_PSI;
      } else if (tank_type == TANK_G) {
        return TANK_G_FACTOR_PSI;
      } else if (tank_type == TANK_H) {
        return TANK_H_FACTOR_PSI;
      }
    }

    return 0.0;
  }

}; // namespace Module