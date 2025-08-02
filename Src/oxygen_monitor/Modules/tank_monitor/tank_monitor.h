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
#define PRESSURE_THRESHOLD_BAR 34.0f    // [bar]
#define SMALL_TANK_RESIDUAL_BAR 10      // [bar]
#define BIG_TANK_RESIDUAL_BAR 20        // [bar]
#define TANK_RESIDUAL_BAR 13.8f         // [bar]

#define TANK_D_FACTOR_BAR 2.3f          // [L/bar]
#define TANK_E_FACTOR_BAR 3.5f          // [L/bar]
#define TANK_M_FACTOR_BAR 17.4f         // [L/bar]
#define TANK_G_FACTOR_BAR 27.0f         // [L/bar]
#define TANK_H_FACTOR_BAR 35.0f         // [L/bar]

#define TANK_RESIDUAL_PSI 200           // [psi]
#define PRESSURE_THRESHOLD_PSI 500.0f   // [psi]

#define TANK_D_FACTOR_PSI 0.16f         // [L/psi]
#define TANK_E_FACTOR_PSI 0.28f         // [L/psi]
#define TANK_M_FACTOR_PSI 1.56f         // [L/psi]
#define TANK_G_FACTOR_PSI 2.41f         // [L/psi]
#define TANK_H_FACTOR_PSI 3.14f         // [L/psi]

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
 * @brief String for tank type G.
 */
const char TANK_G_STR[]              = "G";

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
  TANK_LEVEL_UNKNOWN = 2
} tank_state_t;

/**
 * @enum TANK_TYPE
 * @brief TODO: Completar
 */
typedef enum tank_type {
  TANK_D = 0,
  TANK_E = 1,
  TANK_M = 2,
  TANK_G = 3,
  TANK_H = 4,
  TANK_TYPE_NONE = 5
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
  void setNewTank(const std::string tankType, const int tankCapacity, const float tankGasFlow);

  /**
   * @brief TODO: Completar
   */
  void setNewGasFlow(const float tankGasFlow);

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
  * @retval true
  * @retval false 
  */
  bool isTankTypeValid(const std::string fTankType);

  /**
  * @brief TODO: Completar
  * @retval true
  * @retval false 
  */
  bool isTankRegistered();

  /**
  * @brief TODO: Completar
  * @retval true
  * @retval false 
  */
  bool isUnitSet();

  /**
  * @brief TODO: Completar
  * @param 
  * @retval true
  * @retval false 
  */
  bool setPressureGaugeUnit(std::string unitStr);

  /**
  * @brief TODO: Completar
  * @return
  */
  std::string getPressureGaugeUnitStr();

 private:
  
  TankMonitor() {};
  ~TankMonitor() = default;

  void _init();
  tank_type_t _findType(const std::string fTankType);
  float _getTypeFactor(Drivers::PressureGauge::unit_t unit);

  tank_state_t tank_state;  /**< Current tank state. */
  tank_type_t tank_type;    /**< Current tank type. */
  float gas_flow;           /**< Current gas flow [L/min] */
  float tank_capacity;      /**< Current tank capacity [L] */
  bool tankRegistered;      /**< Tank registered */

}; // Class PressureMonitor

}; // namespace Module

#endif // PRESSURE_MONITOR_H