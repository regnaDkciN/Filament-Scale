/////////////////////////////////////////////////////////////////////////////////
// LoadCell.cpp
//
// Contains methods defined by the LoadCell class.  These methods
// manage the  HX711 Weigh LoadCell sensor.
//
// History:
// - jmcorbett 29-AUG-2020 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#include "LoadCell.h"       // For LoadCell class.
#include <Preferences.h>    // For Save and Restore to/from NVS.


// Setup some conversion constants.
const double LoadCell::GRAMS_PER_KILOGRAM       = 1000.0d;
const double LoadCell::GRAMS_PER_POUND          = 453.592d;
const double LoadCell::OUNCES_PER_POUND         = 16.0d;
const double LoadCell::GRAMS_PER_OUNCE          = GRAMS_PER_POUND / OUNCES_PER_POUND;
const double LoadCell::UNCALIBRATED_READ_VALUE  = -999999999.9d;
const double LoadCell::DEFAULT_AVERAGE_INTERVAL = 10.0d;

const char *LoadCell::pPrefSavedStateLabel  = "Saved State";
const char *LoadCell::UnitsStrings[]        = {" g", " kg", " oz", " lb"};

static const size_t MAX_NVS_NAME_LEN = 15U;


/////////////////////////////////////////////////////////////////////////////
// Construct and initialize the library and the pins used for the load cell.
// The device is initialized with the specified gain and interface pins.
//
// Arguments:
//    - dout   - This specifies the GPIO pin used to connect to the HX711 DOUT
//               (Serial Data Out) pin.
//    - scl    - This specifies the GPIO pin used to connect to the HX711
//               PD_SCK (Power Down and Serial Clock Input) pin.
//    - gain   - valid values are 64 and 128.  This assumes that the load cell
//               is connected to channel A of the HX711.
/////////////////////////////////////////////////////////////////////////////
LoadCell::LoadCell(int dout, int sck, uint8_t gain) : HX711(), m_Gain(gain),
        m_RawTareWeight(0L), m_IsCalibrated(false), m_Offset(0.0d),
        m_Units(eWuGrams), m_AverageInterval(DEFAULT_AVERAGE_INTERVAL),
        m_UnitsScaleFactor(1.0d), m_ConversionFactor(1.0),
        m_MovingAverage(DEFAULT_AVERAGE_INTERVAL), m_pName(NULL)
{
    begin(dout, sck, gain);
} // End constructor.


/////////////////////////////////////////////////////////////////////////////////
// Init()
//
// This method initializes the load cell.
//
// Arguments:
//    - pName   - A string of no more than 15 characters to be used as a
//                name for this instance.  This is mainly used to identify
//                the instance to be used for NVS save and restore.
//
// Returns:
//    Returns a bool indicating whether or not the initialization was successful.
//    A 'true' value indicates success, while a 'false' value indicates failure.
//
/////////////////////////////////////////////////////////////////////////////////
bool LoadCell::Init(const char *pName)
{
    bool status = false;

    if (IsPresent() && (pName != NULL) && (*pName != '\0') &&
        (strlen(pName) <= MAX_NVS_NAME_LEN))
    {
        m_pName = pName;
        status = true;
    }
    return status;
} // End Init().


/////////////////////////////////////////////////////////////////////////////////
// SetGain()
//
// Sets the gain that will be used by the load cell.
//
// Arguments:
//    - gain - valid values are 64 and 128.  This assumes that the load cell
//             is connected to channel A of the HX711.
//
// Returns:
//    Returns a bool indicating whether or not the operation was successful.
//    A value of 'true' indicates success.  A value of 'false' indicates
//    failure.
/////////////////////////////////////////////////////////////////////////////////
bool LoadCell::SetGain(uint8_t gain)
{
    // Assume we're going to fail.
    bool status = false;

    // If the gain value has changed and is OK then set the new gain.
    if (((gain == 128U) || (gain == 64U)) && (gain != m_Gain))
    {
        // Need to recalibrate since the gain has changed.
        m_IsCalibrated = false;

        // Set the new gain.
        m_Gain = gain;
        set_gain(m_Gain);
        status = true;

        // Take a throwaway reading to clear out the value from the previous
        // gain setting.
        ReadARawValue();

        // Seed our average value with a new raw reading.
        ResetAverage();
    }
    // Return the success/failre status.
    return status;
} // End SetGain().


