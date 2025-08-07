/********************************************************************************
 * @file oxygen_monitor.h
 * @brief Oxygen monitoring control module.
 * @author Gonzalo Puy
 * @date Jun 2024
 *******************************************************************************/

#ifndef OXYGEN_MONITOR_H
#define OXYGEN_MONITOR_H

#include "delay.h"

namespace Module {

  /**
   * @class OxygenMonitor
   * @brief Singleton class for managing the oxygen monitoring logic.
   * 
   * This class is responsible for coordinating periodic monitoring cycles.
   * It work in conjunction with other modules like TankMonitor or the TelegramBot
   */
  class OxygenMonitor
  {
    public:
      /**
       * @brief Retrieves the singleton instance of OxygenMonitor.
       * @return Reference to the OxygenMonitor instance.
       */
      static OxygenMonitor& getInstance(){
        static OxygenMonitor instance;

        return instance;
      }

      /// Delete copy constructor and copy assignment.
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
  };

} // namespace Subsystems

#endif // OXYGEN_MONITOR_H