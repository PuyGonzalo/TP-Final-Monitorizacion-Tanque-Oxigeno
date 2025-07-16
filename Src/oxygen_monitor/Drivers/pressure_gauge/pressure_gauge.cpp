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

  float PressureGauge::get_last_reading()
  {
    return last_reading;
  }

  //-------------------------------------------------------------------------------

  void PressureGauge::update()
  {
    float analog_reading = _pin.read();
    float voltage = analog_reading * ref;
    printf("Analog reading: %.2f V\n\r", voltage);
    if (voltage < MIN_READING_VALUE) {
      voltage = MIN_READING_VALUE;
    }
    else if (voltage > MAX_READING_VALUE ) {
      voltage = MAX_READING_VALUE;
    }

    float pressure = (voltage - MIN_READING_VALUE) * (MAX_PRESS_VALUE / (MAX_READING_VALUE - MIN_READING_VALUE));

    last_reading = pressure;

  }

}; // namespace Drivers