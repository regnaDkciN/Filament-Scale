/////////////////////////////////////////////////////////////////////////////////
// Display.cpp
//
// Contains miscellaneous methods used by the scale display.
//
// History:
// - jmcorbett 26-DEC-2020 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#include <Preferences.h>        // For NVS save/restore.
#include "Display.h"            // Our own class definition.
#include "JmcFilamentScale.h"   // For BOX_RADIUS.
#include "ScaleIcon.h"          // For ScaleIcon .


// Some constants used by the class.
const size_t Display::MAX_NVS_NAME_LEN     = 15U;
const char  *Display::pPrefSavedStateLabel = "Saved State";
const double Display::BACKLIGHT_FREQUENCY  = 5000.0d;


/////////////////////////////////////////////////////////////////////////////////
// Constructor
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
/////////////////////////////////////////////////////////////////////////////////
Display::Display(int csPin, int dcPin, int rstPin, int backlightPin,
                 uint8_t displayType, uint8_t rotation) :
        Adafruit_ST7735(csPin, dcPin, rstPin), m_pName(NULL),
        m_BacklightPin(backlightPin), m_BacklightPercent(0)
{
    // Configure the lite functionality.
    ledcSetup(BACKLIGHT_CHANNEL, BACKLIGHT_FREQUENCY, BACKLIGHT_RESOLUTION);

    // Attach the analog channel to the GPIO pin to be controlled.
    ledcAttachPin(m_BacklightPin, BACKLIGHT_CHANNEL);

    // Set the gBacklight brightness.
    ledcWrite(BACKLIGHT_CHANNEL, BACKLIGHT_MIN_BRIGHTNESS);

    // The 1.8" TFT display from Adafruit seems to have a startup problem in
    // which many times on initial power-up it doesn't get initialized
    // properly and just shows a blank white screen.  The following code
    // resets the display before initializing it in the hope of working around
    // this problem.  Since this is a constructor, the delay() function will not
    // work. We use delayMicroseconds() in its place, which should work in
    // constructors.  The following section of code does the same thing as
    // the reset that is done in Adafruit_SPITFT::initSPI(), but uses
    // delayMicroseconds() instead of delay().
    // Note that argument to delayMicroseconds() must be less than 16K, so the
    // delays used here all use 1000 uS or 1ms as a basis, and loop the
    // appropriate amount of times to affect the needed long delays.
    pinMode(rstPin, OUTPUT);
    digitalWrite(rstPin, HIGH);
    // delay(100);  // Delay 100 ms.
    int i = 0;
    for (i = 0; i < 100; i++)
    {
        delayMicroseconds(1000);
    }
    digitalWrite(rstPin, LOW);
    // delay(100);  // Delay 100 ms.
    for (i = 0; i < 100; i++)
    {
        delayMicroseconds(1000);
    }
    digitalWrite(rstPin, HIGH);
    // delay(200);  // Delay 200 ms.
    for (i = 0; i < 200; i++)
    {
        delayMicroseconds(1000);
    }

    // Setup the display hardware and firmware library.
    initR(displayType);
    setRotation(rotation);
    fillScreen(ST7735_BLACK);
} // End constructor.


/////////////////////////////////////////////////////////////////////////////////
// Init()
//
// This method initializes the filament class.
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
/////////////////////////////////////////////////////////////////////////////////
bool Display::Init(const char *pName)
{
    // Assume we're gonna fail.
    bool status = false;

    // Make sure the name is valid.  If not then we fail.
    if ((pName != NULL) && (*pName != '\0') && (strlen(pName) <= MAX_NVS_NAME_LEN))
    {
        // Name was OK.  Save it and remember that we succeeded.
        m_pName            = pName;
        status             = true;
    }
    return status;
} // End Init().


