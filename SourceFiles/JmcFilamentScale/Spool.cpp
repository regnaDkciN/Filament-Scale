/////////////////////////////////////////////////////////////////////////////////
// Spool.cpp
//
// Contains methods defined by the Spool class.  These methods
// manage data pertaining to the filament contained on a spool.
//
// History:
// - jmcorbett 13-DEC-2020 Original creation.
// - jmcorbett 30-AUG-2022 SetColor() returns void.
//
// Copyright (c) 2022, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#include <Preferences.h>
#include "Spool.h"


/////////////////////////////////////////////////////////////////////////////////
// Spool class constants.  Should be self explanatory.
/////////////////////////////////////////////////////////////////////////////////
const FilamentType Spool::DEFAULT_FILAMENT_TYPE = eFtPla;
const float Spool::DEFAULT_SPOOL_WEIGHT         = 250.0;
const float Spool::DEFAULT_FILAMENT_DIAMETER    = 1.75;
const char *Spool::DEFAULT_SPOOL_NAME           = "Spool";
const float Spool::MIN_FILAMENT_DIAMETER        = 0.01;
const float Spool::MAX_FILAMENT_DIAMETER        = 5.0;
const float Spool::MIN_SPOOL_WEIGHT             = 0.0;
const float Spool::MAX_SPOOL_WEIGHT             = 5000.0;


/////////////////////////////////////////////////////////////////////////////////
// Spool class instance count.  Gets incremented for each new instance that
// is created.
/////////////////////////////////////////////////////////////////////////////////
size_t Spool::m_InstanceCount = 0;


/////////////////////////////////////////////////////////////////////////////////
// Constructor.
//
// Sets default values for all instance data.  Uses the DEFAULT_SPOOL_NAME to
// create a unique name for the instance by appending the instance number.
/////////////////////////////////////////////////////////////////////////////////
Spool::Spool() :
    m_Type(DEFAULT_FILAMENT_TYPE), m_Density(Filament::GetDensity(m_Type)),
    m_SpoolWeight(DEFAULT_SPOOL_WEIGHT), m_Diameter(DEFAULT_FILAMENT_DIAMETER),
    m_Color(0)
{
    memset(m_Name, '\0', MAX_NAME_SIZE + 1);
    snprintf(m_Name, MAX_NAME_SIZE + 1,
            "%s %02d", DEFAULT_SPOOL_NAME, ++m_InstanceCount);
} // End Spool().


/////////////////////////////////////////////////////////////////////////////////
// Simple setters.  Each returns true if successful or false otherwise.
/////////////////////////////////////////////////////////////////////////////////
bool Spool::SetName(const char *pName)
{
    // Validate the string.
    bool status = (pName != NULL) && (*pName != '\0');
    if (status)
    {
        // String was OK.  Save it.  May be truncated.
        strlcpy(m_Name, pName, MAX_NAME_SIZE + 1);
    }
    return status;
} // End SetName().


bool Spool::SetType(FilamentType type)
{
    // Validate the new type value.
    bool status = type < eFtCount;
    if (status)
    {
        // New type value is OK.  Save it.
        m_Type = type;
    }
    return status;
} // End SetType().


bool Spool::SetDensity(float density)
{
    // Validate the new density value.
    bool status = (density >= Filament::MIN_DENSITY) &&
                  (density <= Filament::MAX_DENSITY);
    if (status)
    {
        // New density value is OK.  Save it.
        m_Density = density;
    }
    return status;
} // End SetDensity().


bool Spool::SetSpoolWeight(float weight)
{
    // Validate the new weight value.
    bool status = ((weight >= MIN_SPOOL_WEIGHT) && (weight <= MAX_SPOOL_WEIGHT));
    if (status)
    {
        // Weight value is OK.  Save it.
        m_SpoolWeight = weight;
    }
    return status;
} // End SetSpoolWeight().


bool Spool::SetDiameter(float diameter)
{
    bool status = ((diameter >= MIN_FILAMENT_DIAMETER) &&
                   (diameter <= MAX_FILAMENT_DIAMETER));
    if (status)
    {
        m_Diameter = diameter;
    }
    return status;
} // End SetDiameter().


void Spool::SetColor(uint16_t color)
{
    m_Color = color;
} // End SetColor().

