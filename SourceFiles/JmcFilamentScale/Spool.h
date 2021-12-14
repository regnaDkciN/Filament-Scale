/////////////////////////////////////////////////////////////////////////////////
// Spool.h
//
// This class implements the Spool class.  It manage data pertaining to the
// filament contained on a spool.
//
// History:
// - jmcorbett 13-DEC-2020 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#if !defined SPOOL_H
#define SPOOL_H

#include <string.h>         // For strlcpy().
#include "Filament.h"       // For FilamentType.


/////////////////////////////////////////////////////////////////////////////////
// Spool class
//
// Handles all filament type and density related behavior.
/////////////////////////////////////////////////////////////////////////////////
class Spool
{
public:
    /////////////////////////////////////////////////////////////////////////////
    // Constructor.
    //
    // Sets default values for all instance data.  Uses the DEFAULT_SPOOL_NAME to
    // create a unique name for the instance by appending the instance number.
    /////////////////////////////////////////////////////////////////////////////
    Spool();


    /////////////////////////////////////////////////////////////////////////////
    // Simple destructor.
    /////////////////////////////////////////////////////////////////////////////
    ~Spool() {}


    /////////////////////////////////////////////////////////////////////////////
    // Simple getters.
    /////////////////////////////////////////////////////////////////////////////
    char *GetName()                   { return m_Name; }
    FilamentType GetType()    const   { return m_Type; }
    float GetDensity()        const   { return m_Density; }
    uint16_t GetColor()       const   { return m_Color; }
    float GetSpoolWeight()    const   { return m_SpoolWeight; }
    float GetDiameter()       const   { return m_Diameter; }
    static size_t GetInstanceCount()  { return m_InstanceCount; }
    static size_t GetMaxNameSize()    { return MAX_NAME_SIZE; }
    static float  GetMaxSpoolWeight() { return MAX_SPOOL_WEIGHT; }


    /////////////////////////////////////////////////////////////////////////////
    // Simple setters.  Each returns true if successful or false otherwise.
    /////////////////////////////////////////////////////////////////////////////
    bool SetName(const char *pName);
    bool SetType(FilamentType type);
    bool SetDensity(float density);
    bool SetSpoolWeight(float weight);
    bool SetDiameter(float diameter);
    bool SetColor(uint16_t color);


    /////////////////////////////////////////////////////////////////////////////
    // Instance data.
    //
    // Kludge alert!!!  The instance data items are declared in the public
    // section in order to work better with the ArduinoMenu implemantation.
    // This normally shouldn't be done, but in the case of using this class
    // with ArduinoMenu, it eliminates a ton of workarounds.
    /////////////////////////////////////////////////////////////////////////////
    static const uint32_t MAX_NAME_SIZE = 12U;


protected:


private:
    /////////////////////////////////////////////////////////////////////////////
    // Unimplemented methods.  We don't want users to try to use these.
    /////////////////////////////////////////////////////////////////////////////
    Spool(Spool &rCs);
    Spool &operator=(Spool &rCs);


    /////////////////////////////////////////////////////////////////////////////
    // Private static constants.
    /////////////////////////////////////////////////////////////////////////////
    static const char *DEFAULT_SPOOL_NAME;

    static const FilamentType DEFAULT_FILAMENT_TYPE;

    static const float DEFAULT_SPOOL_WEIGHT;
    static const float MIN_SPOOL_WEIGHT;
    static const float MAX_SPOOL_WEIGHT;

    static const float DEFAULT_FILAMENT_DIAMETER;
    static const float MIN_FILAMENT_DIAMETER;
    static const float MAX_FILAMENT_DIAMETER;


    /////////////////////////////////////////////////////////////////////////////
    // Instance count.  Gets incremented for each new instance that is created.
    /////////////////////////////////////////////////////////////////////////////
    static size_t m_InstanceCount;


    /////////////////////////////////////////////////////////////////////////////
    // Private instance data.
    /////////////////////////////////////////////////////////////////////////////
    char         m_Name[MAX_NAME_SIZE + 1]; // Instance name string.
    FilamentType m_Type;                    // Type of filament.
    float        m_Density;                 // Density of filament.
    float        m_Diameter;                // Filament diameter.
    float        m_SpoolWeight;             // Empty spool weight.
    uint16_t     m_Color;                   // Spool filament color.

}; // End class Spool.



#endif // SPOOL_H