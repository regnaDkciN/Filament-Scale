/////////////////////////////////////////////////////////////////////////////////
// Filament.h
//
// This class implements the Filament class.  It maintains data pertaining to
// the supported types of filament and their associated densities.
//
// History:
// - jmcorbett 12-DEC-2020 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#if !defined FILAMENT_H
#define FILAMENT_H

#include <string.h>         // For strlcpy().


/////////////////////////////////////////////////////////////////////////////////
// FilamentType
//
// This enum contains entries that correspond to the supported filament types.
// The value of each entry also corresponds to the index into the filament type's
// string or density arrays.
/////////////////////////////////////////////////////////////////////////////////
enum FilamentType
{
    eFtAbs   = 0,       // ABS
    eFtAsa   = 1,       // ASA
    eFtCopr  = 2,       // Copper
    eFtHips  = 3,       // HIPS
    eFtNylon = 4,       // Nylon
    eFtPetg  = 5,       // PETG
    eFtPla   = 6,       // PLA
    eFtPmma  = 7,       // PMMA
    eFtPlyC  = 8,       // PolyC
    eFtPva   = 9,       // PVA
    eFtTpe   = 10,      // TPE
    eFtTpu   = 11,      // TPU
    eFtUser1 = 12,      // User specified - 1
    eFtUser2 = 13,      // User specified - 2
    eFtUser3 = 14,      // User specified - 3
    eFtCount = 15       // Used only to count the number of supported filaments.
};


/////////////////////////////////////////////////////////////////////////////////
// Filament type string defines.
//
// NOTE:
//    This is a kludge that was necessitated by the use of the filament type
//    strings in the ArduinoMenu system.  I could not find a reasonable way to
//    allow pointers to the strings to be used in the TOGGLE menu macro.  These
//    macros were created as a compromise, so that the strings would agree
//    between this class and the filament types displayed by the menu system.
/////////////////////////////////////////////////////////////////////////////////
#define FILAMENT_STRING_ABS    "ABS"
#define FILAMENT_STRING_ASA    "ASA"
#define FILAMENT_STRING_COPPER "Copr"
#define FILAMENT_STRING_HIPS   "HIPS"
#define FILAMENT_STRING_NYLON  "Nyln"
#define FILAMENT_STRING_PETG   "PETG"
#define FILAMENT_STRING_PLA    "PLA"
#define FILAMENT_STRING_PMMA   "PMMA"
#define FILAMENT_STRING_POLYC  "PlyC"
#define FILAMENT_STRING_PVA    "PVA"
#define FILAMENT_STRING_TPE    "TPE"
#define FILAMENT_STRING_TPU    "TPU"
#define FILAMENT_STRING_USER1  "USR1"
#define FILAMENT_STRING_USER2  "USR2"
#define FILAMENT_STRING_USER3  "USR3"

#define FILAMENT_LSTRING_ABS    FILAMENT_STRING_ABS
#define FILAMENT_LSTRING_ASA    FILAMENT_STRING_ASA
#define FILAMENT_LSTRING_COPPER "Copper"
#define FILAMENT_LSTRING_HIPS   FILAMENT_STRING_HIPS
#define FILAMENT_LSTRING_NYLON  "Nylon"
#define FILAMENT_LSTRING_PETG   FILAMENT_STRING_PETG
#define FILAMENT_LSTRING_PLA    FILAMENT_STRING_PLA
#define FILAMENT_LSTRING_PMMA   FILAMENT_STRING_PMMA
#define FILAMENT_LSTRING_POLYC  "PolyC"
#define FILAMENT_LSTRING_PVA    FILAMENT_STRING_PVA
#define FILAMENT_LSTRING_TPE    FILAMENT_STRING_TPE
#define FILAMENT_LSTRING_TPU    FILAMENT_STRING_TPU
#define FILAMENT_LSTRING_USER1  "User-1"
#define FILAMENT_LSTRING_USER2  "User-2"
#define FILAMENT_LSTRING_USER3  "User-3"



/////////////////////////////////////////////////////////////////////////////////
// Filament class
//
// Handles all filament type and density related behavior.
/////////////////////////////////////////////////////////////////////////////////
class Filament
{
public:
    /////////////////////////////////////////////////////////////////////////////
    // Simple constructor and destructor.
    /////////////////////////////////////////////////////////////////////////////
    Filament() : m_pName(NULL) {}
    ~Filament() {}


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
    // Simple Getters and Setters.
    /////////////////////////////////////////////////////////////////////////////
    static void SetDensity(FilamentType type, float value)
    {
        m_Densities[static_cast<size_t>(type)] = value;
    }
    static float GetDensity(FilamentType type)
    {
        return m_Densities[static_cast<size_t>(type)];
    }
    static char * GetTypeString(FilamentType type, char *buf,
                                size_t maxLen = TYPE_STRING_MAX_SIZE)
    {
        strlcpy(buf, TypeStrings[static_cast<size_t>(type)], maxLen);
        return buf;
    }
    static char * GetTypeLString(FilamentType type, char *buf,
                                size_t maxLen = TYPE_LSTRING_MAX_SIZE)
    {
        strlcpy(buf, TypeLStrings[static_cast<size_t>(type)], maxLen);
        return buf;
    }
    static size_t GetNumberFilaments()
    {
        return NUMBER_FILAMENTS;
    }


    /////////////////////////////////////////////////////////////////////////////
    // Public static constants.
    /////////////////////////////////////////////////////////////////////////////
    static const size_t TYPE_STRING_MAX_SIZE  = 5;
    static const size_t TYPE_LSTRING_MAX_SIZE = 7;
    static const float  MAX_DENSITY;
    static const float  MIN_DENSITY;


protected:


private:
    /////////////////////////////////////////////////////////////////////////////
    // Unimplemented methods.  We don't want users to try to use these.
    /////////////////////////////////////////////////////////////////////////////
    Filament(Filament &rCs);
    Filament &operator=(Filament &rCs);


    /////////////////////////////////////////////////////////////////////////////
    // Private static constants.
    /////////////////////////////////////////////////////////////////////////////
    static const uint16_t     NUMBER_FILAMENTS = static_cast<uint16_t>(eFtCount);
    static const char * const TypeStrings[NUMBER_FILAMENTS];
    static const char * const TypeLStrings[NUMBER_FILAMENTS];
    static const char        *pPrefSavedStateLabel;
    static const size_t       MAX_NVS_NAME_LEN;

    const char *m_pName;                // NVS instance name.

    /////////////////////////////////////////////////////////////////////////////
    // The density table.  Each entry corresponds to a filament type.  Values
    // for the table were taken from an article in Nuts/Volts magazine,
    // 2019/issue-4, entitled "Build a 3D Printer Filament Scale".
    /////////////////////////////////////////////////////////////////////////////
    static float m_Densities[NUMBER_FILAMENTS];

}; // End class Filament.



#endif // FILAMENT_H