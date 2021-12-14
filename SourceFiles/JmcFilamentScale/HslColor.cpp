/////////////////////////////////////////////////////////////////////////////////
// HslColor.cpp
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

#include "HslColor.h"      // For our own definitions, etc.


/////////////////////////////////////////////////////////////////////////////////
// GetRed(), GetGreen(), GetBlue()
//
// These methods dissect an RGB565 value and return the value of the selected
// component in the range of 0 to 255.
//
// Arguments:
//   rgb565 - The RGB565 value to be dissected.
//
// Returns:
//   Returns the uint8_t value of the specified component of the rgb565 value
//   in the range of 0 to 255.
/////////////////////////////////////////////////////////////////////////////////
uint8_t HslColor::GetRed(uint16_t rgb565)
{
    return static_cast<uint8_t>(((rgb565 >> 11) << 3)); // | 7;
} // End GetRed().

uint8_t HslColor::GetGreen(uint16_t rgb565)
{
    return static_cast<uint8_t>(((rgb565 >> 5) << 2)); //  | 3;
} // End GetGreen().

uint8_t HslColor::GetBlue(uint16_t rgb565)
{
    return static_cast<uint8_t>(rgb565 << 3); // | 7;
} // End GetBlue().


/////////////////////////////////////////////////////////////////////////////////
// Min3(), Max3()
//
// These helper methods return the minimum or maximum value of 3 floats that are
// passed as arguments.
//
// Arguments:
//   v1, v2, v3 - These are the values with which to perform the calculation.
//
// Returns:
//   Always return the minimum or maximum of the 3 argument values.
/////////////////////////////////////////////////////////////////////////////////
 float HslColor::Min3(float v1, float v2, float v3)
 {
     float min = v1;
     if (v2 < min)
     {
         min = v2;
     }
     if (v3 < min)
     {
         min = v3;
     }
     return min;
 } // End Min2().

float HslColor::Max3(float v1, float v2, float v3)
 {
     float max = v1;
     if (v2 > max)
     {
         max = v2;
     }
     if (v3 > max)
     {
         max = v3;
     }
     return max;
 } // End Max3().


/////////////////////////////////////////////////////////////////////////////////
// Constructor
//
// Constructs an HSL object and converts the RGB565 value argument to HSL values
// of the object.
//
// Arguments:
//   rgb565 - This is the RGB565 value to be converted to HSL of the object.
/////////////////////////////////////////////////////////////////////////////////
HslColor::HslColor(uint16_t rgb565)
{
    SetFromRgb565(rgb565);
} // End constructor.


/////////////////////////////////////////////////////////////////////////////////
// Constructor
//
// This is the copy constructor for the HSL class.
//
// Arguments:
//   c - This is a reference to the HSL object to be copied.
/////////////////////////////////////////////////////////////////////////////////
HslColor::HslColor(const HslColor &c)
{
    m_Hue = c.m_Hue;
    m_Sat = c.m_Sat;
    m_Lum = c.m_Lum;
} // End copy constructor.


/////////////////////////////////////////////////////////////////////////////////
// SetFromRgb565()
//
// Sets the HSL values of the HSL object based on the RGB565 argument.
//
// Arguments:
//   rgb565 - This is the RGB565 value to be converted.
/////////////////////////////////////////////////////////////////////////////////
void HslColor::SetFromRgb565(uint16_t rgb565)
{
    float h = 0.0;
    float s = 0.0;
    float l = 0.0;

    // normalizes red-green-blue values
    float r = GetRed(rgb565)   / 255.f;
    float g = GetGreen(rgb565) / 255.f;
    float b = GetBlue(rgb565)  / 255.f;

    float maxVal = Max3(r, g, b);
    float minVal = Min3(r, g, b);

    // hue
    if (maxVal == minVal)
    {
        h = 0; // undefined
    }
    else if ((maxVal == r) && (g >= b))
    {
        h = 60.0f * (g - b) / (maxVal - minVal);
    }
    else if ((maxVal == r) && (g < b))
    {
        h = 60.0f * (g - b) / (maxVal - minVal) + 360.0f;
    }
    else if (maxVal == g)
    {
        h = 60.0f * (b - r) / (maxVal - minVal) + 120.0f;
    }
    else if (maxVal == b)
    {
        h = 60.0f * (r - g) / (maxVal - minVal) + 240.0f;
    }

    // luminance
    l = (maxVal + minVal) / 2.0f;

    // saturation
    if ((l == 0) || (maxVal == minVal))
    {
        s = 0;
    }
    else if ((0 < l) && (l <= 0.5f))
    {
        s = (maxVal - minVal) / (maxVal + minVal);
    }
    else if (l > 0.5f)
    {
        s = (maxVal - minVal) / (2 - (maxVal + minVal)); //(maxVal-minVal > 0)?
    }

    SetHue(h);
    SetSat(s * 100.0);
    SetLum(l * 100.0);
} // End SetFromRgb565.


