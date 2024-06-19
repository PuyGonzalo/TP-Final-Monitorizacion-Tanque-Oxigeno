/********************************************************************************
 * @file pressure_monitor.h
 * @brief Pressure monitor class header file.
 * @author Gonzalo Puy.
 * @date Jun 2024
 *******************************************************************************/

#ifndef PRESSURE_MONITOR_H
#define PRESSURE_MONITOR_H

#include "mbed.h"
#include "pressure_gauge.h"

//=========================[Module Defines]=====================================

namespace Module {

class PressureMonitor {
 
 public:
  
  /**
   * @brief Initializes Pressure Monitor Module
   */
  void init();


}; // Class PressureMonitor

}; // namespace Module

#endif // PRESSURE_MONITOR_H