/////////////////////////////////////////////////////////////////////////////////
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
//    Returns 'true' if successful.  Returns 'false' if an invalid 'level' value
//    was passed.
/////////////////////////////////////////////////////////////////////////////////
bool Display::SetBacklightPercent(uint32_t percent)
{
    bool status = false;

    if (percent <= 100)
    {
        m_BacklightPercent = percent;
        uint32_t backlightValue =
          (percent * (BACKLIGHT_MAX_BRIGHTNESS - BACKLIGHT_MIN_BRIGHTNESS) / 100) +
           BACKLIGHT_MIN_BRIGHTNESS;
        ledcWrite(BACKLIGHT_CHANNEL, backlightValue);
        status = true;
    }
    return status;
} // End SetBacklightPercent().


/////////////////////////////////////////////////////////////////////////////////
// DisplayBox()
//
// Draws a box of the specified size on the screen at the specified location.
//
// Arguments:
//    - line    - The line at which the box will be displayed.  Valid values
//                are 0, 1, and 2.
//    - side    - Specifies the horizontal location to start the box as well as
//                the width of the box.  Valid values are:
//                  o eAll   - to start the box at the left edge of the
//                             display and extend it the entire width of the
//                             display screen.
//                  o eLeft  - to start the box at the left edge of the
//                             display and extend it half the width of the
//                             display screen.
//                  o eRight - to start the box at the horizontal middle of
//                             display and extend it to the right edge of
//                             the display screen.
//    - fgColor - The color of the outline that will be drawn around the box.
//                Setting this to the same value as bgColor will cause no
//                outline to be seen on the display (although one will be
//                drawn).
//    - bgColor - The color of the box that will be drawn.
//    - radius  - The radius, in pixels, of the rounded corners of the box.
//                A value of zero will cause the box to have square corners.
/////////////////////////////////////////////////////////////////////////////////
void Display::DisplayBox(int line, BoxLocale side, uint16_t fgColor,
                         uint16_t bgColor, int radius)
{
    int w = width();
    int h = height();
    int x0 = (side == eRight) ? w / 2 : 0;
    int y0 = line * h / 3;
    int xw = (side == eAll) ? w : w / 2;
    int yh = h / 3 + 1;
    fillRect(x0, y0 + 1, xw, yh, ST7735_BLACK);
    fillRoundRect(x0, y0, xw, yh, radius, bgColor);
    drawRoundRect(x0, y0, xw, yh, radius, fgColor);
} // End DisplayBox().


/////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////
void Display::DisplayBoxHeader(const char *pStr, int line, BoxLocale side,
                               uint16_t fgColor, uint16_t bgColor, int margin)
{
    // Save entry state.
    DisplayState state(textsize_x, textsize_y, textcolor, textbgcolor);

    // Get the size of the string to be displayed.
    int16_t  ulx = 0;   // Upper left X coord of string (pixels).
    int16_t  uly = 0;   // Upper left Y coord of string (pixels).
    uint16_t xl  = 0;   // X length of string in pixels.
    uint16_t yl  = 0;   // Y height of string in pixels.
    setTextSize(1, 1);
    getTextBounds(pStr, 0, 0, &ulx, &uly, &xl, &yl);

    // Determine the current screen size (can change in use).
    int16_t screenWidth  = width();
    int16_t screehHeight = height();

    // Determine the coords to start printing the string.
    int16_t  cursorX = margin;
    int16_t  cursorY = line * screehHeight / 3 + 3;
    if (side == eRight)
    {
        cursorX += screenWidth / 2;
    }

    // Clear the area of the box from the end of the new text to the end
    // of the box.
    uint16_t fieldWidth = (side == eAll) ? screenWidth : screenWidth / 2;
    int16_t  clearWidth = fieldWidth - 2 * margin - xl;
    fillRect(cursorX + xl, cursorY, clearWidth, yl, bgColor);

    // Get ready to print the string.
    setCursor(cursorX, cursorY);
    setTextColor(fgColor, bgColor);

    // Print the string.
    print(pStr);

    // Restore the entry state.
    state.RestoreState(textsize_x, textsize_y, textcolor, textbgcolor);
} // End DisplayBoxHeader().


