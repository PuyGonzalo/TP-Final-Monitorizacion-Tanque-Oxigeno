/*!****************************************************************************
 * @file delay.cpp
 * @brief Implementation of non-blocking delay module
 * @author Gonzalo Puy
 * @date May 2025
 *******************************************************************************/

#include "delay.h"

namespace Util { 

//=====[Declaration and initialization of private global variables]============

tick_t Tick::mTickCounter = 0;
Ticker Tick::mTicker;

//=====[Implementations of public methods]=====================================

//-----------------------------------------------------------------------------
bool Delay::HasFinished()
{
    bool timeArrived = false;
    tick_t elapsedTime;

    if(!mIsRunning) 
    {
        mStartTime = Tick::GetTickCounter();
        mIsRunning = true;
    } 
    else 
    {
        elapsedTime = Tick::GetTickCounter() - mStartTime;
        if (elapsedTime >= mDuration) 
        {
            timeArrived = true;
            mIsRunning = false;
        }
    }

   return timeArrived;
}

//-----------------------------------------------------------------------------
void Delay::Start(tick_t durationValue )
{
   mDuration = durationValue;
}

//----static-------------------------------------------------------------------
void Tick::Init() 
{
    mTicker.attach(TickerCallback, 1ms);
}

//----static-------------------------------------------------------------------
void Tick::TickerCallback() 
{
    mTickCounter++;
}

//----static-------------------------------------------------------------------
tick_t Tick::GetTickCounter() 
{
    return mTickCounter;
}

} // namespace Util