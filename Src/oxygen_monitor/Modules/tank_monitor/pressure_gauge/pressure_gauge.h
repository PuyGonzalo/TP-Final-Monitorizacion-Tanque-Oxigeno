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

//=========================[Module Defines]=====================================

#define PRESS_SENSOR_NUM_SAMPLES 5
#define PRESS_SENSOR_PIN A1
#define MIN_PRESS_VALUE  0.0
#define MAX_PRESS_VALUE  99.9

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
  float get_last_reading(); /// TODO: Ver si me sirve.

 private:
  using PressReadingsVec = std::vector<float>; // Aliasing used for vector class.

  AnalogIn _pin; ///< AnalogIn pin.
  PressReadingsVec pressure_reading_vec; ///< Vector used for measure storage. TODO: Ver si me sirve o no. Por ahora lo dejo
  PressReadingsVec::iterator pressure_reading_iter; ///< Iterator for measure storage vector.
  float last_reading; ///< Last value read.
  float ref; ///< TODO: Completar esto.

}; // Class PreassureGauge

}; // namespace Drivers

 #endif // PRESSURE_GAUGE_H