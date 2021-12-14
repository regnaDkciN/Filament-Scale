/////////////////////////////////////////////////////////////////////////////////
// SpoolManager.h
//
// This class implements the SpoolManager class.  It maintains the collection
// of spools, and manages the selection of a particular spool.
//
// History:
// - jmcorbett 14-DEC-2020 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#if !defined SPOOLMANAGER_H
#define SPOOLMANAGER_H

#include <Preferences.h>
#include "Spool.h"          // For Spool class.


/////////////////////////////////////////////////////////////////////////////////
// SpoolManager class
//
// Handles all collection and selection of spools.  This is a templated class
// in which the single template argument 'N' represents the number of spools
// managed by the class.
/////////////////////////////////////////////////////////////////////////////////
template <size_t N>
class SpoolManager
{
public:
    /////////////////////////////////////////////////////////////////////////////
    // Constructor.
    //
    // Sets default values for all instance data.  Uses the DEFAULT_SPOOL_NAME to
    // create a unique name for the instance by appending the instance number.
    /////////////////////////////////////////////////////////////////////////////
    SpoolManager() : m_pName(NULL), m_NumSpools(N),
                    m_SelectedSpoolIndex(NO_SPOOL_SELECTED_INDEX)
    {
    } // End constructor.


    /////////////////////////////////////////////////////////////////////////////
    // Simple destructor.
    /////////////////////////////////////////////////////////////////////////////
    ~SpoolManager() {}


    /////////////////////////////////////////////////////////////////////////////
    // Init()
    //
    // This method initializes the spool collection.
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
    bool Init(const char *pName);


    /////////////////////////////////////////////////////////////////////////////
    // GetSelectedSpool()
    //
    // This method returns a pointer to the currently selected spool.
    //
    // Returns:
    //    Returns a pointer to the selected spool, or NULL if none is selected.
    //
    /////////////////////////////////////////////////////////////////////////////
    Spool *GetSelectedSpool();


    /////////////////////////////////////////////////////////////////////////////
    // GetSpool()
    //
    // This method returns a pointer to the specified spool.
    //
    // Arguments:
    //    - index - The index of the spool to return.  Must be between 0 and N.
    //
    // Returns:
    //    Returns a pointer to the specified spool, or NULL if the index is
    //    reater than N.
    //
    /////////////////////////////////////////////////////////////////////////////
    Spool *GetSpool(uint32_t index);


    /////////////////////////////////////////////////////////////////////////////
    // IsSelected()
    //
    // This method returns an indication of whether or not the specified spool is
    // currently selected.
    //
    // Arguments:
    //    - index - The index of the spool to be checked.
    //
    // Returns:
    //    Returns 'true' if the specified spool is currently selected, or false
    //    otherwise.
    //
    /////////////////////////////////////////////////////////////////////////////
    bool  IsSelected(uint32_t index) const;


    /////////////////////////////////////////////////////////////////////////////
    // SelectSpool()
    //
    // This method selects a spool as the active spool.
    //
    // Arguments:
    //    - index - The index of the spool to be selected.  Must be between
    //              0 and N.  If index is out of range, nothing changes
    //              (i.e. the currently selected spool remains unchanged).
    //
    // Returns:
    //    Returns a pointer to the newly selected spool, or NULL if indes is out
    //    of range.  In the latter case, the selected spool remains unchanged.
    //
    /////////////////////////////////////////////////////////////////////////////////
    Spool *SelectSpool(uint32_t index);


    /////////////////////////////////////////////////////////////////////////////
    // DeselectSpool()
    //
    // This method deselects all spools.
    //
    /////////////////////////////////////////////////////////////////////////////
    void DeselectSpool();


    /////////////////////////////////////////////////////////////////////////////
    // Simple getters.
    /////////////////////////////////////////////////////////////////////////////
    bool     IsInitialized()         const { return m_pName != NULL; }
    uint32_t GetSelectedSpoolIndex() const { return m_SelectedSpoolIndex; }
    uint32_t GetNumberOfSpools()     const { return N; }

    /////////////////////////////////////////////////////////////////////////////
    // Simple setters.  Each returns true if successful or false otherwise.
    /////////////////////////////////////////////////////////////////////////////

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


protected:


private:
    /////////////////////////////////////////////////////////////////////////////
    // Unimplemented methods.  We don't want users to try to use these.
    /////////////////////////////////////////////////////////////////////////////
    SpoolManager(SpoolManager &rSm);
    SpoolManager &operator=(SpoolManager &rSm);


