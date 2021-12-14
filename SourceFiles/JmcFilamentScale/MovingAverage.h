/////////////////////////////////////////////////////////////////////////////////
// MovingAverage.h
//
// This class implements the MovingAverage template class which calculates a
// moving average of up to MAX_SIZE values.  This template is very loosly based
// on code by Tony Delroy on stackoverflow.com:
//    https://stackoverflow.com/questions/10990618/calculate-rolling-moving-average-in-c
// For this version, it was required that the number of the averaged samples be
// modifiable.
//
// History:
// - jmcorbett 18-OCT-2021 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////
#if !defined MOVINGAVERAGE_H
#define MOVINGAVERAGE_H

#include <cstddef>      // For size_t.
#include <algorithm>    // For std::min().



/////////////////////////////////////////////////////////////////////////////////
// MovingAverage template class
//
// Arguments:
// - V  - The class used to store values.
// - T  - The class used to maintain the running total value.
/////////////////////////////////////////////////////////////////////////////////
template<class V, class T>
class MovingAverage
{
public:
    /////////////////////////////////////////////////////////////////////////////
    // Construct and initialize the class.
    //
    // Arguments:
    //    - size - This specifies the number of samples to average for the moving
    //             average.  Valid range is from MIN_SIZE to MAX_SIZE.
    /////////////////////////////////////////////////////////////////////////////
    MovingAverage(size_t size = MIN_SIZE) : m_NumValues(0), m_Size(0), m_Total(0)
    {
        SetSize(size);
    } // End constructor.




    /////////////////////////////////////////////////////////////////////////////
    // Add()
    //
    // This method adds a value to the running total, and handles removing
    // older values.
    //
    // Arguments:
    //    - val   - The value to be added to the running total.
    //
    // Returns:
    //    Returns the moving running total of the values entered so far.
    //
    /////////////////////////////////////////////////////////////////////////////
    T Add(V val)
    {
        // Add the value to the running total.
        m_Total += val;

        // Do we need to wrap yet?
        if (m_NumValues < m_Size)
        {
            // No, so just add the value to our value array.
            m_Values[m_NumValues++] = val;
        }
        else
        {
            // Yes, remove the oldest value from our array and add the new value.
            V &oldest = m_Values[m_NumValues++ % m_Size];
            m_Total -= oldest;
            oldest = val;
        }

        // Return our running total.
        return m_Total;
    } // End Add().


    /////////////////////////////////////////////////////////////////////////////
    // Average()
    //
    // This method returns the moving average of the values added so far.
    //
    // Returns:
    //    Returns the moving average of the values added so far.
    /////////////////////////////////////////////////////////////////////////////
    V Average() const
    {
        return m_Total / std::min(m_NumValues, m_Size);
    } // End Average().


    /////////////////////////////////////////////////////////////////////////////
    // Total()
    //
    // This method returns the moving total of the values entered so far..
    //
    // Returns:
    //    Returns the moving total of the values added so far.
    /////////////////////////////////////////////////////////////////////////////
    T Total() const { return m_Total; }


    /////////////////////////////////////////////////////////////////////////////
    // Reset()
    //
    // This method resets the total and index to re-start the moving average.
    /////////////////////////////////////////////////////////////////////////////
    void Reset()
    {
        m_Total = 0;
        m_NumValues = 0;
    } // End Reset().


    /////////////////////////////////////////////////////////////////////////////
    // SetSize()
    //
    // This method (possibly) changes the number of values to average.
    //
    // Arguments:
    //  - newSize - This is the desired size of the averaging interval.  Legal
    //              values range from MIN_SIZE to MAX_SIZE inclusive.
    //
    // Returns:
    //   Returns the (possibly) new size of the averaging interval, which may
    //   have been limited to a range between MIN_SIZE and MAX_SIZE includive.
    /////////////////////////////////////////////////////////////////////////////
    size_t SetSize(size_t newSize)
    {
        // Clip the new size to fit within the valid range.
        if (newSize < MIN_SIZE)
        {
            newSize = MIN_SIZE;
        }
        else if (newSize > MAX_SIZE)
        {
            newSize = MAX_SIZE;
        }

        // If the new size has changed, then remember it, and reset the
        // averaging mechanism.
        if (newSize != m_Size)
        {
            m_Size = newSize;
            Reset();
        }

        // Return the (possibly) new size value.
        return m_Size;
    } // End SetSize().


    /////////////////////////////////////////////////////////////////////////////
    // Size()
    //
    // This method returns the number of items for the interval of the moving
    // average.
    //
    // Returns:
    //   Returns the number of items for the interval of the moving average.
    /////////////////////////////////////////////////////////////////////////////
    size_t Size() const { return m_Size; }

private:
    static const size_t MAX_SIZE = 100; // Maximum interval for moving average.
    static const size_t MIN_SIZE = 1;   // Minimum interval for moving average.

    V         m_Values[MAX_SIZE];       // Array of values.
    size_t    m_NumValues;              // Number of values entered so far.
    size_t    m_Size;                   // Size of the averaging interval.
    T         m_Total;                  // Total of the values entered so far.

}; // End class MovingAverage.


#endif // MOVINGAVERAGE_H