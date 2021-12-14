/////////////////////////////////////////////////////////////////////////////////
// Display.h
//
// This class implements the Display class.  It handles miscellaneous scale
// related display tasks.
//
// History:
// - jmcorbett 26-DEC-2020 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#if !defined DISPLAY_H
#define DISPLAY_H

#include <Adafruit_ST7735.h>    // For Adafruit 1.8" TFT display.


/////////////////////////////////////////////////////////////////////////////////
// BoxLocale
//
// This enum specifies valid values for the side in which a box may reside.
/////////////////////////////////////////////////////////////////////////////////
enum BoxLocale
{
    eLeft   = 0,    // Box is half width and starts on the left.
    eRight  = 1,    // Box is half width and starts at the center.
    eAll    = 2     // Box is full width of the display.
}; // End BoxLocale.


/////////////////////////////////////////////////////////////////////////////////
// DisplayState class
//
// Handles saving and restoring of display state data.  This data consists of
// text X and Y size, and foreground and background color.
/////////////////////////////////////////////////////////////////////////////////
class DisplayState
{
public:
    DisplayState(uint8_t x, uint8_t y, uint16_t f, uint16_t b) :
        m_SavedTextSizeX(x), m_SavedTextSizeY(y),
        m_SavedFgColor(f), m_SavedBgColor(b)
    {};

    void RestoreState(uint8_t &x, uint8_t &y, uint16_t &f, uint16_t &b)
    {
        x = m_SavedTextSizeX;
        y = m_SavedTextSizeY;
        f = m_SavedFgColor;
        b = m_SavedBgColor;
    }

private:
    DisplayState();

    uint8_t     m_SavedTextSizeX;       // Saved text X size.
    uint8_t     m_SavedTextSizeY;       // Saved text Y size.
    uint16_t    m_SavedFgColor;         // Saved foreground color.
    uint16_t    m_SavedBgColor;         // Saved background color.
}; // End DisplayState.


/////////////////////////////////////////////////////////////////////////////////
// Display class
//
// Handles miscellaneous display tasks.
/////////////////////////////////////////////////////////////////////////////////
class Display : public Adafruit_ST7735
{
public:
    /////////////////////////////////////////////////////////////////////////////
    // Constructor.
    //
    // Initialize our instance data and setup for use of the backlight and
    // initializes the TFT hardware and library with a blank screen.
    //
    // Arguments:
    //    - csPin        - Display's CS pin.
    //    - dcPin        - Display's DC pin.
    //    - rstPin       - Display's RST pin.
    //    - backlightPin - Display's backlight pin.
    //    - displayType  - Display type identified by colored tab on the display's
    //                     plastic overlay.  (INITR_GREENTAB, INITR_REDTAB, or
    //                     INITR_BLACKTAB).
    //    - rotation     - Display's rotation orientation  (0, 1, 2, or 3).
    /////////////////////////////////////////////////////////////////////////////
    Display(int csPin, int dcPin, int rstPin, int backlightPin,
            uint8_t displayType = INITR_BLACKTAB, uint8_t rotation = 1);


    /////////////////////////////////////////////////////////////////////////////
    // Simple destructor.
    /////////////////////////////////////////////////////////////////////////////
    ~Display() {}


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
    // SetBacklightPercent()
    //
    // Sets the backlight level to a percentage of its maximum brightness.
    //
    // Arguments:
    //    - percent - This is the percentage of maximum brightness which the
    //                backlight will be set to.  Valid values are 0 through
    //                100 inclusive.
    //
    // Returns:
    //    Returns 'true' if successful.  Returns 'false' if an invalid 'level'
    //    value was passed.
    /////////////////////////////////////////////////////////////////////////////
    bool SetBacklightPercent(uint32_t percent);


    /////////////////////////////////////////////////////////////////////////////////
    // FillScreen()
    //
    // Fills the screen with a (possibly) outlined box with a given radius.
    //
    // Arguments:
    //    - fgColor - Specifies the outline color of the box.
    //    - bgColor - Specifies the background color of the box.
    //    - radius  - Radius in pixels for rounded box.
    /////////////////////////////////////////////////////////////////////////////////
    void FillScreen(uint16_t fgColor, uint16_t bgColor, int radius);


    /////////////////////////////////////////////////////////////////////////////
    // DisplayBox()
    //
    // Draws a box of the specified size on the screen at the specified location.
    //
    // Arguments:
    //    - line    - The line at which the box will be displayed.  Valid values
    //                are 0, 1, and 2.
    //    - side    - Specifies the horizontal location of the box as well
    //                as its the width.  Valid values are:
    //                  o eAll   - the box starts at the left edge of the
    //                             display and extends to the right edge of
    //                             the display screen.
    //                  o eLeft  - the box starts at the left edge of the
    //                             display and extends half the width of the
    //                             display screen.
    //                  o eRight - the box starts at the horizontal middle of
    //                             display and extend it to the right edge of
    //                             the display screen.
    //    - fgColor - The color of the outline that will be drawn around the box.
    //                Setting this to the same value as bgColor will cause no
    //                outline to be seen on the display (although one will be
    //                drawn).
    //    - bgColor - The color of the box that will be drawn.
    //    - radius  - The radius, in pixels, of the rounded corners of the box.
    //                A value of zero will cause the box to have square corners.
    /////////////////////////////////////////////////////////////////////////////
    void DisplayBox(int line, BoxLocale side, uint16_t fgColor,
                    uint16_t bgColor, int radius = 0);


