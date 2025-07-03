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
#define PRESSURE_THRESHOLD 14.0f     // [bar] TODO: Cambiar!!! Esto no esta en bar
#define TANK_RESERVE 14.0f           // [bar]
#define TANK_TYPICAL_PRESS 138.0f   // [bar]
#define TANK_D_FACTOR 2.3f          // [L/bar]
#define TANK_E_FACTOR 4.1f          // [L/bar]
#define TANK_M_FACTOR 22.6f         // [L/bar]
#define TANK_H_FACTOR 45.5f         // [L/bar]

/**
 * @brief String for tank type D.
 */
const char TANK_D_STR[]              = "D";

/**
 * @brief String for tank type E.
 */
const char TANK_E_STR[]              = "E";

/**
 * @brief String for tank type M.
 */
const char TANK_M_STR[]              = "M";

/**
 * @brief String for tank type H.
 */
const char TANK_H_STR[]              = "H";

//===========================[Module Types]=====================================

/**
 * @enum TANK_STATE
 * @brief TODO: Completar
 */
typedef enum tank_state {
  TANK_LEVEL_OK = 0,
  TANK_LEVEL_LOW = 1,
} tank_state_t;

/**
 * @enum TANK_TYPE
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
  TankMonitor(const TankMonitor&) = delete;
  TankMonitor& operator=(const TankMonitor&) = delete;
  
  /**
   * @brief Initializes Pressure Monitor Module
   */
  static void init();

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
   * @return
   */
  tank_state_t getTankState();

  /**
   * @brief TODO: Completar
   * @return
   */
  float getTankStatus();

  /**
  * @brief TODO: Completar
  * @return 
  */
  bool isTankTypeValid(const std::string fTankType);

  /**
  * @brief TODO: Completar
  * @return 
  */
  bool isTankRegistered();


 private:
  
  TankMonitor() {};
  ~TankMonitor() = default;

  void _init();
  tank_type_t _findType(const std::string fTankType);
  float _getTypeFactor();

  tank_state_t tank_state;  /**< Current tank state. */
  tank_type_t tank_type;    /**< Current tank type. */
  float gas_flow;           /**< Current gas flow [L/min] */
  float tank_capacity;      /**< Current tank capacity [L] */
  bool tankRegistered;      /**< Tank registered */

}; // Class PressureMonitor

}; // namespace Module

#endif // PRESSURE_MONITOR_H