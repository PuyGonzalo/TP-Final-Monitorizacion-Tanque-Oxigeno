/********************************************************************************
 * @file pressure_gauge.cpp
 * @brief TODO: Terminar
 * @author Gonzalo Puy.
 * @date Jun 2024
 *******************************************************************************/
#include "mbed.h" 
#include "pressure_gauge.h"


namespace Drivers {

  //====================[Implementations of public methods]========================

  PressureGauge::PressureGauge(const PinName pin)
    : _pin(pin)
  {
  }

  //-------------------------------------------------------------------------------

  void PressureGauge::init()
  {
    last_reading = 0.0;
    ref = 3.3f;
  }

  //-------------------------------------------------------------------------------

  void PressureGauge::update()
  {
    float pressure;
    float analog_reading = _pin.read();
    float voltage = analog_reading * ref;
    printf("Analog reading: %.2f V\n\r", voltage);
    if (voltage < MIN_READING_VALUE) {
      voltage = MIN_READING_VALUE;
    }

    if (unit == UNIT_BAR) {
      pressure = (voltage - MIN_READING_VALUE) * (MAX_PRESS_VALUE_BAR / (MAX_READING_VALUE - MIN_READING_VALUE));
    } else if (unit == UNIT_PSI) {
      pressure = (voltage - MIN_READING_VALUE) * (MAX_PRESS_VALUE_PSI / (MAX_READING_VALUE - MIN_READING_VALUE));
    } else {
      pressure = 0;
    }
    
    last_reading = pressure;

  }

  //-------------------------------------------------------------------------------

  float PressureGauge::get_last_reading()
  {
    return last_reading;
  }

  //-------------------------------------------------------------------------------

  void PressureGauge::setUnit(unit_t fUnit)
  {
    unit = fUnit;
  }

  bool PressureGauge::isUnitSet()
  {
    return (unit != UNIT_UNKNOWN);
  }

  PressureGauge::unit_t PressureGauge::get_unit()
  {
    return unit;
  }

}; // namespace Drivers