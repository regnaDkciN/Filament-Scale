/////////////////////////////////////////////////////////////////////////////////
// EnvSensor.h
//
// This class implements the EnvSensor class.  It interfaces with the
// DHT22 Temperature and Humidity sensor to provide a normalized interface.
//
// History:
// - jmcorbett 29-AUG-2020 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#if !defined ENVSENSOR_H
#define ENVSENSOR_H


#include <DHT.h>                // For DHT hardware specific data.


/////////////////////////////////////////////////////////////////////////////////
// Temperature units enum.  Either degrees F or C.
/////////////////////////////////////////////////////////////////////////////////
enum TempScale
{
    eTempScaleF      = 0,       // Fahrenheit scale.
    eTempScaleC      = 1,       // Celcius scale.
    eTempScaleBadVal = 99       // Illegal value.
};


/////////////////////////////////////////////////////////////////////////////////
// EnvSensor class
/////////////////////////////////////////////////////////////////////////////////
class EnvSensor : private DHT
{
public:
    /////////////////////////////////////////////////////////////////////////////
    // Constructor and destructor.
    /////////////////////////////////////////////////////////////////////////////
    EnvSensor(uint8_t dataPin, uint8_t type) :
              DHT(dataPin, type), m_IsPresent(false), m_TempScale(eTempScaleF) { }
    virtual ~EnvSensor() { }


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
    bool Init(const char *pName);


    /////////////////////////////////////////////////////////////////////////////
    // SetTempScale()
    //
    // Sets the temperature scale to degrees F or degrees C.
    //
    // Arguments:
    //    - scale - This specifies the temperature scale to be used for reporting
    //              temperature (F or C).
    //
    // Returns:
    //    Returns 'true' if successful, or 'false' otherwise.
    /////////////////////////////////////////////////////////////////////////////
    bool  SetTempScale(TempScale scale);


    /////////////////////////////////////////////////////////////////////////////
    // GetTemperature()
    //
    // Returns the temperature based on the currently selected scale (F or C).
    //
    // Returns:
    //    Returns current temperature in degrees C or degrees F based on the
    //    current setting of m_TempScale.  Could return NAN if it has trouble
    //    reading the DHT22 sensor.
    /////////////////////////////////////////////////////////////////////////////
    float GetTemperature();


    /////////////////////////////////////////////////////////////////////////////
    // Save()
    //
    // Saves our current state to NVS.
    //
    // Returns:
    //    Returns 'true' if successful, or 'false' otherwise.
    /////////////////////////////////////////////////////////////////////////////
    bool Save() const;


    /////////////////////////////////////////////////////////////////////////////
    // Restore()
    //
    // Restores our state from NVS.
    //
    // Returns:
    //    Returns 'true' if successful, or 'false' otherwise.
    /////////////////////////////////////////////////////////////////////////////
    bool Restore();


    /////////////////////////////////////////////////////////////////////////////
    // Reset()
    //
    // Reset our state info in NVS.
    //
    // Returns:
    //    Returns 'true' if successful, or 'false' otherwise.
    /////////////////////////////////////////////////////////////////////////////
    bool Reset();


    /////////////////////////////////////////////////////////////////////////////
    // Trivial getters and converters whose behavior should be immediately obvious.
    /////////////////////////////////////////////////////////////////////////////
    bool  IsPresent()                   { return m_IsPresent; }
    float GetDegreesF()                 { return readTemperature(true, false); }
    float GetDegreesC()                 { return readTemperature(false, false); }
    float ConvertCtoF(float c)          { return convertCtoF(c); }
    float ConvertFtoC(float f)          { return convertFtoC(f); }
    float GetHumidity()                 { return readHumidity(false); }
    TempScale GetTempScale() const      { return m_TempScale; }
    const char *GetTempScaleString() const
                                        { return TempScaleStrings[m_TempScale]; }
    const char *GetTempScaleString(TempScale s) const
                                        { return TempScaleStrings[s]; }


protected:


private:
    // Unimplemented methods
    EnvSensor();
    EnvSensor(EnvSensor &rEs);
    EnvSensor &operator=(EnvSensor &Es);

    // Private constant data.
    static const char *pPrefScaleLabel;
    static const char *TempScaleStrings[];

    // Private instance data.
    bool        m_IsPresent;        // True if the env sensor was detected.
    TempScale   m_TempScale;        // Temperature scale in use (F or C).
    const char *m_pName;            // NVS storage name for this instance.

}; // End class EnvSensor.



#endif // ENVSENSOR_H
