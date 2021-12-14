/////////////////////////////////////////////////////////////////////////////////
// Filament.cpp
//
// Contains methods defined by the Filament class.  These methods
// manage filament density and id strings.
//
// History:
// - jmcorbett 12-DEC-2020 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#include <Preferences.h>
#include "Filament.h"


/////////////////////////////////////////////////////////////////////////////////
// Filament type strings array.
//
// NOTE:
//    This is a kludge that was necessitated by the use of the filament type
//    strings in the ArduinoMenu system.  I could not find a reasonable way to
//    allow pointers to the strings to be used in the TOGGLE menu macro.
/////////////////////////////////////////////////////////////////////////////////
const char * const Filament::TypeStrings[NUMBER_FILAMENTS] =
{
    FILAMENT_STRING_ABS,   FILAMENT_STRING_ASA,   FILAMENT_STRING_COPPER,
    FILAMENT_STRING_HIPS,  FILAMENT_STRING_NYLON, FILAMENT_STRING_PETG,
    FILAMENT_STRING_PLA,   FILAMENT_STRING_PMMA,  FILAMENT_STRING_POLYC,
    FILAMENT_STRING_PVA,   FILAMENT_STRING_TPE,   FILAMENT_STRING_TPU,
    FILAMENT_STRING_USER1, FILAMENT_STRING_USER2, FILAMENT_STRING_USER3
}; // End TypeStrings[].


const char * const Filament::TypeLStrings[NUMBER_FILAMENTS] =
{
    FILAMENT_LSTRING_ABS,   FILAMENT_LSTRING_ASA,   FILAMENT_LSTRING_COPPER,
    FILAMENT_LSTRING_HIPS,  FILAMENT_LSTRING_NYLON, FILAMENT_LSTRING_PETG,
    FILAMENT_LSTRING_PLA,   FILAMENT_LSTRING_PMMA,  FILAMENT_LSTRING_POLYC,
    FILAMENT_LSTRING_PVA,   FILAMENT_LSTRING_TPE,   FILAMENT_LSTRING_TPU,
    FILAMENT_LSTRING_USER1, FILAMENT_LSTRING_USER2, FILAMENT_LSTRING_USER3
}; // End TypeLStrings[].


/////////////////////////////////////////////////////////////////////////////////
// The density table.  Each entry corresponds to a filament type.  Values
// for the table were taken from an article in Nuts/Volts magazine,
// 2019/issue-4, entitled "Build a 3D Printer Filament Scale".
/////////////////////////////////////////////////////////////////////////////////
float Filament::m_Densities[NUMBER_FILAMENTS] =
{
//  ABS   ASA   Copr  HIPS  Nyln  PETG  PLA   PMMA
    1.04, 1.07, 3.90, 1.07, 1.08, 1.27, 1.24, 1.18,
//  PlyC  PVA   TPE   TPU   USR1  USR2  USR3
    1.20, 1.19, 1.20, 1.20, 1.24, 1.24, 1.24
}; // End m_Densities[].


// Some constants used by the class.
const size_t Filament::MAX_NVS_NAME_LEN     = 15U;
const char  *Filament::pPrefSavedStateLabel = "Saved State";
const float  Filament::MAX_DENSITY          = 5.0;
const float  Filament::MIN_DENSITY          = 0.01;


/////////////////////////////////////////////////////////////////////////////
// Init()
//
// This method initializes the filament class.
//
// Arguments:
//    - pName   - A string of no more than 15 characters to be used as a
//                name for this instance.  This is mainly used to identify
//                the instance to be used for NVS save and restore.
//
// Returns:
//    Returns a bool indicating whether or not the initialization was
//    successful.  A 'true' value indicates success, while a 'false' value
//    indicates failure.
//
/////////////////////////////////////////////////////////////////////////////
bool Filament::Init(const char *pName)
{
    // Assume we're gonna fail.
    bool status = false;

    // Make sure the name is valid.  If not then we fail.
    if ((pName != NULL) && (*pName != '\0') && (strlen(pName) <= MAX_NVS_NAME_LEN))
    {
        // Name was OK.  Save it and remember that we succeeded.
        m_pName = pName;
        status = true;
    }
    return status;
} // End Init().


/////////////////////////////////////////////////////////////////////////////////
// Save()
//
// Saves our current state to NVS.
//
// Returns:
//    Returns 'true' if successful, or 'false' otherwise.
/////////////////////////////////////////////////////////////////////////////////
bool Filament::Save() const
{
    size_t saved = 0;
    if (m_pName != NULL)
    {
        // Read our currently saved state.  If it hasn't changed, then don't
        // bother to do the save in order to conserve writes to NVS.
        float nvsState[NUMBER_FILAMENTS];

        // Save our state data.
        Preferences prefs;
        prefs.begin(m_pName);

        // Get the saved data.
        size_t nvsSize =
            prefs.getBytes(pPrefSavedStateLabel, &nvsState, sizeof(m_Densities));

        // See if our working data has changed since our last save.
        if ((nvsSize != sizeof(m_Densities)) ||
            memcmp(nvsState, m_Densities, sizeof(m_Densities)))
        {
            // Data has changed so go ahead and save it.
            Serial.println("\nFilament - saving to NVS.");
            saved =
                prefs.putBytes(pPrefSavedStateLabel, m_Densities, sizeof(m_Densities));
        }
        else
        {
            // Data has not changed.  Do nothing.
            saved = sizeof(m_Densities);
            Serial.println("\nFilament - not saving to NVS.");
        }
        prefs.end();
    }

    // Let the caller know if we succeeded or failed.
    return saved == sizeof(m_Densities);
 } // End Save().


/////////////////////////////////////////////////////////////////////////////////
// Restore()
//
// Restores our state from NVS.
//
// Returns:
//    Returns 'true' if successful, or 'false' otherwise.
/////////////////////////////////////////////////////////////////////////////////
bool Filament::Restore()
{
    // Assume we're gonna fail.
    bool succeeded = false;

    // Make sure we have a valid name.
    if (m_pName != NULL)
    {
        // Restore our state data to a temporary structure.
        float cachedState[NUMBER_FILAMENTS];
        Preferences prefs;
        prefs.begin(m_pName);
        size_t restored =
            prefs.getBytes(pPrefSavedStateLabel, &cachedState, sizeof(cachedState));

        // Save the restored values only if the get was successful.
        if (restored == sizeof(cachedState))
        {
            for (size_t i = 0; i < NUMBER_FILAMENTS; i++)
            {
                m_Densities[i] = cachedState[i];
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
bool Filament::Reset()
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