    /////////////////////////////////////////////////////////////////////////////
    // DisplayBoxHeader()
    //
    // Displays small (header) text in a specified box.
    //
    // Arguments:
    //    - pStr    - The text string to be displayed as a header in the
    //                specified box.
    //    - line    - The line of the box containing the header text to be
    //                displayed.  Valid values are 0, 1, and 2.
    //    - side    - Specifies the horizontal location of the box as well
    //                as its the width.  Valid values are:
    //                  o eAll   - the box starts at the left edge of the
    //                             display and extends to the  right edge of
    //                             the display screen.
    //                  o eLeft  - the box starts at the left edge of the
    //                             display and extends half the width of the
    //                             display screen.
    //                  o eRight - the box starts at the horizontal middle of
    //                             display and extend it to the right edge of
    //                             the display screen.
    //    - fgColor - The color of the text that will be displayed.
    //    - bgColor - The color of the box that will hold the text string.
    //    - margin  - The number of pixels to indent the text from the left edge
    //                of the box.  This is generally set to the save value as
    //                the 'radius' value that was used to create the box via
    //                DisplayBox().
    /////////////////////////////////////////////////////////////////////////////
    void DisplayBoxHeader(const char *pStr, int line, BoxLocale side,
                          uint16_t fgColor, uint16_t bgColor, int margin = 0);


    /////////////////////////////////////////////////////////////////////////////
    // DisplayBoxMain()
    //
    // Displays large (main) text in a specified box.
    //
    // Arguments:
    //    - pStr    - The text string to be displayed as a main text in the
    //                specified box.
    //    - line    - The line of the box containing the main text to be
    //                displayed.  Valid values are 0, 1, and 2.
    //    - side    - Specifies the horizontal location of the box as well
    //                as its the width.  Valid values are:
    //                  o eAll   - the box starts at the left edge of the
    //                             display and extends to the  right edge of
    //                             the display screen.
    //                  o eLeft  - the box starts at the left edge of the
    //                             display and extends half the width of the
    //                             display screen.
    //                  o eRight - the box starts at the horizontal middle of
    //                             display and extend it to the right edge of
    //                             the display screen.
    //    - fgColor - The color of the text that will be displayed.
    //    - bgColor - The color of the box that will hold the text string.
    //    - margin  - The number of pixels to indent the text from the left edge
    //                of the box.  This is generally set to the save value as
    //                the 'radius' value that was used to create the box via
    //                DisplayBox().
    /////////////////////////////////////////////////////////////////////////////
    void DisplayBoxMain(const char *pStr, int line, BoxLocale side,
                        uint16_t fgColor, uint16_t bgColor, int margin = 0);


    /////////////////////////////////////////////////////////////////////////////
    // DisplayHVCenteredText()
    //
    // Displays a text string centered vertically and horizontally on the TFT
    // screen.
    //
    // Arguments:
    //    - pStr    - A pointer to the NULL terminated string to be displayed.
    //    - fgColor - Specifies the foreground color of the text being displayed.
    //    - bgColor - Specifies the background color of the text being displayed.
    //    - radius  - The radius, in pixels, of the rounded corners of the box.
    //                A value of zero will cause the box to have square corners.
    /////////////////////////////////////////////////////////////////////////////
    void DisplayHVCenteredText(
        const char *pStr, uint16_t fgColor, uint16_t bgColor, int radius = 0);


    /////////////////////////////////////////////////////////////////////////////
    // DisplayHCenteredText()
    //
    // Displays a text string centered vertically on the TFT screen at the current
    // display line.
    //
    // Arguments:
    //    - pStr    - A pointer to the NULL terminated string to be displayed.
    //    - fgColor - Specifies the foreground color of the text being displayed.
    //    - bgColor - Specifies the background color of the text being displayed.
    //    - margin  - Specifies the left and right margin for the box in which the
    //                string will be displayed.
    /////////////////////////////////////////////////////////////////////////////
    void DisplayHCenteredText(
        const char *pStr, uint16_t fgColor, uint16_t bgColor, int margin = 0);


    /////////////////////////////////////////////////////////////////////////////
    // DisplayCenteredHalf()
    //
    // Displays a text string centered vertically on the TFT screen at the current
    // display line.
    //
    // Arguments:
    //    - pStr     - A pointer to the NULL terminated string to be displayed.
    //    - side    - Side in which the string will be centered.  Legal values
    //                are LEFT_HALF or RIGHT_HALF.
    //    - fgColor - Specifies the foreground color of the text being displayed.
    //    - bgColor - Specifies the background color of the text being displayed.
    //    - margin  - Specifies the left and right margin for the box in which the
    //                string will be displayed.
    /////////////////////////////////////////////////////////////////////////////
    void DisplayCenteredHalf(
        const char *pStr, BoxLocale side,
        uint16_t fgColor, uint16_t bgColor, int margin = 0);


