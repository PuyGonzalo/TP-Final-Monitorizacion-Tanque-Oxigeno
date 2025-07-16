/********************************************************************************
 * @file pressure_gauge.h
 * @brief Pressure gauge class header file.
 * @author Gonzalo Puy.
 * @date Jun 2024
 *******************************************************************************/

 #ifndef PRESSURE_GAUGE_H
 #define PRESSURE_GAUGE_H

#include <vector>
#include "AnalogIn.h"
#include "PinNames.h"
#include "mbed.h"

//=========================[Driver Defines]=====================================

#define PRESS_SENSOR_PIN A1
#define MIN_READING_VALUE  0.36f // ~ 0.5 V * 0.73 (voltage divider)
#define MAX_READING_VALUE  3.3f // ~ 4.5 V * 0.73 (voltage divider)
#define MAX_PRESS_VALUE 200 // [bar] Depends on pressure gauge model.

namespace Drivers {

class PressureGauge {
 public:

  /**
   * @brief Initializes the pressure gauge.
   */
  void init();

  /**
   * @brief PressureGauge class constructor.
   * @param pin PinName type, represents the analog pin where the board is going to read the pressure value.
   */
  PressureGauge(const PinName pin);

  ~PressureGauge() = default;

  /**
   * @brief update the PressureGauge reading.
   */
  void update();

  /**
   * @brief Get last pressure reading.
   * @return float Last pressure reading.
   */
  float get_last_reading();

 private:

  AnalogIn _pin;        /**< AnalogIn pin. */
  float last_reading;   /**< Last pressure value read. */
  float ref;            /**< Vref value of ADC. On nucleo boars it's usually connected to 3.3 V. */

}; // Class PreassureGauge

}; // namespace Drivers

 #endif // PRESSURE_GAUGE_H