    /////////////////////////////////////////////////////////////////////////////
    // Private static constants.
    /////////////////////////////////////////////////////////////////////////////
    static const char    *pPrefSavedStateLabel;
    static const uint32_t NO_SPOOL_SELECTED_INDEX = 9999;
    static const size_t   MAX_NVS_NAME_LEN;


    /////////////////////////////////////////////////////////////////////////////
    // Instance data.
    /////////////////////////////////////////////////////////////////////////////
    const char *m_pName;                // NVS instance name.

    uint32_t    m_NumSpools;            // Number of spools.
    Spool       m_Spools[N];            // Spool array.
    uint32_t    m_SelectedSpoolIndex;   // Index of selected spool.

    /////////////////////////////////////////////////////////////////////////////
    // Structure for saving/restoring to/from NVS.
    /////////////////////////////////////////////////////////////////////////////
    struct NvsSaveBuffer
    {
        uint32_t    m_NumSpools;            // Number of spools.
        Spool       m_Spools[N];            // Spool array.
        uint32_t    m_SelectedSpoolIndex;   // Index of selected spool.
    };

}; // End class SpoolManager.




/////////////////////////////////////////////////////////////////////////////////
// SpoolManager class constants.  Should be self explanatory.
/////////////////////////////////////////////////////////////////////////////////
template <size_t N>
    const char *SpoolManager<N>::pPrefSavedStateLabel = "Saved State";
template <size_t N>
    const size_t SpoolManager<N>::MAX_NVS_NAME_LEN = 15U;


/////////////////////////////////////////////////////////////////////////////////
// Init()
//
// This method initializes the spool collection.
//
// Arguments:
//    - pName   - A string of no more than 15 characters to be used as a
//                name for this instance.  This is mainly used to identify
//                the instance to be used for NVS save and restore.
//    - numSpools - Number of spools to create.
//
// Returns:
//    Returns a bool indicating whether or not the initialization was successful.
//    A 'true' value indicates success, while a 'false' value indicates failure.
//
/////////////////////////////////////////////////////////////////////////////////
template <size_t N>
bool SpoolManager<N>::Init(const char *pName)
{
    bool status = false;

    if (!IsInitialized() && (pName != NULL) &&
        (*pName != '\0') && (strlen(pName) <= MAX_NVS_NAME_LEN))
    {
        m_pName = pName;
        status = true;
    }
    return status;
} // End Init().


/////////////////////////////////////////////////////////////////////////////////
// GetSelectedSpool()
//
// This method returns a pointer to the currently selected spool.
//
// Returns:
//    Returns a pointer to the currently selected spool, or NULL if none is
//    selected.
//
/////////////////////////////////////////////////////////////////////////////////
template <size_t N>
Spool *SpoolManager<N>::GetSelectedSpool()
{
    Spool *pSpool = NULL;
    if (m_SelectedSpoolIndex < N)
    {
        pSpool = &m_Spools[m_SelectedSpoolIndex];
    }
    return pSpool;
} // End GetSelectedSpool().


/////////////////////////////////////////////////////////////////////////////////
// GetSpool()
//
// This method returns a pointer to the specified spool.
//
// Arguments:
//    - index - The index of the spool to return.  Must be between 0 and N.
//
// Returns:
//    Returns a pointer to the specified spool, or NULL if the index is greater
//    than N.
//
/////////////////////////////////////////////////////////////////////////////////
template <size_t N>
Spool *SpoolManager<N>::GetSpool(uint32_t index)
{
    Spool *pSpool = NULL;
    if (index < N)
    {
        pSpool = &m_Spools[index];
    }
    return pSpool;
} // End GetSpool().


/////////////////////////////////////////////////////////////////////////////////
// IsSelected()
//
// This method returns an indication of whether or not the specified spool is
// currently selected.
//
// Arguments:
//    - index - The index of the spool to be checked.
//
// Returns:
//    Returns 'true' if the specified spool is currently selected, or false
//    otherwise.
//
/////////////////////////////////////////////////////////////////////////////////
template <size_t N>
bool  SpoolManager<N>::IsSelected(uint32_t index) const
{
    bool status = false;
    if ((index < N) && (m_SelectedSpoolIndex < N) &&
        (index == m_SelectedSpoolIndex))
    {
        status = true;
    }
    return status;
} // End IsSelected().


