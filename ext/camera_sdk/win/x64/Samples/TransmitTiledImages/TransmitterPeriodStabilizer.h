// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

///
/// \class TransmitterPeriodStabilizer
///
/// \brief Encapsulate the time manipulation to check to time to wait
/// before the next transmit
///
class TransmitterPeriodStabilizer
{
public:
    ///
    /// \brief Constructor
    ///
    TransmitterPeriodStabilizer() 
    {
        Reset();
    }

    ///
    /// \brief Destructor
    ///
    virtual ~TransmitterPeriodStabilizer() {}

    ///
    /// \brief Reset the internal value
    ///
    void Reset()
    {
        mPeriod = 0.0F;
    }

    ///
    /// \brief Set the expected framerate
    ///
    /// \param aFps The expected fps
    ///
    inline void SetFPS( double aFps )
    {
        mPeriod = 1000.0F / aFps;
        mNextExpectedStop = 0;
    }

    ///
    /// \brief Prepare the stabiliser for the first iteration
    inline void Prepare()
    {
        mNextExpectedStop = ( double ) ::GetTickCount() + mPeriod;
    }

    ///
    /// \brief Log the start of an iteration
    ///
    inline void Start()
    {
        mNextExpectedStop += mPeriod;
    }

    ///
    /// \brief Return the time to reach the period
    ///
    /// \return The time to wait in ms
    ///
    inline DWORD WaitTime()
    {
        double lRemainingTime;

        lRemainingTime = mNextExpectedStop - ::GetTickCount();
        if( lRemainingTime > 1.0F )
        {
            return ( DWORD ) lRemainingTime;
        }

        return 0;
    }

private:
    // Period to respect
    double mPeriod; 
        
    // Next expected period elapsing
    double mNextExpectedStop;
};
