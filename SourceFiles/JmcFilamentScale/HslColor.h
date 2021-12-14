/////////////////////////////////////////////////////////////////////////////////
// HslColor.h
//
// Contains the class that supports HSL <==> RGB565 conversions and other useful
// methods.  The main conversion code was adapted from the ProgrammerSought web
// site: https://www.programmersought.com/article/33522690198/
//
// History:
// - jmcorbett 27-JUN-2021 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////
#if !defined HSLCOLOR_H
#define HSLCOLOR_H

#include "JmcFilamentScale.h"   // For uint16_t, ...


/////////////////////////////////////////////////////////////////////////////////
// HslColor class
//
// Mainly handles HSL data, but also has conversion methods between HSL and RGB565.
/////////////////////////////////////////////////////////////////////////////////
class HslColor
{
public:
    // Constructors.

    // Default constructor.
    HslColor() : m_Hue(0.0), m_Sat(100.0), m_Lum(100.0) {}

    // Constructor to convert RGB565 value to an HSL object.
    HslColor(uint16_t rgb565);

    // Copy Constructor.
    HslColor(const HslColor &c);
    uint16_t ToRgb565();

    // Setters.
    void SetHue(float hue);
    void SetSat(float sat);
    void SetLum(float lum);
    void SetFromRgb565(uint16_t rgb565);

    // Getters for HSL related data.
    float GetHue() const { return m_Hue; }
    float GetSat() const { return m_Sat; }
    float GetLum() const { return m_Lum; }

    // Getters for RGB565 values.  Dissect the RGB565 value and return an
    // individual component value in the range of - to 255.
    static uint8_t  GetRed(uint16_t rgb565);
    static uint8_t  GetGreen(uint16_t rgb565);
    static uint8_t  GetBlue(uint16_t rgb565);

    // Return RGB565 value of white or black depending on the brightness of the
    // input RGB565 value.  Used to determine a contrasting value that will work
    // as a label or other annotation against the color specified.
    static uint16_t Contrast(uint16_t rgb565);

private:
    // Return the min or max of 3 float values.
    float Min3(float v1, float v2, float v3);
    float Max3(float v1, float v2, float v3);

    // The HSL data itself.
	float m_Hue;        // [0,360]
	float m_Sat;        // [0,100]
	float m_Lum;        // [0,100]
}; // End HslColor class.



#endif // HSLCOLOR_H