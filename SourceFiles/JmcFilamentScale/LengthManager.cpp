/////////////////////////////////////////////////////////////////////////////////
// LengthManager.cpp
//
// Contains methods defined by the LengthManager class.  These methods
// manage filament length calaulations.
//
// History:
// - jmcorbett 21-DEC-2020 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#include <Preferences.h>
#include "LengthManager.h"


/////////////////////////////////////////////////////////////////////////////////
// LengthManager type strings array.
//
// NOTE:
//    This is a kludge that was necessitated by the use of the length strings
//    in the ArduinoMenu system.  I could not find a reasonable way to
//    allow pointers to the strings to be used in the TOGGLE menu macro.
/////////////////////////////////////////////////////////////////////////////////
const char * const LengthManager::UnitStrings[NUMBER_UNITS] =
{
    UNIT_STRING_MM, UNIT_STRING_CM, UNIT_STRING_M,
    UNIT_STRING_IN, UNIT_STRING_FT, UNIT_STRING_YD
};


/////////////////////////////////////////////////////////////////////////////////
// LengthManager precision array.
//
// This array contains the number of digits to the right of the decimal point
// to be displayed when the associated length units are active.  For example,
// if the current length units is selected as yards, then 3 digits to the
// right of the decimal point will be displayed.
/////////////////////////////////////////////////////////////////////////////////
const uint16_t LengthManager::UnitPrecicions[NUMBER_UNITS] =
{
 // mm  cm  m   in  ft  yd
    0U, 1U, 3U, 2U, 3U, 3U
};


// Some constants used by the class.
static const size_t MAX_NVS_NAME_LEN = 15U;
const char *LengthManager::pPrefSavedStateLabel  = "Saved State";
const float LengthManager::MM_PER_MM = 1.0;
const float LengthManager::CM_PER_MM = 1.0 / 10.0;
const float LengthManager::M_PER_MM  = 1.0 / 1000.0;
const float LengthManager::IN_PER_MM = 1.0 / 25.4;
const float LengthManager::FT_PER_MM = IN_PER_MM / 12.0;
const float LengthManager::YD_PER_MM = FT_PER_MM / 3.0;


/////////////////////////////////////////////////////////////////////////////
// Init()
//
// This method initializes the filament class.
//
// Arguments:
//    - pName   - A string of no more than 14 characters to be used as a
//                name for this instance.  This is mainly used to identify
//                the instance to be used for NVS save and restore.
//
// Returns:
//    Returns a bool indicating whether or not the initialization was
//    successful.  A 'true' value indicates success, while a 'false' value
//    indicates failure.
//
/////////////////////////////////////////////////////////////////////////////
bool LengthManager::Init(const char *pName)
{
    // Assume we're gonna fail.
    bool status = false;

    // Make sure the name is valid.  If not then we fail.
    if ((pName != NULL) && (*pName != '\0') &&
        (strlen(pName) <= MAX_NVS_NAME_LEN))
    {
        // Name was OK.  Save it and remember that we succeeded.
        m_pName = pName;
        status = true;
    }
    return status;
} // End Init().


/////////////////////////////////////////////////////////////////////////////
// GetUnitsString()
//
// This method returns a pointer to a string representing the specified
// units type (mm, cm, ...).
//
// Arguments:
//    - type - The type of units whose string is desired.
//
// Returns:
//    Returns a pointer to the string corresponding to the specified units
//    type.  If an invalid type is specified, then millimeters is used.
//
/////////////////////////////////////////////////////////////////////////////
const char *LengthManager::GetUnitsString(LengthUnits type)
{
    const char *pString = UnitStrings[luMm];
    switch(type)
    {
    case luCm : pString = UnitStrings[luCm]; break;
    case luM  : pString = UnitStrings[luM];  break;
    case luIn : pString = UnitStrings[luIn]; break;
    case luFt : pString = UnitStrings[luFt]; break;
    case luYd : pString = UnitStrings[luYd]; break;
    case luMm :
    default   : pString = UnitStrings[luMm]; break;
    }
    return pString;
} // End GetUnitsString().


/////////////////////////////////////////////////////////////////////////////
// GetPrecision()
//
// This method returns the precision to be used for the specified units type.
// The precision is the number of decimal places to the right of the decimal
// point that should be displayed for the corresponding type.
//
// Arguments:
//    - type - The type of units whose precision is desired.
//
// Returns:
//    Returns a precision corresponding to the specified units
//    type.  If an invalid type is specified, then millimeters is used.
//
/////////////////////////////////////////////////////////////////////////////
uint16_t LengthManager::GetPrecision(LengthUnits type)
{
    uint16_t precision = UnitPrecicions[luMm];
    switch(type)
    {
    case luCm : precision = UnitPrecicions[luCm]; break;
    case luM  : precision = UnitPrecicions[luM];  break;
    case luIn : precision = UnitPrecicions[luIn]; break;
    case luFt : precision = UnitPrecicions[luFt]; break;
    case luYd : precision = UnitPrecicions[luYd]; break;
    case luMm :
    default   : precision = UnitPrecicions[luMm]; break;
    }
    return precision;
} // End GetPrecision().


/////////////////////////////////////////////////////////////////////////////////
// SetUnits()
//
// This method selects the units that are currently active.
//
// Arguments:
//    - type - The type of unitsto select.
//
// Returns:
//    Returns 'true' if the selected type is valid.  Returns 'false' otherwise.
//
/////////////////////////////////////////////////////////////////////////////////
bool LengthManager::SetUnits(LengthUnits type)
{
    // Assume failure.
    bool status = false;
    Serial.printf("\nSetLengthUnits: %d\n", type);

    // Validate the type argument.
    if (type < luNum)
    {
        // Type is valid, save it as new selected units.
        m_SelectedUnits = type;
        status = true;
    }

    // Return status.
    return status;
} // End SetUnits().


