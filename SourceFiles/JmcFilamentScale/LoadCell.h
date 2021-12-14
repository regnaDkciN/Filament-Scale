/////////////////////////////////////////////////////////////////////////////////
// LoadCell.h
//
// This class implements the LoadCell class.  It interfaces with the
// HX711 Weight LoadCell sensor to provide a normalized interface.
//
// History:
// - jmcorbett 29-AUG-2020 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#if !defined LOADCELL_H
#define LOADCELL_H


#include <HX711.h>              // For HX711 hardware specific data.
#include "MovingAverage.h"      // For MovingAverage class.



enum WeightUnits
{
    eWuGrams     = 0,       // Grams.
    eWuKiloGrams = 1,       // Kilograms.
    eWuOunces    = 2,       // Ounces.
    eWuPounds    = 3,       // Pounds.
    eWuNumUnits  = 4,       // Number of weight units.
    eWuBadVal    = 99       // A bad value (used for initialization).
};


/////////////////////////////////////////////////////////////////////////////////
// LoadCell class
//
// Derived from HX711 class.
/////////////////////////////////////////////////////////////////////////////////
class LoadCell : private HX711
{
public:
    /////////////////////////////////////////////////////////////////////////////
    // Construct and initialize the library and the pins used for the load cell.
    // The device is initialized with the specified gain and interface pins.
    //
    // Arguments:
    //    - dout   - This specifies the GPIO pin used to connect to the HX711 DOUT
    //               (Serial Data Out) pin.
    //    - scl    - This specifies the GPIO pin used to connect to the HX711
    //               PD_SCK (Power Down and Serial Clock Input) pin.
    //    - ledPin - This specifies the GPIO pin used to control the sensor's LED.
    //               This argument is required.  If the LED is permanently wired
    //               to VCC and will not be controlled by the processor, then pass
    //               a value of 0.
    /////////////////////////////////////////////////////////////////////////////
    LoadCell(int dout, int sck, uint8_t gain = DEFAULT_GAIN);


    // Destructor.
    virtual ~LoadCell() { }


    /////////////////////////////////////////////////////////////////////////////
    // Init()
    //
    // This method initializes the load cell.
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
    bool Init(const char *pName);


    /////////////////////////////////////////////////////////////////////////////
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
    /////////////////////////////////////////////////////////////////////////////
    bool SetGain(uint8_t gain);


    /////////////////////////////////////////////////////////////////////////////
    // IsPresent()
    //
    // Determines whether or not a load cell sensor is present.
    //
    // Returns:
    //   Returns true if a load cell was detected, or false otherwise.
    /////////////////////////////////////////////////////////////////////////////
    bool IsPresent() const;


    /////////////////////////////////////////////////////////////////////////////
    // ReadRawAverage()
    //
    // Thius method returns the average of a number of of weight value readings.
    // It verifies that ALL received values are within a limit, thus insuring
    // that the returned value is valid.
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
    /////////////////////////////////////////////////////////////////////////////
    uint32_t ReadRawAverage(uint16_t count = DEFAULT_TARE_COUNT);


    /////////////////////////////////////////////////////////////////////////////
    // Tare()
    //
    // This method calculates the tare offset for the scale.  This is the value
    // that is reported when the scale is empty.  The calculated offset is used
    // in future calculations to report object weights.
    //
    // The HX711 library contains a tare() method, but it simply calculates the
    // average of a number of readings.  This method is different in that it
    // takes the average of a number of readings and verifies that ALL received
    // values are within a limit, thus insuring that the saved tare value is
    // valid.
    //
    // Arguments:
    //   - count - This specifies the number of readings that will be used to
    //             calculate an average that will be used as the tare weight
    //             value.
    //
    // Returns:
    //   Returns 'true' if the tare value was successfully calculated.  Returns
    //   'false' otherwise.
    /////////////////////////////////////////////////////////////////////////////
    bool Tare(uint16_t count = DEFAULT_TARE_COUNT);


    /////////////////////////////////////////////////////////////////////////////
    // Calibrate()
    //
    // This method calculates and sets the base and display scale factors given
    // a raw reading and the cooked value that it represents.
    //
    // Note: We maintain a scale factor, 'm_UnitsScaleFactor', which is used
    //       to return scaled values based on the selected units to the user.
    //
    // Arguments:
    //   - rawWeight    - This specifies the raw reading from  the HX711.  If 0,
    //                    then read an averaged value from the sensor.
    //   - cookedWeight - This specifies the value that should be displayed when
    //                    the  HX711 returns a value equal to the value of
    //                    'rawWeight'.
    //
    // Returns:
    // Returns 'true' if successful, false otherwise.  Will only succeed if a
    // successful tare was previously completed and a raw calibration weight is
    // successfully read.
    /////////////////////////////////////////////////////////////////////////////
    bool Calibrate(uint32_t rawWeight, double cookedWeight);


    /////////////////////////////////////////////////////////////////////////////
    // SetUnits()
    //
    // This method is used to change the units that will be displayed.  It
    // calculates a new m_UnitsScaleFactor value and saves it and the newly
    // selected units value.  It also updates the m_Offset value.
    //
    // Arguments:
    //   - newUnits - This specifies the new display units type.
    //
    // Returns:
    // Returns 'true' if successful, false otherwise.  Can only fail if the new
    // units is specified as an invalid value.  In this case, m_Units is not
    // changed.
    /////////////////////////////////////////////////////////////////////////////
    bool SetUnits(WeightUnits newUnits);


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
    double ReadWeight();