/////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////
void Display::DisplayBoxMain(const char *pStr, int line, BoxLocale side,
                             uint16_t fgColor, uint16_t bgColor, int margin)
{
    // Save entry state.
    DisplayState state(textsize_x, textsize_y, textcolor, textbgcolor);

    // Determine if the length of the string is too long to fit within the
    // normal screen width.  If so, change the text width to double the number
    // of characters that will fit in the box.
    int length = strlen(pStr);
    int limit  = side == eAll ? (SCREEN_CHAR_WIDTH - 1) : (SCREEN_CHAR_WIDTH /2);

    // Get ready to print the string.
    setTextSize(length > limit ? 1 : 2, 3);
    setCursor(margin, line * height() / 3 + 15);

    // Display the data based on the type of box used.
    if (side == eAll)
    {
        DisplayHCenteredText(pStr, fgColor, bgColor, margin);
    }
    else
    {
        DisplayCenteredHalf(pStr, side, fgColor, bgColor, margin);
    }

    // Restore the entry state.
    state.RestoreState(textsize_x, textsize_y, textcolor, textbgcolor);
} // End DisplayBoxMain().


/////////////////////////////////////////////////////////////////////////////////
// DisplayHVCenteredText()
//
// Displays a text string centered vertically and horizontally on the TFT
// screen.
//
// Arguments:
//    - pStr    - A pointer to the NULL terminated string to be displayed.
//    - fgColor - Specifies the foreground color of the text being displayed.
//    - bgColor - Specifies the background color of the text being displayed.
//    - radius  - Radius in pixels for rounded containing box.
/////////////////////////////////////////////////////////////////////////////////
void Display::DisplayHVCenteredText(
        const char *pStr, uint16_t fgColor, uint16_t bgColor, int radius)
{
    // Save entry state.
    DisplayState state(textsize_x, textsize_y, textcolor, textbgcolor);

    // Get the size of the string to be displayed.
    int16_t  ulx = 0;   // Upper left X coord of string (pixels).
    int16_t  uly = 0;   // Upper left Y coord of string (pixels).
    uint16_t xl  = 0;   // X length of string in pixels.
    uint16_t yl  = 0;   // Y height of string in pixels.
    setTextSize(2, 2);
    getTextBounds(pStr, 0, 0, &ulx, &uly, &xl, &yl);

    // Determine the current screen size (can change in use).
    int16_t screenWidth  = width();
    int16_t screenHeight = height();

    // Determine the coords to start printing the string.
    uint16_t cursorX = (screenWidth - xl) / 2;
    uint16_t cursorY = (screenHeight - yl) / 2;

    // Display the text, centered on the display.
    FillScreen(fgColor, bgColor, radius);

    setTextColor(fgColor, bgColor);
    setCursor(cursorX, cursorY);
    print(pStr);
    
    // Repair the left side of the enclosing box in case the string contains
    // any new line characters.
    drawFastVLine(0, cursorY, yl - uly, fgColor);

    // Restore the entry state.
    state.RestoreState(textsize_x, textsize_y, textcolor, textbgcolor);
} // End DisplayHVCenteredText().


/////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////
void Display::DisplayHCenteredText(
        const char *pStr, uint16_t fgColor, uint16_t bgColor, int margin)
{
    // Save entry state.
    DisplayState state(textsize_x, textsize_y, textcolor, textbgcolor);

    int16_t  ulx = 0;   // Upper left X coord of string (pixels).
    int16_t  uly = 0;   // Upper left Y coord of string (pixels).
    uint16_t xl  = 0;   // X length of string in pixels.
    uint16_t yl  = 0;   // Y height of string in pixels.

    // Get the size of the string to be displayed.
    getTextBounds(pStr, 0, 0, &ulx, &uly, &xl, &yl);

    // Determine the current screen size (can change in use).
    int16_t screenWidth = width();

    // Determine the coords to start printing the string.
    uint16_t cursorX = (screenWidth - xl) / 2;
    uint16_t cursorY = getCursorY();

    // Display the text, centered on the display.  Clear the line on both sides
    // of the new string.
    setTextColor(fgColor, bgColor);
    setCursor(cursorX, cursorY);
    fillRect(margin, cursorY, cursorX - margin, yl, bgColor);
    fillRect(cursorX + xl, cursorY,
             screenWidth - margin - (cursorX + xl), yl, bgColor);

    print(pStr);

    // Restore the entry state.
    state.RestoreState(textsize_x, textsize_y, textcolor, textbgcolor);
} // End DisplayHCenteredText().


