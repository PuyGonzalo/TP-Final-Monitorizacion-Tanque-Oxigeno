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

#define PRESSURE_THRESHOLD_BAR     34.0f    /**< Low pressure threshold in BAR */
#define SMALL_TANK_RESIDUAL_BAR    10       /**< Residual pressure for small tanks in BAR */
#define BIG_TANK_RESIDUAL_BAR      20       /**< Residual pressure for big tanks in BAR */
#define TANK_RESIDUAL_BAR          13.8f    /**< Average residual pressure in BAR */

#define TANK_D_FACTOR_BAR          2.3f     /**< Tank D capacity factor in [L/bar] */
#define TANK_E_FACTOR_BAR          3.5f     /**< Tank E capacity factor in [L/bar] */
#define TANK_M_FACTOR_BAR          17.4f    /**< Tank M capacity factor in [L/bar] */
#define TANK_G_FACTOR_BAR          27.0f    /**< Tank G capacity factor in [L/bar] */
#define TANK_H_FACTOR_BAR          35.0f    /**< Tank H capacity factor in [L/bar] */

#define TANK_RESIDUAL_PSI          200      /**< Residual pressure in PSI */
#define PRESSURE_THRESHOLD_PSI     500.0f   /**< Low pressure threshold in PSI */

#define TANK_D_FACTOR_PSI          0.16f    /**< Tank D capacity factor in [L/psi] */
#define TANK_E_FACTOR_PSI          0.28f    /**< Tank E capacity factor in [L/psi] */
#define TANK_M_FACTOR_PSI          1.56f    /**< Tank M capacity factor in [L/psi] */
#define TANK_G_FACTOR_PSI          2.41f    /**< Tank G capacity factor in [L/psi] */
#define TANK_H_FACTOR_PSI          3.14f    /**< Tank H capacity factor in [L/psi] */

//===========================[Module Strings]===================================

/** @brief String for tank type D. */
const char TANK_D_STR[] = "D";

/** @brief String for tank type E. */
const char TANK_E_STR[] = "E";

/** @brief String for tank type M. */
const char TANK_M_STR[] = "M";

/** @brief String for tank type G. */
const char TANK_G_STR[] = "G";

/** @brief String for tank type H. */
const char TANK_H_STR[] = "H";

//===========================[Module Types]=====================================

/**
 * @enum tank_state_t
 * @brief Represents the current level condition of the tank.
 */
typedef enum tank_state {
  TANK_LEVEL_OK = 0,         /**< Tank pressure is within normal range. */
  TANK_LEVEL_LOW = 1,        /**< Tank pressure is below threshold (alert). */
  TANK_LEVEL_UNKNOWN = 2     /**< Tank status cannot be determined. */
} tank_state_t;

/**
 * @enum tank_type_t
 * @brief Enumerates the supported tank types.
 */
typedef enum tank_type {
  TANK_D = 0,           /**< Tank type D. */
  TANK_E = 1,           /**< Tank type E. */
  TANK_M = 2,           /**< Tank type M. */
  TANK_G = 3,           /**< Tank type G. */
  TANK_H = 4,           /**< Tank type H. */
  TANK_TYPE_NONE = 5    /**< No valid tank type configured. */
} tank_type_t;

namespace Module {
  /**
  * @class TankMonitor
  * @brief Singleton class that provides monitoring of oxygen tank status and remaining time.
  *
  * The TankMonitor class integrates with the PressureGauge to read pressure data,
  * and calculates tank status based on known tank types or volumes and configured gas flow.
  */
  class TankMonitor {
  
  public:

    /**
    * @brief Gets the singleton instance of the TankMonitor.
    * @return Reference to the TankMonitor instance.
    */
    static TankMonitor& getInstance(){
      static TankMonitor instance;

      return instance;
    }

    // Erase default C++ copy methods in order to avoid generating accidental copies of singleton.
    TankMonitor(const TankMonitor&) = delete;
    TankMonitor& operator=(const TankMonitor&) = delete;
    
    /**
    * @brief Initializes Tank Monitor Module
    */
    static void init();

    /**
    * @brief Periodically updates pressure and tank state.
    *
    * This method should be called regularly in the main loop.
    */
    void update();

    /**
    * @brief Registers a new tank by type or volume and sets gas flow.
    * @param tankType The tank type string (e.g., "D", "E", "G").
    * @param tankCapacity The tank volume in liters (used if no type is known).
    * @param tankGasFlow The gas flow in L/min.
    */
    void setNewTank(const std::string fTankType, const int fTankCapacity, const float tankGasFlow);

    /**
    * @brief Sets a new gas flow rate.
    * @param tankGasFlow The new gas flow rate in L/min.
    */
    void setNewGasFlow(const float tankGasFlow);

    /**
    * @brief Returns the current tank status (OK, LOW, or UNKNOWN).
    * @return tank_state_t representing the tank condition.
    */
    tank_state_t getTankState();

    /**
    * @brief Estimates remaining tank time based on pressure and gas flow.
    * @param lastReading Reference to store the latest pressure reading.
    * @param currentGasFlow Reference to store the current gas flow.
    * @return float Remaining time in minutes.
    */
    float getTankStatus(float &lastReading, float &currentGasFlow);

    /**
    * @brief Validates a given tank type string.
    * @param fTankType The tank type string.
    * @retval true if valid, false otherwise.
    */
    bool isTankTypeValid(const std::string fTankType);

    /**
    * @brief Checks if a tank has been registered.
    * @retval true if registered, false otherwise.
    */
    bool isTankRegistered();

    /**
    * @brief Checks if the pressure unit has been set.
    * @retval true if unit is configured, false otherwise.
    */
    bool isUnitSet();

    /**
    * @brief Sets the pressure gauge unit (BAR or PSI).
    * @param unitStr Unit string ("BAR" or "PSI").
    * @retval true if set successfully, false if invalid.
    */
    bool setPressureGaugeUnit(std::string unitStr);

    /**
    * @brief Gets the currently configured pressure unit as string.
    * @return std::string representing the unit ("BAR", "PSI", or "UNKNOWN").
    */
    std::string getPressureGaugeUnitStr();

  private:
    
    TankMonitor() {};
    ~TankMonitor() = default;

    void _init();
    tank_type_t _findType(const std::string fTankType);
    float _getTypeFactor(Drivers::PressureGauge::unit_t unit);

    tank_state_t tankState;  /**< Current state of the tank. */
    tank_type_t tankType;    /**< Registered tank type. */
    float gasFlow;           /**< Current gas flow rate [L/min]. */
    float tankCapacity;      /**< Tank volume [L], if type is not set. */
    bool tankRegistered;     /**< Indicates whether a tank has been registered. */

  }; // Class PressureMonitor

}; // namespace Module

#endif // PRESSURE_MONITOR_H