/////////////////////////////////////////////////////////////////////////////////
// SelectSpool()
//
// This method selects a spool as the active spool.
//
// Arguments:
//    - index - The index of the spool to be selected.  Must be between 0 and N.
//              If index is out of range, nothing changes (i.e. the currently
//              selected spool remains unchanged).
//
// Returns:
//    Returns a pointer to the newly selected spool, or NULL if indes is out
//    of range.  In the latter case, the selected spool remains unchanged.
//
/////////////////////////////////////////////////////////////////////////////////
template <size_t N>
Spool *SpoolManager<N>::SelectSpool(uint32_t index)
{
    Spool *pSpool = NULL;
    if (index < N)
    {
        pSpool = &m_Spools[index];
        m_SelectedSpoolIndex = index;
    }
    return pSpool;
} // End SelectSpool().


/////////////////////////////////////////////////////////////////////////////////
// DeselectSpool()
//
// This method deselects all spools.
//
/////////////////////////////////////////////////////////////////////////////////
template <size_t N>
void SpoolManager<N>::DeselectSpool()
{
    m_SelectedSpoolIndex = NO_SPOOL_SELECTED_INDEX;
} // End DeselectSpool().


/////////////////////////////////////////////////////////////////////////////////
// Save()
//
// Saves our current state to NVS.
//
// Returns:
//    Returns 'true' if successful, or 'false' otherwise.
/////////////////////////////////////////////////////////////////////////////////
template <size_t N>
bool SpoolManager<N>::Save() const
{
    size_t saved = 0;
    if (m_pName != NULL)
    {
        // Cache our state information for transfer to NVS as a single unit.
        NvsSaveBuffer nvsBuffer;
        NvsSaveBuffer ramBuffer;

        // Copy our local data into a buffer for later comparison.
        ramBuffer.m_NumSpools = m_NumSpools;
        memcpy(ramBuffer.m_Spools, m_Spools, sizeof(m_Spools));
        ramBuffer.m_SelectedSpoolIndex = m_SelectedSpoolIndex;

        // Prepare for NVS use.
        Preferences prefs;
        prefs.begin(m_pName);

        // Fetch our currently saved NVS data for comparison.
        NvsSaveBuffer nvsState;
        size_t nvsSize =
            prefs.getBytes(pPrefSavedStateLabel, &nvsBuffer,
                           sizeof(NvsSaveBuffer));

        // Compare the saved and current data.  If it hasn't changed, then
        // we don't need to save anything (saving NVS writes).
        if ((nvsSize != sizeof(NvsSaveBuffer)) ||
            memcmp(&nvsBuffer, &ramBuffer, sizeof(NvsSaveBuffer)))
        {
            // NVS and working didn't match, so save the working.
            saved =
                prefs.putBytes(pPrefSavedStateLabel, &ramBuffer,
                               sizeof(NvsSaveBuffer));
            Serial.println("\nSpoolManager - saving to NVS.");
        }
        else
        {
            // NVS matched working.  Simply setup to return success.
            saved = sizeof(NvsSaveBuffer);
            Serial.println("\nSpoolManager - not saving to NVS");
        }
        prefs.end();
    }

    // Let the caller know if we succeeded or failed.
    return saved == sizeof(NvsSaveBuffer);
 } // End Save().


/////////////////////////////////////////////////////////////////////////////////
// Restore()
//
// Restores our state from NVS.
//
// Returns:
//    Returns 'true' if successful, or 'false' otherwise.
/////////////////////////////////////////////////////////////////////////////////
template <size_t N>
bool SpoolManager<N>::Restore()
{
    // Assume we're gonna fail.
    bool succeeded = false;

    // Make sure we have a valid name.
    if (m_pName != NULL)
    {
        // Restore our state data to a temporary structure.
        NvsSaveBuffer nvsBuffer;

        Preferences prefs;
        prefs.begin(m_pName);
        size_t restored =
            prefs.getBytes(pPrefSavedStateLabel, &nvsBuffer, sizeof(NvsSaveBuffer));

        // Save the restored values only if the get was successful.
        if ((restored == sizeof(NvsSaveBuffer)) &&
            (nvsBuffer.m_NumSpools == N))
        {
            // Copy our nvs data to our instance data.
            m_NumSpools = nvsBuffer.m_NumSpools;
            memcpy(m_Spools, nvsBuffer.m_Spools, sizeof(m_Spools));
            m_SelectedSpoolIndex = nvsBuffer.m_SelectedSpoolIndex;
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
template <size_t N>
bool SpoolManager<N>::Reset()
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


#endif // SPOOLMANAGER_H