/////////////////////////////////////////////////////////////////////////////////
// DisplayCenteredHalf()
//
// Displays a text string centered vertically on the TFT screen at the current
// display line.
//
// Arguments:
//    - pStr    - A pointer to the NULL terminated string to be displayed.
//    - side    - Side in which the string will be centered.  Legal values
//                are LEFT_HALF or RIGHT_HALF.
//    - fgColor - Specifies the foreground color of the text being displayed.
//    - bgColor - Specifies the background color of the text being displayed.
//    - margin  - Specifies the left and right margin for the box in which the
//                string will be displayed.
/////////////////////////////////////////////////////////////////////////////////
void Display::DisplayCenteredHalf(
        const char *pStr, BoxLocale side, uint16_t fgColor, uint16_t bgColor, int margin)
{
    // Save entry state.
    DisplayState state(textsize_x, textsize_y, textcolor, textbgcolor);

    int16_t  ulx = 0;   // Upper left X coord of string (pixels).
    int16_t  uly = 0;   // Upper left Y coord of string (pixels).
    uint16_t xl  = 0;   // X length of string in pixels.
    uint16_t yl  = 0;   // Y height of string in pixels.

    // Get the size of the string to be displayed.
    getTextBounds(pStr, 0, 0, &ulx, &uly, &xl, &yl);

    // Determine the current screen size (can change in use).
    int16_t screenWidth  = width();

    // Determine the coords to start printing the string.
    uint16_t cursorX = (screenWidth / 2 - xl) / 2;
    uint16_t cursorY = getCursorY();
    uint16_t fieldX  = 0;
    if (side == eRight)
    {
        cursorX += screenWidth / 2;
        fieldX  = screenWidth / 2;
    }

    // Display the text, centered on the display.  Clear the line on both sides
    // of the new string.
    setTextColor(fgColor, bgColor);
    setCursor(cursorX, cursorY);
    fillRect(fieldX + margin, cursorY, cursorX - fieldX - margin, yl, bgColor);
    fillRect(cursorX + xl, cursorY,
             screenWidth / 2 - (cursorX - fieldX) - xl - margin, yl, bgColor);

    print(pStr);

    // Restore the entry state.
    state.RestoreState(textsize_x, textsize_y, textcolor, textbgcolor);
} // End DisplayCenteredHalf().


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
void Display::FillScreen(uint16_t fgColor, uint16_t bgColor, int radius)
{
    // Remove any colors that may be past the rounded box corners.
    if (radius)
    {
        fillScreen(ST7735_BLACK);
    }
    fillRoundRect(0, 0, width(), height(), radius, bgColor);
    drawRoundRect(0, 0, width(), height(), radius, fgColor);
}

/////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////
void Display::BlinkCenteredDisplay(
        const char *pStr, uint16_t textColor, uint16_t backColor, int radius,
        uint32_t duration)
{
    const uint32_t BLINK_RATE = 500;

    for (uint32_t i = 0; i < duration; i += 2 * BLINK_RATE)
    {
        DisplayHVCenteredText(pStr, textColor, backColor, radius);
        delay(BLINK_RATE);

        FillScreen(textColor, backColor, radius);
        delay(BLINK_RATE);
    }
} // End BlinkCenteredDisplay().


/////////////////////////////////////////////////////////////////////////////////
// DisplayWorkingScreen()
//
// Displays the text "WORKING" centered horizontally and vertically on these
// screen.
/////////////////////////////////////////////////////////////////////////////////
void Display::DisplayWorkingScreen()
{
    // Show the user that we're doing something.
    DisplayHVCenteredText("WORKING", ST77XX_BLUE, ST77XX_WHITE, BOX_RADIUS);
} // End DisplayWorkingScreen().


