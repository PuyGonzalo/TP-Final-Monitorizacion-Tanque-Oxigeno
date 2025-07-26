/********************************************************************************
 * @file oxygen_monitor.h
 * @brief TODO: Completar
 * @author Gonzalo Puy
 * @date Jun 2024
 *******************************************************************************/

#ifndef OXYGEN_MONITOR_H
#define OXYGEN_MONITOR_H

#include "delay.h"

namespace Module {

  class OxygenMonitor
  {
    public:

      static OxygenMonitor& getInstance(){
        static OxygenMonitor instance;

        return instance;
      }

      OxygenMonitor(const OxygenMonitor&) = delete;
      OxygenMonitor& operator=(const OxygenMonitor&) = delete;

        /*!
        * @brief Initializes the OxygenMonitor module.
        */
        static void init();

        /*!
        * @brief Updates the OxygenMonitor module.
        */
        void update();

    private:

      OxygenMonitor();
      ~OxygenMonitor() = default;
      void _init();
      static void onO2MonitorTimeoutFinishedCallback(); 
      // Util::Delay mDelay;
  };

} // namespace Subsystems

#endif // OXYGEN_MONITOR_H