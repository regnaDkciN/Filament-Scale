/////////////////////////////////////////////////////////////////////////////////
// Network.h
//
// This class implements the Network class.  It handles miscellaneous scale
// related network connection tasks.
//
// History:
// - jmcorbett 26-DEC-2020 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#if !defined NETWORK_H
#define NETWORK_H

#include <ESPmDNS.h>            // For Mdns support.
#include <WiFiManager.h>        // Manage connection. https://github.com/tzapu/WiFiManager
#include <WebServer.h>          // For web server handling.


/////////////////////////////////////////////////////////////////////////////////
// Network class
//
// Handles miscellaneous network/wifi tasks.
/////////////////////////////////////////////////////////////////////////////////
class Network : public WebServer
{
public:
    /////////////////////////////////////////////////////////////////////////////
    // Constructors.
    //
    // Initialize our instance data.
    //
    // Arguments:
    //  - serverPort - Port used by server.
    /////////////////////////////////////////////////////////////////////////////
    Network(int serverPort = DEFAULT_SERVER_PORT);


    /////////////////////////////////////////////////////////////////////////////
    // Simple destructor.
    /////////////////////////////////////////////////////////////////////////////
    ~Network() {}


    /////////////////////////////////////////////////////////////////////////////
    // Init()
    //
    // This method initializes the network.
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
    /////////////////////////////////////////////////////////////////////////////
    bool Init(const char *pName, const char *pApName, const char *pServerName);


    /////////////////////////////////////////////////////////////////////////////
    // Process()
    //
    // This method handles the non-blocking wifi manager completion.
    //
    // Returns:
    //    Returns a bool indicating whether or not the network is now connected.
    //    A 'true' value indicates connected, while a 'false' value
    //    indicates not connected.
    //
    // NOTE: If a new connection was just establiched we will not return from
    //       this method.  Instead we will reset the ESP32.  (See the commented
    //       code in Network.cpp).
    //
    /////////////////////////////////////////////////////////////////////////////
    bool Process();


    /////////////////////////////////////////////////////////////////////////////
    // ResetCredentials()
    //
    // This method resets/clears any saved network connection credentials
    // (network SSID and password) that may have been previously saved.
    // The next reboot will cause the wifi manager access point to execute and
    // a new set of credentials will be needed.
    //
    /////////////////////////////////////////////////////////////////////////////
    void ResetCredentials();


    /////////////////////////////////////////////////////////////////////////////
    // Simple getters.
    /////////////////////////////////////////////////////////////////////////////


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


    // Simple getters and setters.
    bool IsConnected() const { return m_Connected; }


protected:


private:
    /////////////////////////////////////////////////////////////////////////////
    // Unimplemented methods.  We don't want users to try to use these.
    /////////////////////////////////////////////////////////////////////////////
    Network(Network &rCs);
    Network &operator=(Network &rCs);


    /////////////////////////////////////////////////////////////////////////////
    // Private static constants.
    /////////////////////////////////////////////////////////////////////////////
    static const char    *pPrefSavedStateLabel;
    static const size_t   MAX_NVS_NAME_LEN;
    static const int      DEFAULT_SERVER_PORT = 80;


    /////////////////////////////////////////////////////////////////////////////
    // Private instance data.
    /////////////////////////////////////////////////////////////////////////////
    const char *m_pName;                // NVS instance name.
    WiFiManager m_WiFiManager;          // Sets up IP address.
    const char *m_pApName;              // Access point mdns network name.
    const char *m_pServerName;          // Server mdns network name.
    bool        m_Connected;            // Set if server is connected to net.

}; // End class Network.



#endif // NETWORK_H