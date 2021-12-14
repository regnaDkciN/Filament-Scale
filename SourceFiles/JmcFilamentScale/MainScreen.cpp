/////////////////////////////////////////////////////////////////////////////////
// MainScreen.cpp
//
// Contains structures, data, and functions to handle the display of main
// scale data on the TFT display.
//
// History:
// - jmcorbett 01-JUN-2021 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////
#include "JmcFilamentScale.h"       // For main screen related data.
#include "MainScreen.h"             // For function prototypes.
#include "SCB.h"


/////////////////////////////////////////////////////////////////////////////////
// MainScreen static variables.
/////////////////////////////////////////////////////////////////////////////////
uint32_t MainScreen::m_ScrollDelayMs = DEFAULT_SCROLL_DELAY_MS;
                                            // Time in ms to delay before
                                            //    forceScroll the scrollable
                                            //    data.  Zero indicates only
                                            //    scroll when forced.
uint32_t MainScreen::m_Boxes[BOX_TABLE_LENGTH] = {0};
                                            // Array of pointers to SCBs to
                                            //    be displayed.
const char *MainScreen::m_pName = NULL;     // NVS storage name for this instance.
const char  *MainScreen::pPrefSavedStateLabel = "Saved State";


/////////////////////////////////////////////////////////////////////////////////
// SCBs
//
// This is an array containing one SCB per display method.  For this application,
// SCBs entry 0 (Net Weight) is always displayed in the first row (row 0).
// The second row (row 1) will always display SBCs entry 1 (Length) if a spool
// has been selected.  If not, rows 1 and 2 will scroll the reamining available
// data.
//
// Note that all non-full line boxes must occur in pairs.  For example, if an
// entry's side is specified as eLeft, the next entry's side must be eRight.
/////////////////////////////////////////////////////////////////////////////////
static SCB SCBs[MainScreen::SCB_TABLE_LENGTH] =
{
    // Must be first.
    {&SCB::NetWeightStrings, 0, eAll, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR},

    // Must be second.
    {&SCB::LengthStrings, 1, eAll, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR},

    {&SCB::GrossWeightStrings, 2, eAll, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR},

    {&SCB::SpoolIdStrings, 2, eAll, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR},

    {&SCB::SpoolWeightStrings, 2, eAll, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR},

    {&SCB::FilamentColorStrings, 2, eLeft, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR},

    {&SCB::FilamentTypeStrings, 2, eRight, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR},

    {&SCB::FilamentDensityStrings, 2, eLeft, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR},

    {&SCB::FilamentDiaStrings, 2, eRight, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR},

    {&SCB::NetworkNameStrings, 2, eAll, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR},

    {&SCB::IpAddrStrings, 2, eAll, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR},

    {&SCB::SignalStrengthStrings, 2, eAll, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR},

    {&SCB::ApNetworkNameStrings, 2, eAll, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR},

    {&SCB::ApIpAddrStrings, 2, eAll, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR},

    {&SCB::TemperatureStrings, 2, eLeft, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR},

    {&SCB::HumidityStrings, 2, eRight, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR},

    {&SCB::UptimeStrings, 2, eAll, MAIN_PAGE_FG_COLOR,
     MAIN_PAGE_FG_COLOR, MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, MAIN_PAGE_FG_COLOR}

}; // End SCBs.