/////////////////////////////////////////////////////////////////////////////////
// IsPresent()
//
// Determines whether or not a load cell sensor is present.
//
// Returns:
//   Returns true if a load cell was detected, or false otherwise.
/////////////////////////////////////////////////////////////////////////////////
bool LoadCell::IsPresent() const
{
    // Assume success.
    bool status = true;

    // We can't simply wait for the card to be ready since when no card is
    // present, the DOUT pin can be read as low, which is how the library
    // determines whether or not the HX711 is ready.  However, when no HX711
    // is present, a read() of the device will return a value of 0.  So we use
    // this as an indicator as to whether or not the sensor is present.
    if (!wait_ready_retry(10U, 10U) || (ReadARawValue() == 0L))
    {
        // Looks like we didn't detect a card.
        status = false;
    }

    // Let the caller know our status.
    return status;
} // End IsPresent().


/////////////////////////////////////////////////////////////////////////////////
// ReadRawAverage()
//
// This method takes the average of a number of readings and verifies that ALL
// read values are within a limit, thus insuring that the returned value is valid.
//
// Arguments:
//   - count - This specifies the number of readings that will be used to
//             calculate an average that will be used as the returned weight
//             value.
//
// Returns:
//   If successful, returns the average value read from the 'count' readings
//   that were taken.  If any values fall outside of an acceptable range,
//   then a value of 0 is returned indicating failure.
/////////////////////////////////////////////////////////////////////////////////
uint32_t LoadCell::ReadRawAverage(uint16_t count)
{
    // We need to keep track of the sum of the readings to calculate the
    // average value.  We also need to determine the minimum and maximum
    // values that are read in order to insure that the readings are
    // acceptable (within an acceptable spread).
    int32_t sum = 0L;
    int32_t min = INT_MAX;
    int32_t max = INT_MIN;

    // *** For some reason, the first read always seems higher than it should
    // *** be.  Haven't been able to find the reason for this.
    // *** Take a throwaway reading to get rid of the (probably) high first one.
    ReadARawValue();

    // Gather the readings, and update our sum, min and max values.
    for (uint16_t i = 0U; i < count; i++)
    {
        int32_t weight = ReadARawValue();
        Serial.println(weight);
        sum += weight;
        if (weight < min)
        {
            min = weight;
        }
        if (weight > max)
        {
            max = weight;
        }
    }

    // Calculate the average and the limit values.
    // We want all readings to be within 0.125% of the average, 
    // (1.25/1000 = .00125).  This is a precaution to try to reject readings
    // that are taken while the scale is being disturbed.  If any individual
    // value falls outside the range of  average +/- .125% the readings will be
    //rejected, and a value of 0 will be returned, indicating an error.
    
    const int32_t LIMIT_DIVISOR = 800;
    int32_t avg = sum / count;
    int32_t upperLimit = avg + avg / LIMIT_DIVISOR;
    int32_t lowerLimit = avg - avg / LIMIT_DIVISOR;
    Serial.print("Read average = ");
    Serial.println(avg);
    Serial.print("Read min = ");
    Serial.println(min);
    Serial.print("Read max = ");
    Serial.println(max);
    Serial.print("Read span = ");
    Serial.println(max - min);
    Serial.print("Read positive limit = ");
    Serial.println(upperLimit);
    Serial.print("Read negative limit = ");
    Serial.println(lowerLimit);
    Serial.print("Read min, max head room: ");
    Serial.print(min - lowerLimit);
    Serial.print(", ");
    Serial.println(upperLimit - max);

    // See if the average value falls within our acceptable limit.
    if ((max < upperLimit) && (min > lowerLimit))
    {
        // Average is acceptable.
        Serial.println("Good value read.");
    }
    else
    {
        // Average is not acceptable.  Return a value of 0.
        avg = 0;
        Serial.println("Bad value read.");
    }

    // Let the caller know our read average, or 0 if unsuccessful.
    return avg;
} // End ReadRawAverage().