/////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////
void Display::DisplayResult(
        bool success, const char *pGoodStr, const char *pBadStr,
        int radius, uint32_t duration)
{
    uint16_t backColor = ST77XX_GREEN;
    uint16_t textColor = ST77XX_WHITE;

    // Let the user know if we succeeded or failed.
    if (success)
    {
        DisplayHVCenteredText(pGoodStr, textColor, backColor, radius);
        delay(duration);
    }
    else
    {
        backColor = ST77XX_RED;
        BlinkCenteredDisplay(pBadStr, textColor, backColor, radius, duration);
    }
} // End DisplayResult().


/////////////////////////////////////////////////////////////////////////////////
// WelcomeScreen()
//
// Displays a welcome screen.
//
// Arguments:
//    fgColor - Foreground color for the welcome screen.
//    bgColor - Background color for the welcome screen.
//    radius  - Radius for rounded box background.
/////////////////////////////////////////////////////////////////////////////////
void Display::WelcomeScreen(uint16_t fgColor, uint16_t bgColor, int radius)
{
    // Make sure the backlight is on.  The NVS restore is performed later, and
    // will set the backlight as the user wants, but we always want the welcome
    // screen to display with 100% brightness.
    SetBacklightPercent(100);

    const char *pName = "JMC 3DP SCALE";
    const uint32_t SCROLL_DWELL_MS = 10;

    // Get ready to display the welcome screen.
    setTextSize(2, 3);
    setTextWrap(false);
    FillScreen(fgColor, bgColor, radius);

    // Display the scale graphic background.
    drawBitmap((width() - 120) / 2, (height() - 120) / 2,
                     ScaleIcon, 120, 120, 0);
    delay(1000);

    // Get the size of the string to be displayed.
    int16_t  ulx = 0;   // Upper left X coord of string (pixels).
    int16_t  uly = 0;   // Upper left Y coord of string (pixels).
    uint16_t xl  = 0;   // X length of string in pixels.
    uint16_t yl  = 0;   // Y height of string in pixels.
    getTextBounds(pName, 0, 0, &ulx, &uly, &xl, &yl);

    // Calculate where to place the string.
    uint16_t cursorY = (height() - yl) / 2;
    uint16_t xStop   = (width() - xl) / 2;

    // Slowly scroll the string from the right side.
    setTextColor(fgColor, bgColor);
    for (int i = width() - 1; i >= xStop; i--)
    {
        setCursor(i, cursorY);
        print(pName);
        drawFastVLine(width() - 1, radius, height() - 2 * radius, fgColor);
        delay(SCROLL_DWELL_MS);
    }

    // Wait a second before scrolling the footer.
    delay(1000);
    setTextSize(1);
    const char *pFooter = "(c) 2021, J. M. Corbett";
    getTextBounds(pFooter, 0, 0, &ulx, &uly, &xl, &yl);

    // Calculate where to place the string.
    cursorY = height() - 2 * yl;
    xStop   = (width() - xl) / 2;

    // Slowly scroll the string from the right side.
    for (int i = width() - 1; i >= xStop; i--)
    {
        setCursor(i, cursorY);
        print(pFooter);
        drawFastVLine(width() - 1, radius, height() - 2 * radius, fgColor);
        delay(SCROLL_DWELL_MS);
    }
} // End WelcomeScreen().


/////////////////////////////////////////////////////////////////////////////////
// Save()
//
// Saves our current state to NVS.
//
// Returns:
//    Returns 'true' if successful, or 'false' otherwise.
/////////////////////////////////////////////////////////////////////////////////
bool Display::Save() const
{
    size_t saved = 0;
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
bool Display::Restore()
{
    // Assume we're gonna fail.
    bool succeeded = false;

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
bool Display::Reset()
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


