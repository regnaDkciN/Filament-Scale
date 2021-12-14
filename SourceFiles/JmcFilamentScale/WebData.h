/////////////////////////////////////////////////////////////////////////////////
// WebData.h
//
// This file supports handling of Web requests.
//
// History:
// - jmcorbett 01-JUN-2021 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////
#if !defined WEBDATA_H
#define WEBDATA_H


namespace WebData
{
    /////////////////////////////////////////////////////////////////////////////
    // InitNetworkHandlers()
    //
    // Called at power-up to initialize the handling of client requests.  Sets up
    // a handler for each of the URLs that are expected to be received from the
    // network client.
    /////////////////////////////////////////////////////////////////////////////
    void InitNetworkHandlers();


    /////////////////////////////////////////////////////////////////////////////
    // HandleWebTimeout()
    //
    // The web lock is used to insure that a web client and the local user cannot
    // attempt to modify any options at the same time.  When the web client has
    // an options page displayed, it takes ownership of the web lock.  This
    // prevents the local user from entering the options menu.  If the web client
    // goes away while owning the web lock, this could prevent the local user
    // from ever being able to enter the options menu.  To prevent this, each
    // time an update messsage is received from a web client, we remember the
    // time at which it arrives.  The main loop calls this function every
    // iteration to verify that the client connection is still alive.  If too
    // much time has passed, then it is assumed that the connection is lost, so
    // if the web client was the owner of the lock, it is unlocked.
    /////////////////////////////////////////////////////////////////////////////
    void HandleWebTimeout();

    /////////////////////////////////////////////////////////////////////////////
    // Local locking/unlocking functions.
    /////////////////////////////////////////////////////////////////////////////
    bool LockLocal();       // Lock the local screen for options setting.
    bool IsLocalOwner();    // True if local is current owner of lock.
    void Unlock();          // Unlock the lock.

} // End namespace WebData.


#endif // WEBDATA_H