/////////////////////////////////////////////////////////////////////////////////
// Tare()
//
// This method calculates the tare offset for the scale.  This is the value that
// is reported when the scale is empty.  The calculated offset is used in future
// calculations to report object weights.
//
// The HX711 library contains a tare() method, but it simply calculates the
// average of a number of readings.  This method is different in that it takes
// the average of a number of readings and verifies that ALL received values are
// within a limit, thus insuring that the saved tare value is valid.
//
// Arguments:
//   - count - This specifies the number of readings that will be used to
//             calculate an average that will be used as the tare weight
//             value.
//
// Returns:
//   Returns 'true' if the tare value was successfully calculated.  Returns
//   'false' otherwise.
/////////////////////////////////////////////////////////////////////////////////
bool LoadCell::Tare(uint16_t count)
{
    // Allow load cell time for first reading.
    delay(500);

    uint32_t tareValue = ReadRawAverage(count);
    if (tareValue)
    {
        m_RawTareWeight  = tareValue;
        m_MovingAverage.Reset();
        Serial.print("Tare using weight = ");
        Serial.println(tareValue);
    }
    return tareValue != 0;
} // End Tare().


/////////////////////////////////////////////////////////////////////////////////
// Calibrate()
//
// This method calculates and sets the base and display scale factors given
// a raw reading and the cooked value that it represents.
//
// Note: We maintain a scale factor, 'm_UnitsScaleFactor', which is used
//       to return scaled values based on the selected units to the user.
//
// Arguments:
//   - rawWeight       - This specifies the raw reading from  the HX711.  If 0,
//                       then read an averaged value from the sensor.
//   - cookedCalWeight - This specifies the value that should be displayed when
//                       the  HX711 returns a value equal to the value of
//                       'rawCalWeight'.
//
// Returns:
// Returns 'true' if successful, false otherwise.  Will only succeed if a
// successful tare was previously completed and a raw calibration weight is
// successfully read.
/////////////////////////////////////////////////////////////////////////////////
bool LoadCell::Calibrate(uint32_t rawCalWeight, double cookedCalWeight)
{
    // Allow load cell time for first reading.
    delay(500);

    // We can only calibrate if a successful tare has been done.
    if (m_RawTareWeight == 0L)
    {
        return false;
    }

    // Special case when the value of 'raw' is 0, we read a raw value from
    // the HX711.
    if (rawCalWeight == 0UL)
    {
        rawCalWeight = ReadRawAverage();
        if (rawCalWeight == 0UL)
        {
            return false;
        }
    }

    // Seed our averaging code.
    m_MovingAverage.Reset();

    // The display scale factor is simply the ratio of the cooked and raw values.
    m_UnitsScaleFactor =
        cookedCalWeight / static_cast<double>(rawCalWeight - m_RawTareWeight);

    // Remember that we've been calibrated.
    m_IsCalibrated = true;

    return true;
} // End Calibrate().


/////////////////////////////////////////////////////////////////////////////////
// GetBaseUnitsFactor()
//
// Returns the scale factor used to convert the given weight units to the
// base units (grams).
//
// Arguments:
//   - units - This specifies the units to convert from.
//
// Returns:
// Returns the double scale factor that may be used to convert the given
// units to base units.  A value of 0.0 indicates that an invalid value
// was passed in the 'units' argument.
/////////////////////////////////////////////////////////////////////////////////
double LoadCell::GetBaseUnitsFactor(WeightUnits units)
{
    double baseFactor = 0.0;
    switch (units)
    {
    case eWuGrams:
        // We use grams as our base units.
        baseFactor = 1.0d;;
        break;
    case eWuKiloGrams:
        baseFactor = GRAMS_PER_KILOGRAM;
        break;
    case eWuOunces:
        baseFactor = GRAMS_PER_OUNCE;
        break;
    case eWuPounds:
        baseFactor = GRAMS_PER_POUND;
        break;
    default:
        // We will return 0.0 if the specified units are no good.
        break;
    }

    return baseFactor;
} // End GetBaseUnitsFactor().


