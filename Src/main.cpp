#include "mbed.h"
#include "arm_book_lib.h"
#include "tank_monitor.h"

// main() runs in its own thread in the OS
int main() 
{
  Module::TankMonitor tank;
  tank.init(0.2, 0.2, nullptr);

  while (true) {
    tank.update();
  }
}
