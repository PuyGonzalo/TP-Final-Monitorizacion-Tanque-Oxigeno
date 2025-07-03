/****************************************************************************//**
 * @file tank_monitor.h
 * @author Gonzalo Puy.
 * @date Jun 2024
 * @brief Tank level monitor class header file.
 *
 *******************************************************************************/

#ifndef TANK_MONITOR_H
#define TANK_MONITOR_H

#include "mbed.h"
#include <string>
#include "pressure_gauge.h"

//=========================[Module Defines]=====================================
#define PRESSURE_THRESHOLD 0.5f 
#define TANK_D_FACTOR 2.3f      // [L/bar]
#define TANK_E_FACTOR 4.1f      // [L/bar]
#define TANK_M_FACTOR 22.6f     // [L/bar]
#define TANK_H_FACTOR 45.5f     // [L/bar]

/**
 * @brief Command string for starting the bot session.
 */
const char TANK_D_STR[]              = "D";

/**
 * @brief Command string for starting the bot session.
 */
const char TANK_E_STR[]              = "E";

/**
 * @brief Command string for starting the bot session.
 */
const char TANK_M_STR[]              = "M";

/**
 * @brief Command string for starting the bot session.
 */
const char TANK_H_STR[]              = "H";

//===========================[Module Types]=====================================

/**
 * @brief TODO: Completar
 */
typedef enum tank_state {
  TANK_LEVEL_OK = 0,
  TANK_LEVEL_LOW = 1,
} tank_state_t;

/**
 * @brief TODO: Completar
 */
typedef enum tank_type {
  TANK_D = 0,
  TANK_E = 1,
  TANK_M = 2,
  TANK_H = 3,
  TANK_TYPE_NONE = 4
} tank_type_t;


namespace Module {

class TankMonitor {
 
 public:

  static TankMonitor& getInstance(){
    static TankMonitor instance;

    return instance;
  }
  // Erase default C++ copy methods in order to avoid generating accidental copies of singleton.
  TankMonitor(TankMonitor const&) = delete;
  void operator=(TankMonitor const&);
  
  /**
   * @brief Initializes Pressure Monitor Module
   */
  void init();

  /**
   * @brief TODO: Completar
   */
  void update();

  /**
   * @brief TODO: Completar
   */
  void setNewTank(const std::string tankType, const int tankCapacity, const int tankGasFlow);

  /**
   * @brief TODO: Completar
   */
  void setNewGasFlow(const int tankGasFlow);
  /**
   * @brief TODO: Completar
   */
  tank_state_t getTankState();

  /**
   * @brief TODO: Completar
   */
  void getTankStatus();

  /**
  * @brief TODO: Completar
  */
  bool isTankTypeValid(const std::string fTankType);

  /**
  * @brief TODO: Completar
  */
  bool isTankRegistered();


 private:
  
  TankMonitor() {};
  ~TankMonitor() = default;

  void _init();
  tank_type_t _findType(const std::string fTankType);

  tank_state_t tank_state;
  tank_type_t tank_type;
  float gas_flow;
  float tank_capacity;
  bool tankRegistered;

}; // Class PressureMonitor

}; // namespace Module

#endif // PRESSURE_MONITOR_H