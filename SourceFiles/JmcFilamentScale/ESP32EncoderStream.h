/* -*- C++ -*- */
/**************

ESP32EncoderStream.h

Jun. 2016
Modified by Christophe Persoz and Rui Azevedo.
Based on keyStream.h developed by Rui Azevado.
and ESP32Encoder library by Peter Dannegger.
https://github.com/christophepersoz/encoder

Sept. 2014 Rui Azevedo - ruihfazevedo(@rrob@)gmail.com

quick and dirty keyboard driver
metaprog keyboard driver where N is the number of keys
all keys are expected to be a pin (buttons)

***/


#ifndef ESP32ENCODERSTREAM_H
#define ESP32ENCODERSTREAM_H

#include <Arduino.h>
#include <ESP32Encoder.h>       // For the quadrature encoder library.
#include <Bounce2.h>            // For button debounce library (Bounce2).
#include <menuDefs.h>

namespace Menu
{

// Emulate a stream based on ESP32Encoder movement returning +/- for every
// 'm_Sensivity' steps.  Buffer not needed because we have an accumulator.
class ESP32EncoderStream : public menuIn
{
    enum ButtonState
    {
        BUTTON_CLEAR = 0,
        BUTTON_ROSE  = 1,
        BUTTON_LONG  = 2
    };

public:
    inline void update()
    {
        m_Pb.update();
        m_Pos = m_Encoder.getCount();
    } // End update().


    ESP32EncoderStream(int enc1Pin, int enc2Pin, int pbPin, int sensivity) :
        m_Encoder(), m_Pb(), m_Sensivity(sensivity), m_OldPos(0), m_Pos(0),
        m_LastPeek(options->navCodes[downCmd].ch), m_ButtopnState(BUTTON_CLEAR)
    {
        // Setup The Encoder.  Note that some encoders may need to attach to
        // the HalfQuad: m_encoder.attachHalfQuad().
        m_Encoder.useInternalWeakPullResistors = NONE;
        m_Encoder.attachFullQuad(enc1Pin, enc2Pin);
        m_Encoder.setFilter(500);
        m_Encoder.clearCount();
        m_OldPos = m_Pos = m_Encoder.getCount();

        // Setup the pushbutton.
        pinMode(pbPin, INPUT_PULLUP);
        m_Pb.attach(pbPin);
        m_Pb.interval(25);
        m_Pb.update();
    } // End constructor.


    inline void setSensivity(int s)
    {
        m_Sensivity = s;
    } // End setSensivity().


    int available(void)
    {
        return peek() != -1;
    } // End available().


    int peek(void)
    {
        update();
        if (m_Pb.rose() || (m_ButtopnState != BUTTON_CLEAR))
        {
            if ((m_Pb.previousDuration() >= LONG_PRESS_INTERVAL) ||
                (m_ButtopnState == BUTTON_LONG))
            {
                m_ButtopnState = BUTTON_LONG;
                return options->navCodes[escCmd].ch;    //menu::escCode;
            }
            else
            {
                m_ButtopnState = BUTTON_ROSE;
                return options->navCodes[enterCmd].ch;  //menu::enterCode;
            }
        }

        m_ButtopnState = BUTTON_CLEAR;

        int d = m_Pos - m_OldPos;
        if (d <= -m_Sensivity)
        {
            m_LastPeek = options->navCodes[downCmd].ch;
            return m_LastPeek;  //menu::downCode;
        }
        if (d >= m_Sensivity)
        {
            m_LastPeek = options->navCodes[upCmd].ch;
            return m_LastPeek;  //menu::upCode;
        }
        return -1;
    } // End peek().


    int read()
    {
        int ch = peek();
        m_ButtopnState = BUTTON_CLEAR;
        if (ch == options->navCodes[upCmd].ch)  //menu::upCode)
        {
            m_OldPos += m_Sensivity;
        }
        else if (ch == options->navCodes[downCmd].ch)   //menu::downCode)
        {
            m_OldPos -= m_Sensivity;
        }
        return ch;
    } // End read().


    void flush()
    {
        m_ButtopnState = BUTTON_CLEAR;
        update();
        m_OldPos = m_Pos;
    } // End flush().


    size_t write(uint8_t v)
    {
        m_OldPos = v;
        return 1;
    } // End write().


    void incEncoder()
    {
        noInterrupts();
        m_Encoder.setCount(m_Encoder.getCount() + m_Sensivity);
        interrupts();
    } // End incEncoder().


    void decEncoder()
    {
        noInterrupts();
        m_Encoder.setCount(m_Encoder.getCount() - m_Sensivity);
        interrupts();
    } // End decEncoder().


    int64_t pauseCount()
    {
        return m_Encoder.pauseCount();
    } // End pauseCount().


    int64_t resumeCount()
    {
        return m_Encoder.resumeCount();
    } // End resumeCount().


    int GetLastEncoderDirection()
    {
        return m_LastPeek;
    } // End GetLastEncoderDirection().


    ESP32Encoder *GetEncoder() { return &m_Encoder; }
    Bounce       *GetPb()      { return &m_Pb;      }

private:

    static const ulong LONG_PRESS_INTERVAL = 1000;

    ESP32Encoder m_Encoder; //associated hardware
    Bounce       m_Pb;
    int8_t       m_Sensivity;
    int          m_OldPos;
    int          m_Pos;
    int          m_LastPeek;
    ButtonState  m_ButtopnState;

}; // End class ESP32EncoderStream.

} //End namespace Menu.

#endif /* ESP32ENCODERSTREAM_H */