    /////////////////////////////////////////////////////////////////////////////
    // BlinkCenteredDisplay()
    //
    // Blinks a text string centered vertically and horizontally on the TFT
    // screen.  Note that the string will blink at a rate of 500ms on and 500ms
    // off.
    //
    // Arguments:
    //    - pStr      - A pointer to the NULL terminated string to be displayed.
    //    - textColor - Specifies the foreground color of the text being displayed.
    //    - backColor - Specifies the background color of the text being displayed.
    //    - radius    - Radius in pixels for rounded containing box.
    //    - duration  - Duration in milliseconds to display the blinking string.
    /////////////////////////////////////////////////////////////////////////////
    void BlinkCenteredDisplay(
       const char *pStr,  uint16_t textColor, uint16_t backColor, int radius,
       uint32_t duration);


    /////////////////////////////////////////////////////////////////////////////
    // DisplayWorkingScreen()
    //
    // Displays the text "WORKING" centered horizontally and vertically on these
    // screen.
    /////////////////////////////////////////////////////////////////////////////
    void DisplayWorkingScreen();


    /////////////////////////////////////////////////////////////////////////////
    // DisplayResult()
    //
    // Displays a standardized results screen based on the status that is passed
    // to it.
    //
    // Arguments:
    //    - success  - Specifies the status to be displayed.  A value of 'true'
    //                 causes the 'goodStr' string to be displayed as a solid
    //                 (non-blinking) string centered vertically and horizontally
    //                 on the display.  A value of 'false' causes the 'badStr'
    //                 string to be displayed as a blinking string centered
    //                 vertically and horizontally on the screen.
    //    - pGoodStr - Specifies the string to be displayed when 'success' is
    //                 'true'.
    //    - pBadStr  - Specifies the string to be displayed when 'success' is
    //                 'false'.
    //    - radius   - The radius, in pixels, of the rounded corners of the box.
    //                 A value of zero will cause the box to have square corners.
    //    - duration - Duration in milliseconds to display the selected string.
    /////////////////////////////////////////////////////////////////////////////
    void DisplayResult(
        bool success, const char *pGoodStr, const char *pBadStr,
        int radius, uint32_t duration);


    /////////////////////////////////////////////////////////////////////////////
    // WelcomeScreen()
    //
    // Displays a welcome screen.
    //
    // Arguments:
    //    fgColor - Foreground color for the welcome screen.
    //    bgColor - Background color for the welcome screen.
    //    radius  - Radius for rounded box background.
    /////////////////////////////////////////////////////////////////////////////
    void WelcomeScreen(uint16_t fgColor, uint16_t bgColor, int radius);


    /////////////////////////////////////////////////////////////////////////////
    // Simple setters and getters.
    /////////////////////////////////////////////////////////////////////////////
    uint32_t GetBacklightPercent() const { return m_BacklightPercent; }
    void     GetTextColor(uint16_t &txt) const { txt = textcolor; }
    void     GetTextSize(uint8_t &sx, uint8_t &sy) const { sx = textsize_x; sy = textsize_y; }


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
    // Constants used for centered display.
    /////////////////////////////////////////////////////////////////////////////
    static const int LEFT_HALF  = 2;
    static const int RIGHT_HALF = 1;

protected:


private:
    /////////////////////////////////////////////////////////////////////////////
    // Unimplemented methods.  We don't want users to try to use these.
    /////////////////////////////////////////////////////////////////////////////
    Display();
    Display(Display &rCs);
    Display &operator=(Display &rCs);


    /////////////////////////////////////////////////////////////////////////////
    // Private methods.  User doesn't need to use these.
    /////////////////////////////////////////////////////////////////////////////
    void SaveEntryState();
    void RestoreEntryState();


    /////////////////////////////////////////////////////////////////////////////
    // Private static constants.
    /////////////////////////////////////////////////////////////////////////////
    static const char    *pPrefSavedStateLabel;
    static const size_t   MAX_NVS_NAME_LEN;
    static const uint8_t  BACKLIGHT_RESOLUTION     = 8U;
    static const uint8_t  BACKLIGHT_CHANNEL        = 2U;
    static const uint16_t BACKLIGHT_MAX_BRIGHTNESS = (1U << BACKLIGHT_RESOLUTION) - 1U;
    static const uint16_t BACKLIGHT_MIN_BRIGHTNESS = 0U;
    static const double   BACKLIGHT_FREQUENCY;


    /////////////////////////////////////////////////////////////////////////////
    // Private instance data.
    /////////////////////////////////////////////////////////////////////////////
    const char *m_pName;                // NVS instance name.

    int         m_BacklightPin;         // Pin associated with the TFT backlight.
    uint32_t    m_BacklightPercent;     // Current percent brightness of backlight.

    }; // End class Display.



#endif // DISPLAY_H