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

//=====[Implementations of public methods]=======================================

namespace Module {

  void TankMonitor::init() 
  {
    getInstance()._init();
  }

  void TankMonitor::update()
  {
    pressure_sensor.update();
    float last_reading = pressure_sensor.getLastReading();
    float threshold = pressure_sensor.get_unit() == Drivers::PressureGauge::UNIT_BAR ? PRESSURE_THRESHOLD_BAR : PRESSURE_THRESHOLD_PSI;
    printf("TankMonitor - Last reading: [%.2f]\n\r", last_reading);
    if (last_reading < threshold) {

      tankState = TANK_LEVEL_LOW;
    } else {
      tankState = TANK_LEVEL_OK;
    }
    
    if (last_reading == 0) {
      tankState = TANK_LEVEL_UNKNOWN;
    }
  }

  void TankMonitor::setNewTank(const std::string fTankType, const int fTankCapacity, const float tankGasFlow)
  {
    tankType = _findType(fTankType);
    tankCapacity = (float) fTankCapacity;
    gasFlow = tankGasFlow;
    tankRegistered = true;
  }

  void TankMonitor::setNewGasFlow(const float tankGasFlow)
  {
    gasFlow = tankGasFlow;
  }

  tank_state_t TankMonitor::getTankState()
  {
    return tankState;
  }

  float TankMonitor::getTankStatus(float &lastReading, float &currentGasFlow)
  {
    if (!tankRegistered) return -1;
    if (gasFlow == 0) return -1;
    if (!pressure_sensor.isUnitSet()) return -1;

    Drivers::PressureGauge::unit_t unit = pressure_sensor.get_unit();
    pressure_sensor.update();
    lastReading = pressure_sensor.getLastReading();
    currentGasFlow = gasFlow;

    if (unit == Drivers::PressureGauge::UNIT_BAR && tankType == TANK_TYPE_NONE) {
      float count = tankCapacity > 20 ? (lastReading - BIG_TANK_RESIDUAL_BAR) : (lastReading - SMALL_TANK_RESIDUAL_BAR);
      if (count < 0) return -1;
      float availabeVolume = count * tankCapacity;
      float time = availabeVolume / gasFlow;

      return time;
    } else if (tankType != TANK_TYPE_NONE){
      float factor = _getTypeFactor(unit);
      float count = unit == Drivers::PressureGauge::UNIT_PSI ? (lastReading - TANK_RESIDUAL_PSI) : (lastReading - TANK_RESIDUAL_BAR);
      if (count < 0) return -1;
      float availabeVolume = count * factor;
      float time = availabeVolume / gasFlow;

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
  * @brief Internal init.
  */
  void TankMonitor::_init()
  {
    pressure_sensor.init();

    tankState = TANK_LEVEL_UNKNOWN;
    gasFlow = 0;
    tankCapacity = 0;
    tankType = TANK_TYPE_NONE;
    tankRegistered = false;
  }

  /**
  * @brief Converts a string representation of a tank type to its corresponding enum.
  *
  * This helper function allows case-insensitive matching of known tank type strings
  * ("D", "E", "M", "G", "H") to their respective `tank_type_t` enumeration values.
  *
  * @param fTankType String representing the tank type (e.g., "D", "e", "M").
  * @return Corresponding `tank_type_t` value, or `TANK_TYPE_NONE` if the type is not recognized.
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
  * @brief Returns the capacity conversion factor associated with the tank type.
  *
  * This function determines the appropriate factor used to convert pressure readings
  * to remaining volume, based on the tank type and the currently configured pressure unit.
  * If no valid type is set or the unit is unknown, it returns 0.
  *
  * @param unit Unit system in use (BAR or PSI).
  * @return Tank factor in L/bar or L/psi, or 0 if invalid.
  */
  float TankMonitor::_getTypeFactor(Drivers::PressureGauge::unit_t unit)
  {
    if (unit == Drivers::PressureGauge::UNIT_BAR)
    {
      if (tankType == TANK_D ) {
        return TANK_D_FACTOR_BAR;
      } else if (tankType == TANK_E) {
        return TANK_E_FACTOR_BAR;
      } else if (tankType == TANK_M) {
        return TANK_M_FACTOR_BAR;
      } else if (tankType == TANK_G) {
        return TANK_G_FACTOR_BAR;
      } else if (tankType == TANK_H) {
        return TANK_H_FACTOR_BAR;
      }
    }

    if (unit == Drivers::PressureGauge::UNIT_PSI)
    {
      if (tankType == TANK_D ) {
        return TANK_D_FACTOR_PSI;
      } else if (tankType == TANK_E) {
        return TANK_E_FACTOR_PSI;
      } else if (tankType == TANK_M) {
        return TANK_M_FACTOR_PSI;
      } else if (tankType == TANK_G) {
        return TANK_G_FACTOR_PSI;
      } else if (tankType == TANK_H) {
        return TANK_H_FACTOR_PSI;
      }
    }

    return 0.0;
  }

}; // namespace Module