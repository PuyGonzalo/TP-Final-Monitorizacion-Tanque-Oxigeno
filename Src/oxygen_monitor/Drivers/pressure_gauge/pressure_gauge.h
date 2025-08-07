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

/** @brief Analog pin where the pressure sensor is connected. */
#define PRESS_SENSOR_PIN A1

/** @brief Minimum analog voltage output from the sensor after voltage divider. */
#define MIN_READING_VALUE  0.36f  // ~ 0.5 V * 0.73 (voltage divider)

/** @brief Maximum analog voltage output from the sensor after voltage divider. */
#define MAX_READING_VALUE  3.3f   // ~ 4.5 V * 0.73 (voltage divider)

/** @brief Maximum measurable pressure in bar, depends on sensor model. */
#define MAX_PRESS_VALUE_BAR 200   // [bar]

/** @brief Maximum measurable pressure in psi, depends on sensor model. */
#define MAX_PRESS_VALUE_PSI 3000  // [psi]

namespace Drivers {
  /**
  * @class PressureGauge
  * @brief Provides an interface for reading and converting analog pressure values.
  *
  * This class encapsulates the logic to read analog values from a pressure sensor,
  * convert them to physical units (BAR or PSI), and provide utility functions to
  * retrieve and manage unit configuration.
  */
  class PressureGauge {
  public:

  /**
    * @enum UNITS_T
    * @brief Supported pressure units for conversion and display.
    */
  typedef enum {
    UNIT_BAR = 0,      /**< Pressure in BAR. */
    UNIT_PSI = 1,      /**< Pressure in PSI. */
    UNIT_UNKNOWN = 2   /**< Unit not set or unknown. */
  } unit_t;

    /**
    * @brief Constructs a PressureGauge object.
    * @param pin Analog pin connected to the pressure sensor output.
    */
    PressureGauge(const PinName pin);

    /**
    * @brief Initializes the pressure gauge system.
    *
    * Should be called once before use. Typically used to configure internal references.
    */
    void init();

    /**
    * @brief Reads the current sensor voltage and updates the pressure value.
    *
    * This method should be called periodically to refresh the pressure value.
    */
    void update();

    /**
    * @brief Returns the last updated pressure value in the configured unit.
    * @return float Last measured pressure.
    */
    float get_last_reading();

    /**
    * @brief Sets the pressure unit for conversion and display.
    * @param fUnit Unit to be used (BAR or PSI).
    */
    void setUnit(unit_t fUnit);

    /**
    * @brief Checks if a valid unit has been configured.
    * @retval true If unit is set to BAR or PSI.
    * @retval false If unit is UNKNOWN.
    */
    bool isUnitSet();

    /**
    * @brief Gets the currently configured pressure unit.
    * @return unit_t Current unit configuration.
    */
    unit_t get_unit();  

  private:

    AnalogIn _pin;        /**< Analog input pin used to read sensor. */
    unit_t unit;          /**< Configured unit for pressure value. */
    float lastReading;   /**< Last computed pressure value based on sensor reading. */
    float ref;            /**< ADC reference voltage (typically 3.3V on Nucleo boards). */

  }; // Class PreassureGauge

}; // namespace Drivers

 #endif // PRESSURE_GAUGE_H