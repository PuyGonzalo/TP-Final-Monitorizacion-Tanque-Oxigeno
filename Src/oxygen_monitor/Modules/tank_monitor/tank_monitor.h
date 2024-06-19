/********************************************************************************
 * @file tank_monitor.h
 * @brief Tank level monitor class header file.
 * @author Gonzalo Puy.
 * @date Jun 2024
 *******************************************************************************/

#ifndef TANK_MONITOR_H
#define TANK_MONITOR_H

#include "mbed.h"
#include "pressure_gauge.h"

//=========================[Module Defines]=====================================

//===========================[Module Types]=====================================

/**
 * @brief TODO: Completar
 */
typedef enum pressure_state {
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

 /**
   * @brief Pressure Monitor Module constructor.
   */
  TankMonitor();

  /**
   * @brief Pressure Monitor Module destructor.
   */
  ~TankMonitor() = default;
  
  /**
   * @brief Initializes Pressure Monitor Module
   * @param gas_flow Actual gas flow used on the tank. Must be provided by the user through TelegramBot.
   * @param tank_capacity Capacity of the tank. Must be provided by the user through TelegramBot.
   */
  void init(const float gas_flow, const float tank_capacity, tm_function_callback fp);

  /**
   * @brief TODO: Completar
   * @return state of the tank (either OK or LOW).
   */
  tank_state_t update();

 private:
  tank_state_t tank_state;
  float gas_flow;
  float tank_capacity;
  tm_function_callback callback;


}; // Class PressureMonitor

}; // namespace Module

#endif // PRESSURE_MONITOR_H