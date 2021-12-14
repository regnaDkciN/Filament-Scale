/////////////////////////////////////////////////////////////////////////////////
// Network.cpp
//
// Contains miscellaneous methods used by the scale display.
//
// History:
// - jmcorbett 26-DEC-2020 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#include <Preferences.h>
#include "Network.h"


// Some constants used by the class.
const size_t Network::MAX_NVS_NAME_LEN     = 15U;
const char  *Network::pPrefSavedStateLabel = "Saved State";


/////////////////////////////////////////////////////////////////////////////////
// Constructor
//
// Initialize our instance data.
/////////////////////////////////////////////////////////////////////////////////
Network::Network(int serverPort) : WebServer(serverPort), m_pName(NULL),
                                  m_WiFiManager(), m_pApName(NULL),
                                  m_pServerName(NULL), m_Connected(false)
{
} // End constructor.


/////////////////////////////////////////////////////////////////////////////////
// Init()
//
// This method initializes the spool collection.
//
// Arguments:
//    - pName       - A string of no more than 15 characters to be used as a
//                    name for this instance.  This is mainly used to identify
//                    the instance to be used for NVS save and restore.
//    - pApName     - This is the network name to be used for the access point.
//    - pServerName - This is the network name to be used for the server.
//
// Returns:
//    Returns a bool indicating whether or not the initialization was
//    successful.  A 'true' value indicates success, while a 'false' value
//    indicates failure.
//
/////////////////////////////////////////////////////////////////////////////////
bool Network::Init(const char *pName, const char *pApName = NULL,
                   const char *pServerName = NULL)
{
    // Assume we're gonna fail.
    bool status = false;

    // Make sure the name is valid.  If not then we fail.
    if ((pName != NULL) && (*pName != '\0') && (strlen(pName) <= MAX_NVS_NAME_LEN))
    {
        // Name was OK.  Save it.
        m_pName = pName;

        // Setup our AP name if one was given.
        if (pApName != NULL)
        {
            MDNS.begin(pApName);
        }

        // Setup the wifi manager.
        m_WiFiManager.setCaptivePortalEnable(false);
        m_WiFiManager.setCleanConnect(true);
        m_WiFiManager.setShowInfoErase(false);
        m_WiFiManager.setConfigPortalBlocking(false);

        // Attempt to connect to the network.
        m_Connected = m_WiFiManager.autoConnect(pApName);

        // If we're connected, then change our network mdns id and start the server.
        if (m_Connected)
        {
            // Use the passed in name for our net name.  Access via
            // pServerName.local.  For example, if pServerName points to the
            // following string:
            //    "MyDevice"
            // then a browser can find the device via the following:
            //    "http://MyDevice.local".
            MDNS.begin(pServerName);

            // Start the web server.
            WebServer::begin();
        }

        //  Remember that we succeeded.
        status = true;
    }
    return status;
} // End Init().


/////////////////////////////////////////////////////////////////////////////////
// Process()
//
// This method handles the non-blocking wifi manager completion.
//
// Returns:
//    Returns a bool indicating whether or not the network is now connected.
//    A 'true' value indicates connected, while a 'false' value
//    indicates not connected.
//
// NOTE: If a new connection was just establiched we will not return from this
//       method.  Instead we will reset the ESP32.  (See the commented code
//       below).
//
/////////////////////////////////////////////////////////////////////////////////
bool Network::Process()
{
    // If we're not yet connected, then call the wifi manager to see if a
    // connection was recently made.
    if (!m_Connected)
    {
        if (m_WiFiManager.process())
        {
            // Just connected which means that we need to reset the network.
            // This is a kludge, but it is the only way I could get a new
            // connection to take effect on the ESP32.  Here we delay in order
            // to allow the wifi manager to complete its NVS save, then we
            // reset the system.  When we come back up, we should be connected
            // to the new network.
            delay(1000);
            ESP.restart();
            delay(1000);
        }
    }
    else
    {
        WebServer::handleClient();
    }
    return m_Connected;
}


/////////////////////////////////////////////////////////////////////////////
// ResetCredentials()
//
// This method resets/clears any saved network connection credentials
// (network SSID and password) that may have been previously saved.
// The next reboot will cause the wifi manager access point to execute and
// a new set of credentials will be needed.
//
/////////////////////////////////////////////////////////////////////////////
void Network::ResetCredentials()
{
    m_WiFiManager.resetSettings();
}


/////////////////////////////////////////////////////////////////////////////////
// Save()
//
// Saves our current state to NVS.
//
// Returns:
//    Returns 'true' if successful, or 'false' otherwise.
/////////////////////////////////////////////////////////////////////////////////
bool Network::Save() const
{
    size_t saved = 0;
/*
    if (m_pName != NULL)
    {
        // Read our currently saved state.  If it hasn't changed, then don't
        // bother to do the save in order to conserve writes to NVS.
        uint32_t nvsState;

        // Save our state data.
        Preferences prefs;
        prefs.begin(m_pName);

        // Get the saved data.
        size_t nvsSize =
            prefs.getBytes(pPrefSavedStateLabel, &nvsState, sizeof(uint32_t));

        // See if our working data has changed since our last save.
        if ((nvsSize != sizeof(uint32_t)) ||
            memcmp(&nvsState, &m_BacklightPercent, sizeof(uint32_t)))
        {
            // Data has changed so go ahead and save it.
            Serial.println("\nDisplay - saving to NVS.");
            saved =
                prefs.putBytes(pPrefSavedStateLabel, &m_BacklightPercent, sizeof(uint32_t));
        }
        else
        {
            // Data has not changed.  Do nothing.
            saved = sizeof(uint32_t);
            Serial.println("\nDisplay - not saving to NVS.");
        }
        prefs.end();
    }
  */
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
bool Network::Restore()
{
    // Assume we're gonna fail.
    bool succeeded = false;
/*
    // Make sure we have a valid name.
    if (m_pName != NULL)
    {
        // Restore our state data to a temporary structure.
        uint32_t cachedState;
        Preferences prefs;
        prefs.begin(m_pName);
        size_t restored =
            prefs.getBytes(pPrefSavedStateLabel, &cachedState, sizeof(uint32_t));

        // Save the restored values only if the get was successful.
        if (restored == sizeof(uint32_t))
        {
            SetBacklightPercent(cachedState);

            succeeded = true;
        }
        prefs.end();
    }
*/
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
bool Network::Reset()
{
    bool status = false;

    // Erase all stored information in the wifi manager.
    ResetCredentials();
/*
    if (m_pName != NULL)
    {
        // Remove our state data rom NVS.
        Preferences prefs;
        prefs.begin(m_pName);
        status = prefs.remove(pPrefSavedStateLabel);
        prefs.end();
    }
*/
    return status;
} // End Reset().


