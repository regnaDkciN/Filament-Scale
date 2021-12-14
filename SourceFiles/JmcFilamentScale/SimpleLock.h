/////////////////////////////////////////////////////////////////////////////////
// SimpleLock.h
//
// This class implements a simple interlocking mechanism (mutex).  Its original
// purpose was to support mutual exclusion between a web client and a web server
// with the web server residing on an ESP32 processor.
//
// Note that this is an extremely elementary implementation which does not
// support most of the features of a real mutex.  Among the simplifications are:
//    - Doesn't support nesting of the mutex.
//    - Doesn't check for ownership.  If the mutex is locked by one entity,
//      the other entity may unlock it.  This requires close coordination between
//      both entities using it.
//
// History:
// - jmcorbett 28-MAR-2021 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#if !defined SIMPLELOCK_H
#define SIMPLELOCK_H

#include <Arduino.h>    // For interrupt control.



/////////////////////////////////////////////////////////////////////////////////
// SimpleLock class
//
// Handles simple mutex.
/////////////////////////////////////////////////////////////////////////////////
class SimpleLock
{
public:
    /////////////////////////////////////////////////////////////////////////////
    // Constructors.
    //
    // Initialize our instance data.  Start with the mutex unowned.
    //
    // Arguments:
    /////////////////////////////////////////////////////////////////////////////
    SimpleLock() : m_Owner(UNOWNED) {}


    /////////////////////////////////////////////////////////////////////////////
    // Simple destructor.
    /////////////////////////////////////////////////////////////////////////////
    ~SimpleLock() {}


    /////////////////////////////////////////////////////////////////////////////
    // Lock()
    //
    // This method tries to acquire.  If successful, returns 'true'.  Otherwise
    // returns false.
    //
    // Arguments:
    //   - owner - Integer value corresponding to requested owner id.
    //
    // Returns:
    //    Returns a bool indicating whether or not the lock was successful.
    //
    /////////////////////////////////////////////////////////////////////////////
    bool Lock(uint32_t owner)
    {
        bool status = false;
        noInterrupts();
        if (m_Owner == UNOWNED)
        {
            m_Owner = owner;
            status = true;
        }
        interrupts();
        return status;
    }


    /////////////////////////////////////////////////////////////////////////////
    // Unlock()
    //
    // This method frees a lock.
    //
    // NOTE: This method doesn't verify that the requestor owns the lock.  It
    //       blindly unlocks it.
    //
    /////////////////////////////////////////////////////////////////////////////
    void Unlock() { m_Owner = UNOWNED; }


    /////////////////////////////////////////////////////////////////////////////
    // Simple getters.
    /////////////////////////////////////////////////////////////////////////////
    bool IsLocked() const { return (m_Owner != UNOWNED); }
    uint32_t Owner() const { return m_Owner; }


    /////////////////////////////////////////////////////////////////////////////
    // Public constants
    /////////////////////////////////////////////////////////////////////////////
    static const uint32_t UNOWNED = 0;


protected:


private:
    /////////////////////////////////////////////////////////////////////////////
    // Unimplemented methods.  We don't want users to try to use these.
    /////////////////////////////////////////////////////////////////////////////
    SimpleLock(SimpleLock &rCs);
    SimpleLock &operator=(SimpleLock &rCs);


    /////////////////////////////////////////////////////////////////////////////
    // Private instance data.
    /////////////////////////////////////////////////////////////////////////////
    uint32_t m_Owner;                // UNOWNED if not locked.

}; // End class SimpleLock.



#endif // SIMPLELOCK_H