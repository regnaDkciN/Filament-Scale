/////////////////////////////////////////////////////////////////////////////////
// AuxPb.h
//
// Contains the AuxPb class.  This class initializes and monitors a pushbutton.
//
// History:
// - jmcorbett 09-JUN-2021 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////
#if !defined AUXPB_H
#define AUXPB_H

#include <Arduino.h>
#include <Bounce2.h>            // For button debounce library (Bounce2).


/////////////////////////////////////////////////////////////////////////////////
// AuxPb class
/////////////////////////////////////////////////////////////////////////////////
class AuxPb
{
public:

    /////////////////////////////////////////////////////////////////////////////
    // ButtonState enum
    //
    // These are the valid states for the pushbutton.
    /////////////////////////////////////////////////////////////////////////////
    enum ButtonState
    {
        BUTTON_CLEAR = 0,
        BUTTON_SHORT = 1,
        BUTTON_LONG  = 2
    };


    /////////////////////////////////////////////////////////////////////////////
    // Constructor.
    //
    // Arguments:
    //    auxPin - Specifies the pin used for this pushbutton.
    /////////////////////////////////////////////////////////////////////////////
    AuxPb(int auxPin) : m_AuxPb()
    {
        // Setup the pushbuttons.
        pinMode(auxPin, INPUT_PULLUP);
        m_AuxPb.attach(auxPin);
        m_AuxPb.interval(25);

        // Clear out any spurious initial push.
        Read();
    } // End constructor.


    /////////////////////////////////////////////////////////////////////////////
    // Read()
    //
    // Returns the state of the pushbutton. (clear, short press, or long press).
    /////////////////////////////////////////////////////////////////////////////
    int Read()
    {
        // Poll the PB.
        m_AuxPb.update();

        // Determine the current state, and return it to the caller.
        if (m_AuxPb.rose())
        {
            if (m_AuxPb.previousDuration() >= LONG_PRESS_INTERVAL)
            {
                return BUTTON_LONG  ;  // Aux PB long.
            }
            else
            {
                return BUTTON_SHORT ;  // Aux PB short.
            }
        }
        else
        {
            return BUTTON_CLEAR;
        }
    } // End Read().


private:
    // Specify how long a press needs to be to be considered a long press (ms).
    static const ulong LONG_PRESS_INTERVAL = 1000;

    Bounce       m_AuxPb;       // The Bounce pushbutton instance.

}; // End AuxPb class.


#endif // AUXPB_H