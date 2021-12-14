/////////////////////////////////////////////////////////////////////////////////
// SCB.h
//
// Contains the SCB class which acts as a control block for each field on the
// main scale screen.
//
// History:
// - jmcorbett 07-JUN-2021 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////
#if !defined SCB_H
#define SCB_H


#include <cstdint>      // For uint32_t, ...
#include <cstddef>      // For size_t.



/////////////////////////////////////////////////////////////////////////////////
// WhatToDisplay
//
// This enum is used to specify what, if anything, should be displayed.
/////////////////////////////////////////////////////////////////////////////////
enum WhatToDisplay
{
    eCheck  = 0,    // Don't display anything, just check if this value should be displayed.
    eBox    = 1,    // Display the background box.
    eHeader = 2,    // Display the header.
    eMain   = 3     // Display the main data.
}; // End WhatToDisplay.


/////////////////////////////////////////////////////////////////////////////////
// Create a type describing a display function.
/////////////////////////////////////////////////////////////////////////////////
struct SCB;
using dispFunc_t = bool (SCB::*)(char *, size_t, int);


/////////////////////////////////////////////////////////////////////////////////
// SCB (Screen Control Block)
//
// This struct is basically a control block for the display of data on the
// main screen.  A struct is used instead of a class since everything contained
// in it needs to be public.
/////////////////////////////////////////////////////////////////////////////////
struct SCB
{
    /////////////////////////////////////////////////////////////////////////////
    // DisplayABox()
    //
    // Displays a portion of a box as specified by its only argument.
    //
    // Arguments:
    //      what - Specifies what portion of the box, if any, is to be displayed.
    /////////////////////////////////////////////////////////////////////////////
    void DisplayABox(WhatToDisplay what);


    /////////////////////////////////////////////////////////////////////////////
    // CallDisplayFunction()
    //
    // Calls the display method that is associated with this instance of the
    // class.
    //
    // Arguments:
    //      pBuf    - Pointer to the buffer that will contain the string returned
    //                by the function.
    //      bufSize - Size of pBuf.
    //      what    - Which data, if any, to display.
    /////////////////////////////////////////////////////////////////////////////
    bool CallDisplayFunction(char *pBuf, size_t bufSize, int what)
    {
        return (this->*m_pFunc)(pBuf, bufSize, what);
    }


    /////////////////////////////////////////////////////////////////////////////
    // Display Methods:
    //
    // The following methods are specialized to generate the strings to be
    // displayed in specific data boxes.
    //
    // Arguments:
    //      pBuf    - Pointer to the buffer that will contain the string returned
    //                by the function.
    //      bufSize - Size of pBuf.
    //      what    - Which data, if any, to display.
    //
    // Returns:
    //      Returns 'true' if the data is available to be displayed.  Returns
    //      'false' if the data should not be displayed in the current state.
    /////////////////////////////////////////////////////////////////////////////
    bool NetWeightStrings(char *pBuf, size_t bufSize, int what);
    bool LengthStrings(char *pBuf, size_t bufSize, int what);
    bool GrossWeightStrings(char *pBuf, size_t bufSize, int what);
    bool TemperatureStrings(char *pBuf, size_t bufSize, int what);
    bool HumidityStrings(char *pBuf, size_t bufSize, int what);
    bool SpoolIdStrings(char *pBuf, size_t bufSize, int what);
    bool SpoolWeightStrings(char *pBuf, size_t bufSize, int what);
    bool FilamentTypeStrings(char *pBuf, size_t bufSize, int what);
    bool FilamentDiaStrings(char *pBuf, size_t bufSize, int what);
    bool FilamentDensityStrings(char *pBuf, size_t bufSize, int what);
    bool FilamentColorStrings(char *pBuf, size_t bufSize, int what);
    bool UptimeStrings(char *pBuf, size_t bufSize, int what);
    bool NetworkNameStrings(char *pBuf, size_t bufSize, int what);
    bool IpAddrStrings(char *pBuf, size_t bufSize, int what);
    bool SignalStrengthStrings(char *pBuf, size_t bufSize, int what);
    bool ApNetworkNameStrings(char *pBuf, size_t bufSize, int what);
    bool ApIpAddrStrings(char *pBuf, size_t bufSize, int what);


    /////////////////////////////////////////////////////////////////////////////
    // Instance data .
    /////////////////////////////////////////////////////////////////////////////

    dispFunc_t  m_pFunc;            // Pointer to string generating method.
    int         m_Line;             // Display line number (0, 1, or 2).
    BoxLocale   m_Side;             // Side of row to display the data.
    uint16_t    m_OutlineFgColor;   // Background outline color.
    uint16_t    m_HeaderFgColor;    // Header display color.
    uint16_t    m_MainFgColor;      // Main data display color.
    uint16_t    m_BgColor;          // Background box color.
    uint16_t    m_LastBgColor;      // Color of background when last displayed.


}; // End SCB.

#endif // SCB_H