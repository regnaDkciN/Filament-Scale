/////////////////////////////////////////////////////////////////////////////////
// EnvSensor.cpp
//
// Contains methods defined by the EnvSensor class.  These methods
// manage the DHT22 Temperature and Humidity sensor.
//
// History:
// - jmcorbett 01-DEC-2020 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#include "EnvSensor.h"       // For EnvSensor class.
#include <Preferences.h>     // For Save and Restore to/from NVS.


// Setup our scale (C/F) strings.
#define DEGREE_SYMBOL "\xF7"
const char *EnvSensor::TempScaleStrings[] =
    { DEGREE_SYMBOL "F", DEGREE_SYMBOL "C" };

const char *EnvSensor::pPrefScaleLabel = "TempScale";

static const size_t MAX_NVS_NAME_LEN = 15U;



/////////////////////////////////////////////////////////////////////////////
// Init()
//
// Initializes the environmental sensor and returns a status indicating
// whether or not a sensor was found.
//
// Arguments:
//    - pName   - A string of no more than 15 characters to be used as a
//                name for this instance.  This is mainly used to identify
//                the instance to be used for NVS save and restore.
//
// Returns:
//     Returns 'true' if a sensor was found, and false otherwise.
/////////////////////////////////////////////////////////////////////////////
bool EnvSensor::Init(const char *pName)
{
    bool status = false;

    if ((pName != NULL) && (*pName != '\0') && (strlen(pName) <= MAX_NVS_NAME_LEN))
    {
        begin();
        m_IsPresent = !isnan(GetDegreesF());
        status = true;
        m_pName = pName;
    }
    return status && m_IsPresent;
} // End Init().


/////////////////////////////////////////////////////////////////////////////////
// SetTempScale()
//
// Sets the temperature scale to degrees F or degrees C.
//
// Arguments:
//    - units - This specifies the temperature units to be used for reporting
//              temperature (F or C).
//
// Returns:
//    Returns 'true' if successful, or 'false' otherwise.
/////////////////////////////////////////////////////////////////////////////////
bool EnvSensor::SetTempScale(TempScale scale)
{
    bool succeeded = false;

    // Verify that a valid scale type has been specified.
    if ((scale == eTempScaleF) || (scale == eTempScaleC))
    {
        // Only save the scale type if it is valid.
        m_TempScale = scale;
        succeeded = true;
    }

    // Let the caller know if we succeeded or failed.
    return succeeded;
} // End SetTempScale().


/////////////////////////////////////////////////////////////////////////////////
// GetTemperature()
//
// Returns the temperature based on the currently selected scale (F or C).
//
// Returns:
//    Returns current temperature in degrees C or degrees F based on the current
//    setting of m_TempScale.  Could return NAN if it has trouble reading the
//    DHT22 sensor.
/////////////////////////////////////////////////////////////////////////////////
float EnvSensor::GetTemperature()
{
    float temp = 0.0;

    // Get the temperature based on the selected temperature scale.
    if (m_TempScale == eTempScaleF)
    {
        temp = GetDegreesF();
    }
    else
    {
        temp = GetDegreesC();
    }

    return temp;
} // End GetTemperature().


/////////////////////////////////////////////////////////////////////////////////
// Save()
//
// Saves our current state to NVS.
//
// Returns:
//    Returns 'true' if successful, or 'false' otherwise.
/////////////////////////////////////////////////////////////////////////////////
bool EnvSensor::Save() const
{
    size_t saved = 0;
    if (m_pName != NULL)
    {
        Preferences prefs;
        prefs.begin(m_pName);
        uint32_t nvsValue = 0;
        uint32_t currentValue = static_cast<uint32_t>(m_TempScale);
        size_t nvsSize =
            prefs.getBytes(pPrefScaleLabel, &nvsValue, sizeof(uint32_t));

        // See if our working data has changed since our last save.
        if ((nvsSize != sizeof(uint32_t)) || (nvsValue != currentValue))
        {
            // Data has changed so go ahead and save it.
            Serial.println("\nEnvSensor - saving to NVS.");
            saved =
                prefs.putBytes(pPrefScaleLabel, &currentValue, sizeof(uint32_t));
        }
        else
        {
            // Data has not changed.  Do nothing.
            saved = sizeof(uint32_t);
            Serial.println("\nEnvSensor - not saving to NVS.");
        }
        prefs.end();
    }

    // Let the caller know if we succeeded or failed.
    return saved == sizeof(uint32_t);
} // End Save().


/////////////////////////////////////////////////////////////////////////////////
// Restore()
//
// Restores our state from NVS.
//
// Returns:
//    Returns 'true' if successful, or 'false' otherwise.
/////////////////////////////////////////////////////////////////////////////////
bool EnvSensor::Restore()
{
    bool succeeded = false;

    // Make sure we have a valid name.
    if (m_pName != NULL)
    {
        Preferences prefs;
        prefs.begin(m_pName);
        uint32_t nvsValue = 0;
        size_t   nvsSize =
            prefs.getBytes(pPrefScaleLabel, &nvsValue, sizeof(uint32_t));

        // Save the restored value only if the get was successful.
        if ((nvsSize == sizeof(uint32_t)) &&
                ((nvsValue == eTempScaleF) || (nvsValue == eTempScaleC)))
        {
            m_TempScale = static_cast<TempScale>(nvsValue);
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
bool EnvSensor::Reset()
{
    bool status = false;
    if (m_pName != NULL)
    {
        // Remove our state data rom NVS.
        Preferences prefs;
        prefs.begin(m_pName);
        status = prefs.remove(pPrefScaleLabel);
        prefs.end();
    }
    return status;
} // End Reset().