/////////////////////////////////////////////////////////////////////////////////
// ReadWeight()
//
// This method reads a value from the HX711 and returns the scaled value
// representing the read weight scaled and offset by the value of m_Offset.
//
// Returns:
//    Returns the scaled and offset value if successful.  Otherwise it returns
//    a value of UNCALIBRATED_READ_VALUE.
/////////////////////////////////////////////////////////////////////////////////
double LoadCell::ReadWeight()
{
    // Assume that we fail.
    double scaledValue = UNCALIBRATED_READ_VALUE;

    // If we haven't been calibrated yet, then we fail.
    if (m_IsCalibrated)
    {
        // Let's convert the averaged weight to a double.
        double doubleWeight = static_cast<double>(ReadAndAverageRawWeight());

        // Scale the value.
        scaledValue =
            ((doubleWeight - static_cast<double>(m_RawTareWeight)) *
                    m_UnitsScaleFactor
            ) - m_Offset;
    }

    return scaledValue;
} // End ReadWeight().


/////////////////////////////////////////////////////////////////////////////////
// ResetAverage()
//
// Reset the moving average total to 0.
/////////////////////////////////////////////////////////////////////////////////
void LoadCell::ResetAverage()
{
    m_MovingAverage.Reset();
} // End ResetAverage().


/////////////////////////////////////////////////////////////////////////////////
// ReadAndAverageRawWeight()
//
// Take a reading from the HX711 and average it over the rolling average
// interval.  Return the result.
//
// Returns:
// Always returns the newly calculated value for m_RollingAverage.
/////////////////////////////////////////////////////////////////////////////////
int64_t LoadCell::ReadAndAverageRawWeight()
{
    // Update our moving average total.
    m_MovingAverage.Add(ReadARawValue());

    // Return the new average.
    return m_MovingAverage.Average();
} // End ReadAndAverageRawWeight().


/////////////////////////////////////////////////////////////////////////////////
// SetAverageInterval()
//
// Sets the number of intervals that will be used for raw weight averaging.
//
// Arguments:
//   - interval - This is the number of intvals to use for the raw weight
//                averaging calculation.  Must be an integer greater than 0.
//
// Returns:
// Returns 'true' if successful, or 'false' if the value was limited.
/////////////////////////////////////////////////////////////////////////////////
bool LoadCell::SetAverageInterval(int32_t interval)
{
    // Set the new size.
    m_MovingAverage.SetSize(interval);

    // Get the size (may have been limited) and remember it.
    int32_t size = m_MovingAverage.Size();
    m_AverageInterval = static_cast<double>(size);

    // Let the user know if the requested size was accepted or limited.
    return size == interval;
} // End SetAverageInterval().


/////////////////////////////////////////////////////////////////////////////////
// SetUnits()
//
// This method is used to change the units that will be displayed.  It calculates
// a new m_UnitsScaleFactor value and saves it and the newly selected units
// value.  It also updates the m_Offset value.
//
// Arguments:
//   - newUnits - This specifies the new display units type.
//
// Returns:
// Returns 'true' if successful, false otherwise.  Can only fail if the new
// units is specified as an invalid value.  In this case, m_Units is not changed.
/////////////////////////////////////////////////////////////////////////////////
bool LoadCell::SetUnits(WeightUnits newUnits)
{
    bool status = true;

    // Calculate the new m_UnitsScaleFactor and m_Offset values.
    if (newUnits != m_Units)
    {
        // We calculate a multiplier to be used to determine the new values for
        // our current offset and scale factors.  To do this, we calculate a
        // factor by essentially converting the current units to base units,
        // then converting the resulting base units to the newly specified units.
        WeightUnits oldUnits  = m_Units;
        double toBaseFactor   = GetBaseUnitsFactor(oldUnits);
        double fromBaseFactor = GetBaseUnitsFactor(newUnits);
        if ((toBaseFactor == 0.0d) || (fromBaseFactor == 0.0d))
        {
            // If either of the returned factors is 0.0, this implies that the
            // corresponding call to GetBaseUnitsFactor() failed.
            status = false;
        }
        else
        {
            // The returned factors were good, so determine our multiplier.
            m_ConversionFactor = toBaseFactor / fromBaseFactor;

            // Adjust the existing offset and scale factor values and save the
            // new units value.  Protect these settings so they will be seen
            // as an atomic change.
            noInterrupts();
            m_Offset            *= m_ConversionFactor;
            m_UnitsScaleFactor  *= m_ConversionFactor;
            m_Units              = newUnits;
            interrupts();
        }
    }

    return status;
} // End SetUnits().


