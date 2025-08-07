/****************************************************************************//**
 * @file main.cpp
 * @brief main file of program.
 * @author Gonzalo Puy.
 * @date Jun 2024
 *******************************************************************************/

#include "mbed.h"
#include "arm_book_lib.h"
#include "oxygen_monitor.h"
#include <cstdio>


int main() 
{
  Module::OxygenMonitor::init();

  while (true)
  {
    Module::OxygenMonitor::getInstance().update();
  }

}