/////////////////////////////////////////////////////////////////////////////////
// SelectDisplayData()
//
// Takes a pointer to an array of uint32_t, and fills it with pointers to SCBs
// to be displayed.  This function takes care of handling scrolling as well as
// displaying the fixed data fields.
//
// Arguments:
//      - boxes[] - Pointer to the array that will hold the list of SCBs indices
//                  that will be displayed.
//      - scroll  - Scroll the scrollable area.  Valid values are:
//                  -1 - Scroll backwards;
//                   0 - Don't scroll;
//                   1 - Scroll forwards.
/////////////////////////////////////////////////////////////////////////////////
static void SelectDisplayData(uint32_t boxes[], int32_t scroll = 0)
{
    // boxes is a 7 element array that contains 1 or 2 entries for each of the 3
    // display rows plus a sentry entry. Each entry is an index into the SCBs
    // representing the SB to display in the next screen location.  An entry
    // value greater than the SCBs size represents the end of the table..

    // For this application, SCBs entry 0 (Net Weight) is always displayed
    // in the first row (row 0).
    SCB *pScb = SCBs;
    int boxesIndex = 0;
    int displayRow = 0;
    pScb->m_Line = displayRow;
    boxes[boxesIndex++] = pScb - SCBs;
    pScb++;
    displayRow++;

    // For this application, the second row (row 1) will always display SCBs
    // entry 1 (Length) if a spool has been selected.  If not, rows 1 and 2 will
    // scroll the reamining available data.
    if (pScb->CallDisplayFunction(NULL, 0, eCheck))
    {
        pScb->m_Line = displayRow;
        boxes[boxesIndex++] = pScb - SCBs;
        pScb++;
        displayRow++;
    }

    // pWrapScb points to the first SCBs entry that is capable of scrolling.
    SCB *pWrapScb = pScb;

    // pEndOfScbs points just past the last entry of the SCBs table.
    const SCB *pEndOfScbs =  &SCBs[MainScreen::SCB_TABLE_LENGTH];

    // pLastScb points to the SCBs entry past the last one that scrolled.
    static SCB *pLastScb = SCBs;

    // Make sure that pLastScb doesn't point before the one or two stationary entries.
    if (pLastScb < pScb)
    {
        pLastScb = pScb;
    }

    // Skip any entries that don't want to be displayed now.
    while (!pLastScb->CallDisplayFunction(NULL, 0, eCheck))
    {
        if (++pLastScb >= pEndOfScbs)
        {
            pLastScb = pWrapScb;
        }
    }

    // Scroll the display if commanded to do so.
    if (scroll)
    {
        // Handle positive scroll.
        if (scroll > 0)
        {
            // Increment pLastScb twice if it was pointing to a half line box.
            // Otherwise increment only once.
            if (pLastScb->m_Side != eAll)
            {
                pLastScb++;
            }
            // Wrap if pLastScb has gone past the end of SCBs.
            if (++pLastScb >= pEndOfScbs)
            {
                pLastScb = pWrapScb;
            }
            // Skip any entries that don't want to be displayed now.
            while (!pLastScb->CallDisplayFunction(NULL, 0, eCheck))
            {
                if (++pLastScb >= pEndOfScbs)
                {
                    pLastScb = pWrapScb;
                }
            }

        }
        else  // Scroll negative.
        {
            // Decrement pLastScb twice if the previous box was half line.
            // Otherwise decrement only once.
            do
            {
                pLastScb--;
                if (pLastScb < pWrapScb)
                {
                    pLastScb = &SCBs[MainScreen::SCB_TABLE_LENGTH - 1];
                }
                // Wrap if pLastScb has gotten before the first wrappable SCB.
                if (pLastScb->m_Side != eAll)
                {
                    pLastScb--;
                }
            } while (!pLastScb->CallDisplayFunction(NULL, 0, eCheck));
        }
    }

    // Start by pointing to the next scrolling candidate.
    pScb = pLastScb;

    // Loop till all 3 rows are full.
    while (displayRow < 3)
    {
        // Skip any entries that don't want to be displayed now.
        while (!pScb->CallDisplayFunction(NULL, 0, eCheck))
        {
            if (++pScb >= pEndOfScbs)
            {
                pScb = pWrapScb;
            }
        }

        // We're now pointing to an entry that can be displayed.  Update its row,
        // and save it in our boxes table.
        pScb->m_Line = displayRow;
        boxes[boxesIndex++] = pScb - SCBs;

        // If this entry is only half a line long, then add its neighbor to the
        // boxes table since half line entries always occur in pairs.
        if (pScb->m_Side != eAll)
        {
            pScb++;
            pScb->m_Line = displayRow;
            boxes[boxesIndex++] = pScb - SCBs;
        }

        // Bump and wrap the pScb pointer.
        if (++pScb >= pEndOfScbs)
        {
            pScb = pWrapScb;
        }
        displayRow++;
    }
    // Terminate the table with a sentry value.
    boxes[boxesIndex] = MainScreen::SENTINAL;
} // End SelectDisplayData().


/////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////
bool MainScreen::Init(const char *pName)
{
    bool status = false;

    if (!IsInitialized() && (pName != NULL) &&
        (*pName != '\0') && (strlen(pName) <= MAX_NVS_NAME_LEN))
    {
        m_pName         = pName;
        m_ScrollDelayMs = DEFAULT_SCROLL_DELAY_MS;
        m_Boxes[0]       = SENTINAL;
        status          = true;
    }
    return status;
} // End Init().


/////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////
void MainScreen::DisplayMainScreen(bool refresh, int32_t scrollDir)
{
    // buf holds any sttrings (header or main) that will be displayed.
    char buf[MAX_STRING_LENGTH * 2 + 1];

    // Last time display was scrolled.
    static uint32_t lastScrollTimeMs = millis() - DEFAULT_SCROLL_DELAY_MS;
    uint32_t currentTime = millis();

    // forceScroll being 'true' or a scroll timeout indicates that it is time to
    // scroll the display.  So we need to update our SCB table.
    bool timeout = (m_ScrollDelayMs &&
                   ((currentTime - lastScrollTimeMs) >= m_ScrollDelayMs));
    int32_t scrollVal = scrollDir;

    // If firstTime is true, then we use scrollDir for direction of scroll.
    // Otherwise we scroll positive only if a timeout occurred.
    if (!refresh)
    {
        scrollVal = timeout ? 1 : 0;
    }

    // If this is the first time or if we timed out, then update our display table.
    if (refresh || timeout)
    {
        // Update our SCB table.
        SelectDisplayData(m_Boxes, scrollVal);
        lastScrollTimeMs = currentTime;

        // Clear the background since the entire screen will now be updated.
        gTft.setTextColor(MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR);
    }

    // Update the display with fresh header and main data.
    int index = 0;
    SCB *pScb = &SCBs[m_Boxes[index]];
    while ((index < BOX_TABLE_LENGTH) && (m_Boxes[index] <= SCB_TABLE_LENGTH))
    {
        // If firstTime or timeout, then we need to update the box background as well.
        if (refresh || timeout)
        {
            pScb->DisplayABox(eBox);
        }
        pScb->DisplayABox(eHeader);
        pScb->DisplayABox(eMain);
        index++;
        pScb = &SCBs[m_Boxes[index]];
    }
} // End DisplayMainScreen().