/////////////////////////////////////////////////////////////////////////////////
// GetUnitsFactor()
//
// These methods return the factor that will be used to convert millimeters
// to another specified length units.  There are two methods here.  The first
// one (which uses a type argument) will return the conversion factor
// corresponding to the specified type.  The second one (which uses no
// arguments) will return the conversion factor corresponding to the
// currently selected units typa.
//
// Arguments:
//    - type - The type of units whose conversion factor is desired.
//
// Returns:
//    Returns a conversion factor corresponding to the specified units
//    type.  If an invalid type is specified, then millimeters is used.
//
/////////////////////////////////////////////////////////////////////////////////
float LengthManager::GetUnitsFactor(uint16_t units)
{
    float factor = MM_PER_MM;
    switch(units)
    {
    case luCm : factor = CM_PER_MM; break;
    case luM  : factor = M_PER_MM;  break;
    case luIn : factor = IN_PER_MM; break;
    case luFt : factor = FT_PER_MM; break;
    case luYd : factor = YD_PER_MM; break;
    case luMm :
    default   : factor = MM_PER_MM; break;
    }
    return factor;
} // End GetUnitsFactor().


/////////////////////////////////////////////////////////////////////////////////
// CalculateLengthFactor()
//
// This method calculates a factor that may be used to convert a filament
// weight to the corresponding length in the currently selected length units.
//
// Arguments:
//    - filamentDiameterMM
//      This is the diameter in millimeters of the filament being weighed.
//    - weightFactor
//      This is a factor that is used to convert the weight in current weight
//      units to the corresponding wieght in grams.
//    - filamentDensity
//      This is the density of the filament being weighed in grams per cubic
//      centimeter.
//
// Returns:
//    Returns the length factor corresponding to the specified parameters.
//
/////////////////////////////////////////////////////////////////////////////////
float LengthManager::CalculateLengthFactor(float filamentDiameterMM,
                                           float weightFactor,
                                           float filamentDensity) const
{
    // Filament diameter is in mm.  Need to convert to radius in cm, so divide
    // by 20 instead of by 2.
    float filamentRadiusCm = filamentDiameterMM / 20.0;
    // Serial.print("\nDiameter cm: "); Serial.println(filamentRadiusCm);

    // Calculate the filament's cross sectional area.
    float filamentCrossSectionalArea = M_PI * filamentRadiusCm * filamentRadiusCm;
    // Serial.print("CSA cm^2: "); Serial.println(filamentCrossSectionalArea);

    // Get the factor to convert the resultant mm length to the currently active units.
    float lengthCorrectionFactor = GetUnitsFactor();
    // Serial.print("Length Correction Factor: "); Serial.println(lengthCorrectionFactor);

    // Save the filament length multiplier which is the inverse of the cross
    // sectional area.  This makes it easy to calculate the length of the remaining
    // filament by simply multiplying its weight in grams by the length factor.
    float result = 10.0 / (filamentDensity * filamentCrossSectionalArea);
    result *= (weightFactor * lengthCorrectionFactor);
    // Serial.printf("Length Factor: %f\n", result);

    return result;
} // End CalculateLengthFactor().


/////////////////////////////////////////////////////////////////////////////////
// Save()
//
// Saves our current state to NVS.
//
// Returns:
//    Returns 'true' if successful, or 'false' otherwise.
/////////////////////////////////////////////////////////////////////////////////
bool LengthManager::Save() const
{
    size_t saved = 0;
    if (m_pName != NULL)
    {
        // Create a temporary value to save the NVS value.
        LengthUnits nvsSelectedUnits;

        // Save our state data.
        Preferences prefs;
        prefs.begin(m_pName);

        // Get the saved data.
        size_t nvsSize =
            prefs.getBytes(pPrefSavedStateLabel, &nvsSelectedUnits, sizeof(LengthUnits));

        // Read our currently saved state.  If it hasn't changed, then don't
        // bother to do the save in order to conserve writes to NVS.
        if ((nvsSize != sizeof(LengthUnits)) ||
            memcmp(&nvsSelectedUnits, &m_SelectedUnits, sizeof(LengthUnits)))
        {
            // Data has changed so go ahead and save it.
            Serial.println("\nLengthManager - saving to NVS.");
            saved =
                prefs.putBytes(pPrefSavedStateLabel, &m_SelectedUnits, sizeof(LengthUnits));
        }
        else
        {
            // Data has not changed.  Do nothing.
            saved = sizeof(LengthUnits);
            Serial.println("\nLengthManager - not saving to NVS.");
        }
        prefs.end();
    }

    // Let the caller know if we succeeded or failed.
    return saved == sizeof(LengthUnits);
 } // End Save().


/////////////////////////////////////////////////////////////////////////////////
// Restore()
//
// Restores our state from NVS.
//
// Returns:
//    Returns 'true' if successful, or 'false' otherwise.
/////////////////////////////////////////////////////////////////////////////////
bool LengthManager::Restore()
{
    // Assume we're gonna fail.
    bool succeeded = false;

    // Make sure we have a valid name.
    if (m_pName != NULL)
    {
        // Restore our state data to a temporary variable.
        LengthUnits tempSelectedUnits;
        Preferences prefs;
        prefs.begin(m_pName);
        size_t restored =
            prefs.getBytes(pPrefSavedStateLabel, &tempSelectedUnits, sizeof(LengthUnits));

        // Save the restored values only if the get was successful.
        if (restored == sizeof(LengthUnits))
        {
            m_SelectedUnits = tempSelectedUnits;
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
bool LengthManager::Reset()
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