// Converts HSL to RGB565
/////////////////////////////////////////////////////////////////////////////////
// ToRgb565()
//
// Returns an RGB565 value based on the current HSL values of the HSL object.
//
// Returns:
//   Always returns an RGB565 value corresponding to the current value of the
//   HSL object.
/////////////////////////////////////////////////////////////////////////////////
uint16_t HslColor::ToRgb565()
{
    float h = m_Hue;            // h must be [0, 360]
    float s = m_Sat / 100.f;    // s must be [0, 1]
    float l = m_Lum / 100.f;    // l must be [0, 1]
    float R;
    float G;
    float B;

    if (m_Sat == 0)
    {
        // achromatic color (gray scale)
        R = G = B = l * 255.f;
    }
    else
    {
        float q = (l < 0.5f) ? (l * (1.0f + s)) : (l + s - (l * s));
        float p = (2.0f * l) - q;

        float Hk = h / 360.0f;
        float T[3];
        T[0] = Hk + 0.3333333f; // Tr   0.3333333f = 1.0 / 3.0
        T[1] = Hk;              // Tb
        T[2] = Hk - 0.3333333f; // Tg

        for (int i = 0; i < 3; i++)
        {
            if (T[i] < 0)
            {
                T[i] += 1.0f;
            }
            else if (T[i] > 1)
            {
                T[i] -= 1.0f;
            }

            if ((T[i] * 6) < 1)
            {
                T[i] = p + ((q - p) * 6.0f * T[i]);
            }
            else if ((T[i] * 2.0f) < 1) //(1.0 / 6.0) <= T[i] && T[i] < 0.5
            {
                T[i] = q;
            }
            else if ((T[i] * 3.0f) < 2) // 0.5 <= T[i] && T[i] < (2.0 / 3.0)
            {
                T[i] = p + (q - p) * ((2.0f / 3.0f) - T[i]) * 6.0f;
            }
            else
            {
                T[i] = p;
            }
        }

        R = T[0] * 255.0f;
        G = T[1] * 255.0f;
        B = T[2] * 255.0f;
    }

    return MYRGB565(
        (uint8_t)((R > 255) ? 255 : ((R < 0) ? 0 : R)),
        (uint8_t)((G > 255) ? 255 : ((G < 0) ? 0 : G)),
        (uint8_t)((B > 255) ? 255 : ((B <0 ) ? 0 : B))
    );
} // End SetFromRgb565().


/////////////////////////////////////////////////////////////////////////////////
// Contrast()
//
// Returns an RGB565 value of either white or black based on the brightness of
// the specified RGB565 color.  This method is normally used to determine a
// contrasting value that will work as a label or other annotation against
// the color specified.
//
// Arguments:
//   rgb565 - This is the background color against which the corresponding
//            foreground color should be.
//
// Returns:
//   Always returns either RGB565 white (0xffff) or black (0) based on the
//   brightness of the input rgb565 value.
/////////////////////////////////////////////////////////////////////////////////
uint16_t HslColor::Contrast(uint16_t rgb565)
{
    float r = (float)GetRed(rgb565);
    float g = (float)GetGreen(rgb565);
    float b = (float)GetBlue(rgb565);

    float brightness =  (int)sqrt(r * r * .241 + g * g * .691 + b * b * .068);

    return brightness < 130 ? 0xffff : 0;
} // End Contrast().


/////////////////////////////////////////////////////////////////////////////////
// SetHue(), SetSat(), SetLum()
//
// These methods set the corresponding hue, saturation, or luminanace value of
// the HSL object.  Values are limited to appropriate ranges.
/////////////////////////////////////////////////////////////////////////////////
void HslColor::SetHue(float hue)
{
    m_Hue = hue;
    if (hue < 0.0)
    {
        m_Hue = 0.0;
    }
    else if (hue > 360.0)
    {
        m_Hue = 360.0;
    }
} // End SetHue().


void HslColor::SetSat(float sat)
{
    m_Sat = sat;
    if (sat < 0.0)
    {
        m_Sat = 0.0;
    }
    else if (sat > 100.0)
    {
        m_Sat = 100.0;
    }
} // End SetSat().


void HslColor::SetLum(float lum)
{
    m_Lum = lum;
    if (lum < 0.0)
    {
        m_Lum = 0.0;
    }
    else if (lum > 100.0)
    {
        m_Lum = 100.0;
    }
} // End SetLum().