/////////////////////////////////////////////////////////////////////////////////
// Save()
//
// Saves our current state to NVS.
//
// Returns:
//    Returns 'true' if successful, or 'false' otherwise.
/////////////////////////////////////////////////////////////////////////////////
bool MainScreen::Save()
{
    size_t saved = 0;
    if (m_pName != NULL)
    {
        SaveRestoreCache cache;
        cache.m_ScrollDelayMs = m_ScrollDelayMs;
        memcpy(cache.m_Boxes, m_Boxes, sizeof(cache.m_Boxes));
        memcpy(cache.m_Scbs, SCBs, sizeof(cache.m_Scbs));

        Preferences prefs;
        prefs.begin(m_pName);

        SaveRestoreCache nvsState;
        size_t nvsSize =
            prefs.getBytes(pPrefSavedStateLabel, &nvsState, sizeof(nvsState));

        // See if our working data has changed since our last save.
        if ((nvsSize != sizeof(cache)) ||
             memcmp(&nvsState, &cache, sizeof(cache)))
        {
            // Data has changed so go ahead and save it.
            Serial.println("\nMainScreen - saving to NVS.");
            saved =
                prefs.putBytes(pPrefSavedStateLabel, &cache, sizeof(cache));
        }
        else
        {
            // Data has not changed.  Do nothing.
            saved = sizeof(cache);
            Serial.println("\nMainScreen - not saving to NVS.");
        }
        prefs.end();
    }

    // Let the caller know if we succeeded or failed.
    return saved == sizeof(SaveRestoreCache);
} // End Save().


/////////////////////////////////////////////////////////////////////////////////
// Restore()
//
// Restores our state from NVS.
//
// Returns:
//    Returns 'true' if successful, or 'false' otherwise.
/////////////////////////////////////////////////////////////////////////////////
bool MainScreen::Restore()
{
    bool succeeded = false;

    // Make sure we have a valid name.
    if (m_pName != NULL)
    {
        SaveRestoreCache cache;
        Preferences prefs;
        prefs.begin(m_pName);
        size_t   nvsSize =
            prefs.getBytes(pPrefSavedStateLabel, &cache, sizeof(cache));

        // Save the restored values only if the get was successful.
        if (nvsSize == sizeof(cache))
        {
            // Restore our scroll delay value.
            m_ScrollDelayMs = cache.m_ScrollDelayMs;

            // Restore our boxes array.
            memcpy(m_Boxes, cache.m_Boxes, sizeof(m_Boxes));

            // Restore our SCBs data being careful to not overwrite pointers.
            for (uint32_t i = 0; i < SCB_TABLE_LENGTH; i++)
            {
                SCBs[i].m_Line           = cache.m_Scbs[i].m_Line;
                SCBs[i].m_Side           = cache.m_Scbs[i].m_Side;
                SCBs[i].m_OutlineFgColor = cache.m_Scbs[i].m_OutlineFgColor;
                SCBs[i].m_HeaderFgColor  = cache.m_Scbs[i].m_HeaderFgColor;
                SCBs[i].m_MainFgColor    = cache.m_Scbs[i].m_MainFgColor;
                SCBs[i].m_BgColor        = cache.m_Scbs[i].m_BgColor;
                SCBs[i].m_LastBgColor    = cache.m_Scbs[i].m_LastBgColor;
            }
            succeeded = true;
        }
        prefs.end();
    }

    // Let the caller know if we succeeded or failed.
    return succeeded;
} // End Restore().


/////////////////////////////////////////////////////////////////////////////
// Reset()
//
// Reset our state info in NVS.
//
// Returns:
//    Returns 'true' if successful, or 'false' otherwise.
/////////////////////////////////////////////////////////////////////////////
bool MainScreen::Reset()
{
    bool status = false;
    if (m_pName != NULL)
    {
        // Remove our state data rom NVS.
        Preferences prefs;
        prefs.begin(m_pName);
        status = prefs.remove(pPrefSavedStateLabel);
        prefs.end();
    }
    return status;
} // End Reset().