/////////////////////////////////////////////////////////////////////////////////
// Save()
//
// Saves our current state to NVS.
//
// Returns:
//    Returns 'true' if successful, or 'false' otherwise.
/////////////////////////////////////////////////////////////////////////////////
bool LoadCell::Save() const
{
    size_t saved = 0;
    if (m_pName != NULL)
    {
        // Cache our state information for transfer to NVS as a single unit.
        SaveRestoreCache currentState;
        currentState.m_Gain             = m_Gain;
        currentState.m_RawTareWeight    = m_RawTareWeight;
        currentState.m_IsCalibrated     = m_IsCalibrated;
        currentState.m_Offset           = m_Offset;
        currentState.m_Units            = m_Units;
        currentState.m_AverageInterval  = m_AverageInterval;
        currentState.m_UnitsScaleFactor = m_UnitsScaleFactor;
        currentState.m_ConversionFactor = m_ConversionFactor;

        // Prepare for NVS use.
        Preferences prefs;
        prefs.begin(m_pName);

        // Fetch our currently saved NVS data for comparison.
        SaveRestoreCache nvsState;
        size_t nvsSize =
            prefs.getBytes(pPrefSavedStateLabel, &nvsState,
                           sizeof(SaveRestoreCache));

        // Compare the saved and current data.  If it hasn't changed, then
        // we don't need to save anything (saving NVS writes).
        if ((nvsSize != sizeof(SaveRestoreCache)) ||
            memcmp(&nvsState, &currentState, sizeof(SaveRestoreCache)))
        {
            // NVS and working didn't match, so save the working.
            saved =
                prefs.putBytes(pPrefSavedStateLabel, &currentState,
                               sizeof(SaveRestoreCache));
            Serial.println("\nLoadCell - saving to NVS.");
        }
        else
        {
            // NVS matched working.  Simply setup to return success.
            saved = sizeof(SaveRestoreCache);
            Serial.println("\nLoadCell - not saving to NVS");
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
bool LoadCell::Restore()
{
    // Assume we're gonna fail.
    bool succeeded = false;

    // Make sure we have a valid name.
    if (m_pName != NULL)
    {
        // Restore our state data to a temporary structure.
        SaveRestoreCache cachedState;
        Preferences prefs;
        prefs.begin(m_pName);
        size_t restored =
            prefs.getBytes(pPrefSavedStateLabel, &cachedState, sizeof(SaveRestoreCache));

        // Save the restored values only if the get was successful.
        if (restored == sizeof(cachedState))
        {
            // Set our calibrated variable.
            m_IsCalibrated = cachedState.m_IsCalibrated;

            // Set the gain.
            SetGain(cachedState.m_Gain);

            // Set the tare weight.
            m_RawTareWeight  = cachedState.m_RawTareWeight;

            // Set our scale factor.
            m_UnitsScaleFactor = cachedState.m_UnitsScaleFactor;

            // Set our averaging interval.
            m_AverageInterval = cachedState.m_AverageInterval;
            m_MovingAverage.SetSize(static_cast<int32_t>(m_AverageInterval));

            // Set our offset (if any).
            m_Offset = cachedState.m_Offset;

            // Set our units.
            m_Units = cachedState.m_Units;

            // Set our conversion factor.
            m_ConversionFactor = cachedState.m_ConversionFactor;

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
bool LoadCell::Reset()
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



