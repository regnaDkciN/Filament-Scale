/////////////////////////////////////////////////////////////////////////////////
// MainScreen.h
//
// This file supports the display of data on the main TFT screen.
//
// History:
// - jmcorbett 01-JUN-2021 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////
#if !defined MAINDISPLAY_H
#define MAINDISPLAY_H

#include <cstdint>      // For uint32_t, ...
#include <cstddef>      // For size_t.
#include <Arduino.h>    // For millis(), ...
#include "SCB.h"        // For SCB structure.


class MainScreen
{
public:
    /////////////////////////////////////////////////////////////////////////////
    // Init()
    //
    // This method initializes the spool collection.
    //
    // Arguments:
    //    - pName   - A string of no more than 15 characters to be used as a
    //                name for this instance.  This is mainly used to identify
    //                the instance to be used for NVS save and restore.
    // Returns:
    //    Returns a bool indicating whether or not the initialization was successful.
    //    A 'true' value indicates success, while a 'false' value indicates failure.
    //
    /////////////////////////////////////////////////////////////////////////////
    static bool Init(const char *pName);


    /////////////////////////////////////////////////////////////////////////////
    // DisplayMainScreen()
    //
    // Called by the main code to display all pertinent data to the TFT screen.
    //
    // Arguments:
    //    refresh   - Set to true if this is the first time to display data
    //                since a change has been made.  If false, only updates the
    //                header and main data within each displayed box.
    //    scrollDir - Scroll the scrollable area.  Valid values are:
    //                  -1 - Scroll backwards;
    //                   0 - Don't scroll;
    //                   1 - Scroll forwards.
    /////////////////////////////////////////////////////////////////////////////
    static void DisplayMainScreen(bool refresh, int32_t scrollDir);


    /////////////////////////////////////////////////////////////////////////////
    // Simple getters and setters.
    /////////////////////////////////////////////////////////////////////////////
    static bool     IsInitialized()    { return m_pName != NULL; }
    static uint32_t GetScrollDelayMs() { return m_ScrollDelayMs; }
    static void     SetScrollDelayMs(uint32_t d)
    {
        m_ScrollDelayMs =
            d <= 1000 * MAX_SCROLL_DELAY_SEC ? d : 1000 * MAX_SCROLL_DELAY_SEC;
    }

    /////////////////////////////////////////////////////////////////////////////
    // Save()
    //
    // Saves our current state to NVS.
    //
    // Returns:
    //    Returns 'true' if successful, or 'false' otherwise.
    /////////////////////////////////////////////////////////////////////////////
   static  bool Save();


    /////////////////////////////////////////////////////////////////////////////
    // Restore()
    //
    // Restores our state from NVS.
    //
    // Returns:
    //    Returns 'true' if successful, or 'false' otherwise.
    /////////////////////////////////////////////////////////////////////////////
    static bool Restore();


    /////////////////////////////////////////////////////////////////////////////
    // Reset()
    //
    // Reset our state info in NVS.
    //
    // Returns:
    //    Returns 'true' if successful, or 'false' otherwise.
    /////////////////////////////////////////////////////////////////////////////
    static bool Reset();


    static const uint32_t DEFAULT_SCROLL_DELAY_MS = 5000;
    static const uint32_t MAX_SCROLL_DELAY_SEC    = 120;
    static const uint32_t SCROLL_DELAY_STEP_SEC   = 5;
    static const size_t   MAX_NVS_NAME_LEN        = 15U;
    static const uint32_t SENTINAL                = 0xffff;
    static const char    *pPrefSavedStateLabel;

    // !!! SCB_TABLE_LENGTH must be the same value as the size of SCBs. !!!
    static const uint32_t SCB_TABLE_LENGTH = 17;

private:
    // Number of boxes plus 1 that may be displayed at one time on the main
    // display screen.  3 rows of 2 boxes each plus 1.
    static const size_t   BOX_TABLE_LENGTH  = 7;

    static const char *m_pName;                 // NVS storage name for this instance.
    static uint32_t m_ScrollDelayMs;            // Time in ms to delay before
                                                //    forceScroll the scrollable
                                                //    data.  Zero indicates only
                                                //    scroll when forced.
    static uint32_t m_Boxes[BOX_TABLE_LENGTH];  // Array of indices into SCBs
                                                //    representing boxes go be
                                                //    displayed.


    /////////////////////////////////////////////////////////////////////////////
    // Structure containing all of our state information that may be
    // saved/restored to/from NVS.
    /////////////////////////////////////////////////////////////////////////////
    struct SaveRestoreCache
    {
        uint32_t m_ScrollDelayMs;            // Scroll delay value.
        uint32_t m_Boxes[BOX_TABLE_LENGTH];  // Screen SCB table.
        SCB      m_Scbs[SCB_TABLE_LENGTH];   // SCB table to save.
    };



}; // End class MainScreen.


#endif // MAINDISPLAY_H