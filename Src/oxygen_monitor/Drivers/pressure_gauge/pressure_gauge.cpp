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
    ref = 5.0;
  }

  //-------------------------------------------------------------------------------

  float PressureGauge::get_last_reading()
  {
    return last_reading;
  }

  //-------------------------------------------------------------------------------

  void PressureGauge::update()
  {
    float analog_reading = _pin.read();

    // Logic to transform reading to pressure
    // TODO: HACER!!!
    float pressure_value = analog_reading * MAX_PRESS_VALUE;

    if (pressure_value < MIN_PRESS_VALUE) {
      last_reading = MIN_PRESS_VALUE;
    }
    else if (pressure_value > MAX_PRESS_VALUE) {
      last_reading = MAX_PRESS_VALUE;
    }
    else {
      last_reading = pressure_value;
    }

  }

}; // namespace Drivers