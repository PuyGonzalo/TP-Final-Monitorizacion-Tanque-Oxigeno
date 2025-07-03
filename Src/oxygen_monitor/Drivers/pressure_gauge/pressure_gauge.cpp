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
    printf("Analog reading: %.2f V\n\r", analog_reading);
    if (analog_reading < MIN_READING_VALUE) {
      analog_reading = MIN_READING_VALUE;
    }
    else if (analog_reading > MAX_READING_VALUE ) {
      analog_reading = MAX_READING_VALUE;
    }

    float pressure = (analog_reading - MIN_READING_VALUE) * (MAX_PRESS_VALUE / (MAX_READING_VALUE - MIN_READING_VALUE));

    last_reading = pressure;

  }

}; // namespace Drivers