    /////////////////////////////////////////////////////////////////////////////
    // SetAverageInterval()
    //
    // Sets the number of intervals that will be used for raw weight averaging.
    //
    // Arguments:
    //   - interval - This is the number of intvals to use for the raw weight
    //                averaging calculation.  Must be an integer greater than 0.
    //
    // Returns:
    // Returns 'true' if successful, or 'false' otherwise.
    /////////////////////////////////////////////////////////////////////////////
    bool SetAverageInterval(int32_t interval);


    /////////////////////////////////////////////////////////////////////////////
    // ResetAverage()
    //
    // Reset the moving average total to 0.
    /////////////////////////////////////////////////////////////////////////////
    void ResetAverage();


    /////////////////////////////////////////////////////////////////////////////
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
    /////////////////////////////////////////////////////////////////////////////
    static double GetBaseUnitsFactor(WeightUnits units);


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
    // Simple Getter and setter methods.
    /////////////////////////////////////////////////////////////////////////////
    uint8_t     GetGain()            const { return m_Gain; }
    double      GetOffset()          const { return m_Offset; }
    WeightUnits GetUnits()           const { return m_Units; }
    double      GetAverageInterval() const { return m_AverageInterval; }
    void SetOffset(double newOffset)       { m_Offset = newOffset; ResetAverage(); }
    bool IsCalibrated()              const { return m_IsCalibrated; }
    const char *GetUnitsString()     const { return UnitsStrings[static_cast<int>(m_Units)]; }
    double GetConversionFactor()     const { return m_ConversionFactor; }

protected:


private:
    // Unimplemented methods.    We don't want users to try to use these.
    LoadCell();
    LoadCell(LoadCell &rLc);
    LoadCell &operator=(LoadCell &rLc);


    /////////////////////////////////////////////////////////////////////////////
    // Private getter methods.
    /////////////////////////////////////////////////////////////////////////////
    double  GetUnitsScaleFactor() const { return m_UnitsScaleFactor; }
    int32_t GetTareValue()        const { return m_RawTareWeight; }


    /////////////////////////////////////////////////////////////////////////////
    // ReadAveragedValue()
    //
    // Take a reading from the HX711 and average it over the rolling average
    // interval.  Return the result.
    //
    // Returns:
    // Always returns the newly calculated value for m_RollingAverage.
    /////////////////////////////////////////////////////////////////////////////
    int64_t ReadAndAverageRawWeight();


    /////////////////////////////////////////////////////////////////////////////
    // ReadARawValue(), ReadARawValueD()
    //
    // Reads a single raw weight value from the HX711.  Returns the value as an
    // int or as a double.
    //
    // Note: The value read from the HX711 is not included in the running
    //       average.
    //
    // Returns:
    // ReadARawValue() returns an int32_t value read from the HX711.
    // ReadARawValueD() returns a value read from the HX711 cast to a double.
    /////////////////////////////////////////////////////////////////////////////
    int32_t ReadARawValue()  const { return read(); }
    double  ReadARawValueD() const { return static_cast<double>(read()); }


    /////////////////////////////////////////////////////////////////////////////
    //  Static constants.
    /////////////////////////////////////////////////////////////////////////////
    static const uint16_t DEFAULT_TARE_COUNT   = 20U;
    static const uint8_t  DEFAULT_GAIN         = 128U;
    static const double   GRAMS_PER_KILOGRAM;
    static const double   GRAMS_PER_POUND;
    static const double   OUNCES_PER_POUND;
    static const double   GRAMS_PER_OUNCE;
    static const double   UNCALIBRATED_READ_VALUE;
    static const double   DEFAULT_AVERAGE_INTERVAL;


    /////////////////////////////////////////////////////////////////////////////
    //  Save/Restore preferences labels.
    /////////////////////////////////////////////////////////////////////////////
    static const char *pPrefSavedStateLabel;
    static const char *UnitsStrings[];


    /////////////////////////////////////////////////////////////////////////////
    // Instance data.
    /////////////////////////////////////////////////////////////////////////////
    uint8_t     m_Gain;                 // HX711 gain.
    int32_t     m_RawTareWeight;        // Raw tare weight.
    bool        m_IsCalibrated;         // True if successfully calibrated.
    double      m_Offset;               // Optional offset for reporting weight.
    WeightUnits m_Units;                // Selected weight units.
    double      m_AverageInterval;      // Number of rolling values to average.
    double      m_UnitsScaleFactor;     // Factor for scaling the displayed weight.
    double      m_ConversionFactor;     // Factor for converting previous units to new.
    MovingAverage<int32_t, int64_t> m_MovingAverage;
                                        // Moving average handling class.
    const char *m_pName;                // NVS instance name.


    /////////////////////////////////////////////////////////////////////////////
    // Structure containing all of our state information that may be
    // saved/restored to/from NVS.
    /////////////////////////////////////////////////////////////////////////////
    struct SaveRestoreCache
    {
        uint8_t     m_Gain;             // HX711 gain.
        int32_t     m_RawTareWeight;    // Raw tare weight.
        bool        m_IsCalibrated;     // True if successfully calibrated.
        double      m_Offset;           // Optional offset for reporting weight.
        WeightUnits m_Units;            // Selected weight units.
        double      m_AverageInterval;  // Number of rolling values to average.
        double      m_UnitsScaleFactor; // Factor for scaling the displayed weight.
        double      m_ConversionFactor; // Factor for converting previous units to new.
    };

    // Allow unit tests to see all of our data and methods.
    friend class LoadCellTests;

}; // End class LoadCell.



#endif // LOADCELL_H
