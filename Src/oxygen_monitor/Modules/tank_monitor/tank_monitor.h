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
#include "pressure_gauge.h"

//=========================[Module Defines]=====================================
#define TEMP_THRESHOLD 23.5f //TODO: Cambiar 

//===========================[Module Types]=====================================

/**
 * @brief TODO: Completar
 */
typedef enum tank_state {
  TANK_LEVEL_OK = 0,
  TANK_LEVEL_LOW = 1,
} tank_state_t;

/**
 * @brief Typedef for callback function.
 */
typedef void (*tm_function_callback)();

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
   * @param gas_flow Actual gas flow used on the tank. Must be provided by the user through TelegramBot.
   * @param tank_capacity Capacity of the tank. Must be provided by the user through TelegramBot.
   * @param fp Function pointer to the callback.
   */
  void init(const float gas_flow, const float tank_capacity, tm_function_callback fp);

  /**
   * @brief TODO: Completar
   * @return state of the tank (either OK or LOW).
   */
  tank_state_t update();

  tank_state_t getTankState();

 private:
  
   /**
   * @brief Pressure Monitor Module constructor.
   */
  TankMonitor();

  /**
   * @brief Pressure Monitor Module destructor.
   */
  ~TankMonitor() = default;

  tank_state_t tank_state;
  float gas_flow;
  float tank_capacity;
  tm_function_callback callback;


}; // Class PressureMonitor

}; // namespace Module

#endif // PRESSURE_MONITOR_H