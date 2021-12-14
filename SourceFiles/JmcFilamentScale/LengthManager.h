/////////////////////////////////////////////////////////////////////////////////
// LengthManager.h
//
// This class implements the LengthManager class.  It contains methods to handle
// calculation of filament length given weight, density, and filament diameter.
//
// History:
// - jmcorbett 12-DEC-2020 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#if !defined LENGTHMANAGER_H
#define LENGTHMANAGER_H



/////////////////////////////////////////////////////////////////////////////////
// LengthUnits
//
// This enum contains entries that correspond to the supported length units.
// The value of each entry also corresponds to the index into the length unit
// strings and precision arrays.
/////////////////////////////////////////////////////////////////////////////////
enum LengthUnits
{
    luMm    = 0,    // Millimeters.
    luCm    = 1,    // Centimeters.
    luM     = 2,    // Meters.
    luIn    = 3,    // Inches.
    luFt    = 4,    // Feet.
    luYd    = 5,    // Yards.
    luNum   = 6     // Number of length units.
};


/////////////////////////////////////////////////////////////////////////////////
// LengthManager type string defines.
//
// NOTE:
//    This is a kludge that was necessitated by the use of the length
//    strings in the ArduinoMenu system.  I could not find a reasonable way to
//    allow pointers to the strings to be used in the TOGGLE menu macro.  These
//    macros were created as a compromise, so that the strings would agree
//    between this class and other length related items displayed by the menu
//    system.
/////////////////////////////////////////////////////////////////////////////////
#define UNIT_STRING_MM "mm"
#define UNIT_STRING_CM "cm"
#define UNIT_STRING_M  "m"
#define UNIT_STRING_IN "in"
#define UNIT_STRING_FT "ft"
#define UNIT_STRING_YD "yd"



/////////////////////////////////////////////////////////////////////////////////
// LengthManager class
//
// Handles all filament type and density related behavior.
/////////////////////////////////////////////////////////////////////////////////
class LengthManager
{
public:
    /////////////////////////////////////////////////////////////////////////////
    // Simple constructor and destructor.
    /////////////////////////////////////////////////////////////////////////////
    LengthManager() : m_pName(NULL), m_SelectedUnits(luMm) {}
    ~LengthManager() {}


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
    bool Init(const char *pName);


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
    // GetUnitsString()
    //
    // These methods return a pointer to a string representing the specified
    // units type (mm, cm, ...).  There are two methods here.  The first one
    // (which uses a type argument) will return the string corresponding to
    // the specified type.  The second one (which uses no arguments) will
    // return the string corresponding to the currently selected units typa.
    //
    // Arguments:
    //    - type - The type of units whose string is desired.
    //
    // Returns:
    //    Returns a pointer to the string corresponding to the specified units
    //    type.  If an invalid type is specified, then millimeters is used.
    //
    /////////////////////////////////////////////////////////////////////////////
    static const char *GetUnitsString(LengthUnits type);
    const char *GetUnitsString() { return GetUnitsString(m_SelectedUnits); }


    /////////////////////////////////////////////////////////////////////////////
    // GetPrecision()
    //
    // These methods return the precision corresponding to the specified
    // units type (mm, cm, ...).  There are two methods here.  The first one
    // (which uses a type argument) will return the precision corresponding to
    // the specified type.  The second one (which uses no arguments) will
    // return the precision corresponding to the currently selected units typa.
    //
    // Arguments:
    //    - type - The type of units whose precision is desired.
    //
    // Returns:
    //    Returns a precision corresponding to the specified units
    //    type.  If an invalid type is specified, then millimeters is used.
    //
    /////////////////////////////////////////////////////////////////////////////
    static uint16_t    GetPrecision(LengthUnits type);
    uint16_t    GetPrecision() const { return GetPrecision(m_SelectedUnits); }


    /////////////////////////////////////////////////////////////////////////////
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
    /////////////////////////////////////////////////////////////////////////////
    bool SetUnits(LengthUnits type);


    /////////////////////////////////////////////////////////////////////////////
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
    /////////////////////////////////////////////////////////////////////////////
    static float GetUnitsFactor(uint16_t units);
    float GetUnitsFactor() const { return GetUnitsFactor(m_SelectedUnits); }


    /////////////////////////////////////////////////////////////////////////////
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
    /////////////////////////////////////////////////////////////////////////////
    float CalculateLengthFactor(float filamentDiameterMM,
                                float weightFactor,
                                float filamentDensity) const;


    /////////////////////////////////////////////////////////////////////////////
    // Getters and setters.
    /////////////////////////////////////////////////////////////////////////////
    LengthUnits GetSelected() const  { return m_SelectedUnits; }


    /////////////////////////////////////////////////////////////////////////////
    // Public static constants.
    /////////////////////////////////////////////////////////////////////////////


protected:


private:
    /////////////////////////////////////////////////////////////////////////////
    // Unimplemented methods.  We don't want users to try to use these.
    /////////////////////////////////////////////////////////////////////////////
    LengthManager(LengthManager &rCs);
    LengthManager &operator=(LengthManager &rCs);


    /////////////////////////////////////////////////////////////////////////////
    // Private static constants.
    /////////////////////////////////////////////////////////////////////////////
    static const uint16_t     NUMBER_UNITS = static_cast<uint16_t>(luNum);
    static const char * const UnitStrings[NUMBER_UNITS];
    static const uint16_t     UnitPrecicions[NUMBER_UNITS];
    static const char        *pPrefSavedStateLabel;
    static const float        MM_PER_MM;
    static const float        CM_PER_MM;
    static const float        M_PER_MM;
    static const float        IN_PER_MM;
    static const float        FT_PER_MM;
    static const float        YD_PER_MM;


    const char *m_pName;                // NVS instance name.
    LengthUnits m_SelectedUnits;        // Selected length units type.

}; // End class LengthManager.



#endif // LENGTHMANAGER_H