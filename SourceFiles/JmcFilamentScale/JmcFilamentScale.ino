/////////////////////////////////////////////////////////////////////////////////
// JmcFilamentScale.ino
//
// Implements a filament scale with the following features:
//   - Menu driven settings.
//   - Selectable weight units display (g, kg, oz, lb).
//   - Weigh items up to 5kg (11 lb).
//   - Addjustable display brightness.
//   - Built-in temperature display.
//   - Built-in humidity display.
//   - Selectable temperature units (degrees C or F).
//   - Storage for data for up to 15 filament spools.
//   - Adjustable spool parameters include:
//      - Spool name (up to 8 characters).
//      - Filament type (15 selectable filament types).
//      - Empty spool weight.
//      - Filament diameter.
//   - Adjustable filament density for eoach of the 15 filament types.
//   - Calculation and display of remaining length of filament per spool.
//   - Selectable filament length units (mm, cm, m, in, ft, yd).
//   - Ability to save settings to NVS for restoration on powerup.
//   - Web interface with ability to monitor all data and modify all options
//     remotely.
//
//  This implementation relies heavily on the ArdiunoMenu library:
//      https://github.com/neu-rah/ArduinoMenu
//
// This code is menat to run on an ESP32 microprocessor with the following or
// similar peripherals:
//      - Adafruit HUZZAH32 - ESP32 Feather Board.
//        https://www.adafruit.com/product/3405
//      - Adafruit 1.8" 128x160  TFT display.
//        https://www.adafruit.com/product/358
//      - 5kg Load Cell and HX711.
//        https://www.amazon.com/gp/product/B075317R45/ref=ppx_yo_dt_b_asin_title_o02_s00?ie=UTF8&psc=1
//      - DHT22 Digital Temperature and Himidity Sensor.
//        https://www.amazon.com/gp/product/B07T63JRT8/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1
//      - Rotaty Encoder with Pushbutton Module.
//        https://www.amazon.com/gp/product/B081YCR3JC/ref=ppx_yo_dt_b_asin_title_o04_s00?ie=UTF8&psc=1
//
// This project was inspired by an article in "Nuts and Volts" magazine
// 2019 Issue-4.  The article is entitled "Build a 3D Printer Filament Scale" by
// Edward Andrews.  I used a few small code fragments from the article, and the
// filament density table from the article.  Additional inspiration was gathered
// from "Filament Health Monitor" at https://replicantfx.com/filament-health-monitor/
//
// History:
// - jmcorbett 04-JAN-2021 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////

#include "JmcFilamentScale.h"   // For global variables and extern definitioins.
#include <menu.h>               // For ArduinoMenu.
#include <menuIO/adafruitGfxOut.h>// For Adafruit graphics library.
#include <menuIO/chainStream.h> // For menu input/output streams.
#include <menuIO/serialOut.h>   // For menu serial output.
#include <menuIO/serialIn.h>    // For menu serial input.
#include <plugin/userMenu.h>    // For OBJ menu macro.
#include "ESP32EncoderStream.h" // For encoder w/pushbutton.
#include "MainScreen.h"         // For MainScreen related stuff.
#include "WebData.h"            // For web page handling code.
#include "ScaleMenu.h"          // For menu  related stuff.
#include "AuxPb.h"              // For AuxPb class.


/////////////////////////////////////////////////////////////////////////////////
// ST7735 1.8TFT 128x160 Pin assignments and related variables.
/////////////////////////////////////////////////////////////////////////////////
static const int TFT_CS   = 14;     // TFT display CS pin.
static const int TFT_DC   = 32;     // TFT display DC pin.
static const int TFT_RST  = 15;     // TFT display RST pin.
static const int TFT_LITE = A0;     // TFT display backlight pin.

// Construct our TFT  object.
// Adafruit_ST7735 gTft(TFT_CS, TFT_DC, TFT_RST);
Display gTft(TFT_CS, TFT_DC, TFT_RST, TFT_LITE);

// TFT related globals and constants.
uint32_t           gBacklightPercent = 100;
static const char *gTftNvsName       = "Dispaly";


/////////////////////////////////////////////////////////////////////////////////
// Rotary encoder pins.
/////////////////////////////////////////////////////////////////////////////////
static const int ENC_PIN_A      = A3;   // Encoder A pin.
static const int ENC_PIN_B      = A4;   // Encoder B pin.
static const int ENC_BUTTON_PIN = 13;   // Encoder pushbutton pin.
static const int ENC_SENSITIVITY = 4;   // Encoder sensitivity.

// Construct the pushbutton encoder stream object.  Note that some encoders will
// require the sensitivity to be set to a different value.
ESP32EncoderStream gEncStream(ENC_PIN_A, ENC_PIN_B, ENC_BUTTON_PIN, ENC_SENSITIVITY);


/////////////////////////////////////////////////////////////////////////////////
// Auxiliary pushbutton pins.
/////////////////////////////////////////////////////////////////////////////////
static const int AUX_BUTTON_PIN = A1;   // Auxiliary pushbutton pin.

// Construct the auxiliary pushbutton object.
AuxPb gAuxPb(AUX_BUTTON_PIN);


/////////////////////////////////////////////////////////////////////////////////
// Define the LoadCell sensor related I/O pins and global data and constants.
/////////////////////////////////////////////////////////////////////////////////
static const int LOADCELL_DOUT_PIN  = A2;   // LoadCell DOUT signal pin.
static const int LOADCELL_SCK_PIN   = A5;   // LoadCell CLOCK pin.

// Construct and init the LoadCell object.
LoadCell gLoadCell(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN, 128U);

// Load cell related globals and constants.
       WeightUnits gScaleUnits        = eWuGrams;
       uint32_t    gScaleAveragingSamples =
                   (AVG_SAMPLES_DEFAULT_MS + WEIGHT_UPDATE_PERIOD_MS / 2) /
                   WEIGHT_UPDATE_PERIOD_MS;
       uint8_t     gScaleGain         = 128;
static const char *gLoadCellNvsName   = "Load Cell";
       float       gCurrentWeight     = 0.0f;
       float       gCurrentLength     = 0.0f;


/////////////////////////////////////////////////////////////////////////////////
// Define the environmental sensor (temperature and humidity) pins and global
// data and constants.
/////////////////////////////////////////////////////////////////////////////////
static const uint8_t ENV_DAT_PIN    = 27;   // Sensor data pin.
static const uint8_t ENV_TYPE       = DHT22;// Sensor type.

// Construct and init the EnvSensor object.
EnvSensor gEnvSensor(ENV_DAT_PIN, ENV_TYPE);

// Environmental sensor related globals and constants.
TempScale gTemperatureUnits     = eTempScaleF;
static const char *gEnvSensorNvsName   = "Env Sensor";
       float       gCurrentTemperature = 0.0f;
       float       gCurrentHumidity    = 0.0f;


/////////////////////////////////////////////////////////////////////////////////
// Filament class related data and constants.
/////////////////////////////////////////////////////////////////////////////////
Filament gFilament;

// Filament related global data and constants.
       float  gWorkingFilamentDensity = 0.01;
       char   gWorkingFilamentType[Filament::TYPE_STRING_MAX_SIZE] = { 0 };
static const char *gFilamentNvsName  = "Filament";


/////////////////////////////////////////////////////////////////////////////////
// Spool Manager related data and constants.
/////////////////////////////////////////////////////////////////////////////////
SpoolManager<NUMBER_SPOOLS> gSpoolMgr;

// Set up spool related global data.  Used for dynamic menu implementation.
       SpoolData gWorkingSpoolData; // Spool data currently being worked om.
static const char *gSpoolMgrNvsName  = "Spool Mgr";


/////////////////////////////////////////////////////////////////////////////////
// LengthManager related data and constants.
/////////////////////////////////////////////////////////////////////////////////
LengthManager gLengthMgr;

// Length manager related globals and constants.
static const       LengthUnits DEFAULT_LENGTH_UNITS = luMm;
                   LengthUnits  gLengthUnits        = DEFAULT_LENGTH_UNITS;
static float       gLengthFactor                    = 0.0;
static const char *gLengthMgrNvsName                = "Length Mgr";


/////////////////////////////////////////////////////////////////////////////////
// Network related data and constants.
/////////////////////////////////////////////////////////////////////////////////
static const int   SERVER_PORT = 80;
       Network     gNetwork(SERVER_PORT);

// NVS name.
static const char *gNetworkNvsName    = "Network";

// Access the scale via this name (i.e. http://JmcScale.local).
const char *gNetworkServerName = "JmcScale";
const char * &rNetworkServerName = gNetworkServerName;


/////////////////////////////////////////////////////////////////////////////////
// Main Screen related data and constants.
/////////////////////////////////////////////////////////////////////////////////
static const char *gMainScreenNvsName = "Main Screen";


/////////////////////////////////////////////////////////////////////////////////
// Weight related data.
/////////////////////////////////////////////////////////////////////////////////
static const  double MAX_CAL_WEIGHT = 5000.0;
static const  double MIN_CAL_WEIGHT = 0.0;
       double gCalibrateWeight      = MIN_CAL_WEIGHT;
       double gMinWeight            = MIN_CAL_WEIGHT;
       double gMaxWeight            = MAX_CAL_WEIGHT;
       double gBigWeightStep        = 10.0;
       double gSmallWeightStep      = .1;


/////////////////////////////////////////////////////////////////////////////////
// Miscellaneous globals.
/////////////////////////////////////////////////////////////////////////////////
       bool   gRunningMenu          = false;
       bool   gDataUpdated          = false;

/////////////////////////////////////////////////////////////////////////////////
// SetDecimalPlaces()
//
// Rounds a double value to a specified precision (number of digits to the right
// of the decimal point).
//
// Arguments:
//  value     - this is the value that will be rounded.
//  precision - this is the number of digits to the right of the decimal point
//              to keep.
//
// Returns:
//   Returns the rounded value.
/////////////////////////////////////////////////////////////////////////////////
double SetDecimalPlaces(double value, int precision)
{
    double factor = pow(10, precision);
    value *= factor;
    value = ceil(value);
    value /= factor;
    return value;
}


/////////////////////////////////////////////////////////////////////////////////
// GetMinScaleWeight()
//
// Returns the minimum allowable scale weight given the current weight units.
// Currently always returns 0.0.  May change in the future.
/////////////////////////////////////////////////////////////////////////////////
static double GetMinScaleWeight()
{
    double minWeight = 0.0;
    switch(gLoadCell.GetUnits())
    {
    case eWuGrams:     minWeight = 0.0; break;
    case eWuKiloGrams: minWeight = 0.0; break;
    case eWuOunces:    minWeight = 0.0; break;
    case eWuPounds:    minWeight = 0.0; break;
    default:           minWeight = 0.0; break;
    }
    return minWeight;
} // End GetMinScaleWeight().


/////////////////////////////////////////////////////////////////////////////////
// GetMaxScaleWeight()
//
// Returns the maximum allowable scale weight given the current weight units.
/////////////////////////////////////////////////////////////////////////////////
double GetMaxScaleWeight()
{
    double maxWeight = 10.0;
    switch(gLoadCell.GetUnits())
    {
    case eWuGrams:     maxWeight = 5000.0; break;
    case eWuKiloGrams: maxWeight = 5.0;    break;
    case eWuOunces:    maxWeight = 176.37; break;
    case eWuPounds:    maxWeight = 11.02;  break;
    default:           maxWeight = 5000.0; break;
    }
    return maxWeight;
} // End GetMaxScaleWeight().


/////////////////////////////////////////////////////////////////////////////////
// GetWeightBigStep()
//
// Returns the size of a large step for when editing weight values based on
// the currently selected weight units.
/////////////////////////////////////////////////////////////////////////////////
static double GetWeightBigStep()
{
    double step = 10.0;
    switch(gLoadCell.GetUnits())
    {
    case eWuGrams:     step = 10.0; break;
    case eWuKiloGrams: step = 0.01; break;
    case eWuOunces:    step = 1.0;  break;
    case eWuPounds:    step = 0.1;  break;
    default:           step = 10.0; break;
    }
    return step;
} // End GetWeightBigStep().


/////////////////////////////////////////////////////////////////////////////////
// GetWeightSmallStep()
//
// Returns the size of a small step for when editing weight values based on
// the currently selected weight units.
/////////////////////////////////////////////////////////////////////////////////
static double GetWeightSmallStep()
{
    double step = 0.1;
    switch(gLoadCell.GetUnits())
    {
    case eWuGrams:     step = 0.1;    break;
    case eWuKiloGrams: step = 0.0001; break;
    case eWuOunces:    step = 0.01;   break;
    case eWuPounds:    step = 0.001;  break;
    default:           step = 0.1;    break;
    }
    return step;
} // End GetWeightSmallStep().


/////////////////////////////////////////////////////////////////////////////////
// GetWeightDecimalPlaces()
//
// Returns the number of digits to display to the right of the decimal point
// based on the currently selected weight units.
/////////////////////////////////////////////////////////////////////////////////
int GetWeightDecimalPlaces()
{
        int decimalPlaces = 1;
        switch(gLoadCell.GetUnits())
        {
        case eWuGrams:     decimalPlaces = 1; break;
        case eWuKiloGrams: decimalPlaces = 4; break;
        case eWuOunces:    decimalPlaces = 2; break;
        case eWuPounds:    decimalPlaces = 3; break;
        default:           decimalPlaces = 1; break;
        }
        return decimalPlaces;
} // End GetWeightDecimalPlaces().


/////////////////////////////////////////////////////////////////////////////////
// SaveSpoolOffset()
//
// If a spool is selected, then save its offset value as the current load cell
// offset.  Otherwise, save a value of 0.0.
/////////////////////////////////////////////////////////////////////////////////
void SaveSpoolOffset()
{
    double currentSpoolOffset = 0.0;

    Spool *pSelectedSpool = gSpoolMgr.GetSelectedSpool();
    if (pSelectedSpool != NULL)
    {
        currentSpoolOffset = pSelectedSpool->GetSpoolWeight();
    }
    gLoadCell.SetOffset(currentSpoolOffset);
} // End SaveSpoolOffset().


/////////////////////////////////////////////////////////////////////////////////
// UpdateLengthFactor()
//
// If a spool is selected, then update our length factor.  Otherwise, save a
// value of 0.0.
/////////////////////////////////////////////////////////////////////////////////
void UpdateLengthFactor()
{
    // Initially reset the length factor.
    gLengthFactor = 0.0;

    // Only update the length factor if a spool is selected.
    Spool *pSelectedSpool = gSpoolMgr.GetSelectedSpool();
    if ((gLoadCell.IsCalibrated()) && (pSelectedSpool != NULL))
    {
        gLengthFactor = gLengthMgr.CalculateLengthFactor(
                        pSelectedSpool->GetDiameter(),
                        gLoadCell.GetBaseUnitsFactor(gScaleUnits),
                        pSelectedSpool->GetDensity());
    }
} // End UpdateLengthFactor().


/////////////////////////////////////////////////////////////////////////////////
// SetLoadCellUnits()
//
// Updates our load cell (weight) units, then scales all spool table weights
// based on the new units.
/////////////////////////////////////////////////////////////////////////////////
void SetLoadCellUnits(WeightUnits units)
{
    // Update our weight units and edit limits.
    gLoadCell.SetUnits(units);
    gMinWeight = GetMinScaleWeight();
    gMaxWeight = GetMaxScaleWeight();
    gBigWeightStep   = GetWeightBigStep();
    gSmallWeightStep = GetWeightSmallStep();

    // Determine our multiplier based on the new weight units.
    double multiplier = gLoadCell.GetConversionFactor();

    // Scale the spool weights for the entire spool table based on the new units.
    for (uint16_t i = 0; i < NUMBER_SPOOLS; i++)
    {
        Spool *pSpool = gSpoolMgr.GetSpool(i);
        float weight = pSpool->GetSpoolWeight() * multiplier;
        pSpool->SetSpoolWeight(weight);
    }
} // End SetLoadCellUnits().


/////////////////////////////////////////////////////////////////////////////////
// AddCommas()
//
// Takes a double value as input, and converts it to a string containing commas
// separating the thousands places.  For example, if a vlaue of 123456.789 is
// passed in with a precision of 2, the resultant converted string will be
// "123,456.78".
//
// Arguments:
//  - val     - This is the double value to be converted.
//  - prec    - Specifies the precision of the converted string.  That is, it
//              specifies the number of digits to the right of the decimal
//              point in the returned string.
//  - pBuf    - Specifies the buffer in which the result will be returned.
//  - bufSize - Specifies the size of 'pBuf'.
//
//  Returns:
//      Always returns 'pBuf'.  It is possible that the converted string will not
//      fit within 'pBuf'.  In this case, the most significant digits of the
//      converted string will be truncated, so beware and be sure to supply
//      a buffer that is large enough to hold the converted string.
/////////////////////////////////////////////////////////////////////////////////
char *AddCommas(double val, int prec, char *pBuf, int bufSize)
{
    const char THOUSANDS_SEPARATOR = ',';
    const char DECIMAL_POINT       = '.';
    const size_t MAX_FORMATTED_NUMBER_SIZE = 30;
    char temp[MAX_FORMATTED_NUMBER_SIZE];

    // If the caller supplied a buffer, start with an empty string.
    if (pBuf)
    {
        *pBuf = '\0';
    }

    // Validate our arguments.  Make sure:
    // - the caller's buffer exists;
    // - the caller's buffer is at least big enough for the smallest possible string;
    // - our temporary buffer is at least big enough for the smallest possible string.
    // If not, just return.
    if ((pBuf == NULL) || (prec > (MAX_FORMATTED_NUMBER_SIZE - 3)) || (bufSize < 3))
    {
        return pBuf;
    }

    // Round value to the given precision.
    double roundVal = ((val > 0.0) ? 0.5 : -0.5);
    val += (roundVal * pow(10.0, -prec));

    // Convert the rounded value to a string.
    snprintf(temp, MAX_FORMATTED_NUMBER_SIZE, "%f", val);

    // Look for a decimal point.
    char *pDot = strchr(temp, DECIMAL_POINT);
    char *pSrc;
    char *pDst;
    if (pDot)   // A decimal point was found.
    {
        // Since the builtin version of printf() does not support variable
        // precision, we need to handle it ourselves.  Here we check to see
        // how many digits are past the decimal point, and clip to the specified
        // precision if needed.
        int dotLen = strlen(pDot);

        // Special case when prec is 0.  Remove decimal point.  I.e. display as int.
        if (prec == 0)
        {
            dotLen = 0;
            *pDot = '\0';
        }
        else if (dotLen > prec)
        {
            pDot[prec + 1] = '\0';
            dotLen = prec + 1;
        }

        pDst = pBuf + bufSize - dotLen - 1; // Set pDst to allow the fractional part to fit.
        strcpy(pDst, pDot);                 // Copy the fractional part.
        *pDot = '\0';                       // Cut the fractional part in temp.
        pSrc = --pDot;                      // Point to the last non fractional char in temp.
        pDst--;                             // Point to the previous char in pDst.
    }
    else    // No decimal point was found.
    {
        pSrc = temp + strlen(temp) - 1;     // pSrc is last char of our float string.
        pDst = pBuf + bufSize - 1;          // pDst is last char of the caller's buffer.
    }

    int len = strlen(temp); // Initialize the mantissa size.
    int digitCount = 0;     // Initialize the digit counter.

    do
    {
        // Count digits (and digits only).
        if ((*pSrc <= '9') && (*pSrc >= '0'))
        {
            // Add thousands separator if we added 3 digits already.
            if (digitCount && !(digitCount % 3))
            {
                *pDst-- = THOUSANDS_SEPARATOR;
            }
            // Increment the mantissa digit count.
            digitCount++;
        }
        // Copy this digit and point to the next.
        *pDst-- = *pSrc--;

      // Finish if we run out of mantissa characters, or if we fill the caller's buffer.
    } while (--len && ((pDst - pBuf) >= 0));

    // Move the converted value string to the start of the caller's buffer.
    // Note that this move depends on strcpy() starting its copy at the lowest
    // address and continuing toward the end of the buffer.  If this is not the
    // case, then it is possible that the string data could get corrupted.
    if ((pDst + 1) != pBuf)
    {
        // Move the string.
        strcpy(pBuf, pDst + 1);
    }

    // Return a pointer to the user's buffer.
    return pBuf;
} // End AddCommas().


/////////////////////////////////////////////////////////////////////////////////
// RestartSystem()
//
// Restarts the system, but clears the screen first to remove any leftover junk
// when the system comes back up.
/////////////////////////////////////////////////////////////////////////////////
void RestartSystem()
{
    // Clear the screen to eliminate junk display on restart.
    gTft.fillScreen(ST7735_BLACK);
    ESP.restart();
} // End RestartSystem().


/////////////////////////////////////////////////////////////////////////////////
// ResetNvs()
//
// Resets (clears) NVS by making each subsystem clear its own data, then restarts
// the system.  When the system comes back up, it will use all default values.
/////////////////////////////////////////////////////////////////////////////////
void ResetNvs()
{
    gLoadCell.Reset();
    gEnvSensor.Reset();
    gFilament.Reset();
    gSpoolMgr.Reset();
    gLengthMgr.Reset();
    gTft.Reset();
    MainScreen::Reset();

    // Reset the system.  This function never returns.
    RestartSystem();
} // End ResetNvs().


/////////////////////////////////////////////////////////////////////////////////
// SaveToNvs()
//
// Saves all system settings to NVS.  The current settings will all be saved to
// NVS and will be restored the next time the system comes up.
/////////////////////////////////////////////////////////////////////////////////
bool SaveToNvs()
{
    bool status = true;
    status &= gLoadCell.Save();
    status &= gEnvSensor.Save();
    status &= gFilament.Save();
    status &= gSpoolMgr.Save();
    status &= gLengthMgr.Save();
    status &= gTft.Save();
    status &= MainScreen::Save();
    return status;
} // End SaveToNvs().


/////////////////////////////////////////////////////////////////////////////////
// RestoreFromNvs()
//
// Restores all system settings from NVS.  Each subsystem is called to restore
// its non-volatile values.  If a system's volatile values were not previously
// saved, then default values are used.  This is called on each power-up.
/////////////////////////////////////////////////////////////////////////////////
bool RestoreFromNvs()
{
    // Assume that we will succeed.
    bool status = true;

    // Restore the LoadCell subsystem.
    if (gLoadCell.Restore())
    {
        // Restored successfully, update our globals accordingly.
        gScaleUnits = gLoadCell.GetUnits();
        gScaleAveragingSamples = gLoadCell.GetAverageInterval();
        gScaleGain = gLoadCell.GetGain();
    }
    else
    {
        // Restore failed.  Use our default values.
        gLoadCell.SetUnits(gScaleUnits);
        gLoadCell.SetAverageInterval(gScaleAveragingSamples);
        gLoadCell.SetGain(gScaleGain);
        SetLoadCellUnits(gScaleUnits);
        status = false;
        Serial.println("LoadCell.Restore() failed.");
    }

    // Restore the EnvSensor subsystem.
    if (gEnvSensor.Restore())
    {
        // Restored successfully, update our globals accordingly.
        gTemperatureUnits = gEnvSensor.GetTempScale();
    }
    else
    {
        // Restore failed.  Use our default values.
        gEnvSensor.SetTempScale(gTemperatureUnits);
        status = false;
        Serial.println("EnvSensor.Restore() failed.");
    }

    // Restore the Filament subsystem.
    if (gFilament.Restore())
    {
        // Restored successfully, update our globals accordingly.
        // No globals to update, so do nothing.
    }
    else
    {
        // Restore failed.  Use our default values.
        status = false;
        Serial.println("Filament.Restore() failed.");
    }

    // Restore the SpoolMgr subsystem.
    if (gSpoolMgr.Restore())
    {
        // Restored successfully, update our globals accordingly.
        // No globals to update, so do nothing.
    }
    else
    {
        // Restore failed.  Use our default values.
        status = false;
        Serial.println("SpoolManager.Restore() failed.");
    }
    // Either way, we need to update our spool offset value.
    SaveSpoolOffset();

    // Restore the LengthMgr subsystem.
    if (gLengthMgr.Restore())
    {
        // Restored successfully, update our globals accordingly.
        gLengthUnits = gLengthMgr.GetSelected();
    }
    else
    {
        // Restore failed.  Use our default values.
        gLengthMgr.SetUnits(gLengthUnits);
        status = false;
        Serial.println("LengthManager.Restore() failed.");
    }
    // Either way we need to update our global length factor.
    UpdateLengthFactor();

    // Restore the Display subsystem.
    if (gTft.Restore())
    {
        // Restored successfully, update our globals accordingly.
        gBacklightPercent = gTft.GetBacklightPercent();
    }
    else
    {
        // Restore failed.  Use our default values.
        gTft.SetBacklightPercent(gBacklightPercent);
        status = false;
        Serial.println("TFT.Restore() failed.");
    }

    // Restore the Main Screen subsystem.
    if (!MainScreen::Restore())
    {
        status = false;
        Serial.println("MainScreen::Restore() failed.");
    }

    return status;
} // End RestoreFromNvs().


/////////////////////////////////////////////////////////////////////////////////
// InitUnusedPins()
//
// Initialize all unused GPIO pins to pulled down inputs.
/////////////////////////////////////////////////////////////////////////////////
static void InitUnusedPins()
{
    // List of pins that are unused for this project.
    // !!! This list will need to be updated if the hardware setup changes.
    int unusedPins[] = {16, 17, 22, 23, 33};

    // Set each unused pin to a pulled down input.
    for (int i = 0; i < sizeof(unusedPins) / sizeof(unusedPins[0]); i++)
    {
        pinMode(unusedPins[i], INPUT_PULLDOWN);
    }
} // End InitUnusedPins().


/////////////////////////////////////////////////////////////////////////////////
// InitObjects()
//
// Initialize all sensors and other objects used by the scale.
//
// Returns:
//   Returns 'true' if all objects were successfully initialized.  Returns false
//   otherwise.
/////////////////////////////////////////////////////////////////////////////////
static bool InitObjects()
{
    // Assume that we will succeed.
    bool status = true;

    // Initialize the unused HW pins.
    InitUnusedPins();

    // Initialize the Display class.
    if (!gTft.Init(gTftNvsName) || !MainScreen::Init(gMainScreenNvsName))
    {
        Serial.println("No Display found.");
        status = false;
    }
    else
    {
        Serial.println("Display found.");
    }

    // Display a welcome screen.
    const uint32_t WELCOME_DWELL_MS = 2000;
    gTft.WelcomeScreen(MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, BOX_RADIUS);
    delay(WELCOME_DWELL_MS);

    // Initialize the menu subsystem.
    InitScaleMenus();

    // Initialize the load cell.
    if (!gLoadCell.Init(gLoadCellNvsName))
    {
        Serial.println("No Load Cell found.");
        status = false;
    }
    else
    {
        Serial.println("Load Cell found.");
    }

    // Initialize the environmental sensot.
    if (!gEnvSensor.Init(gEnvSensorNvsName))
    {
        Serial.println("No Environmental Sensor found.");
        status = false;
    }
    else
    {
        Serial.println("Environmental Sensor found.");
    }

    // Initialize the filament class.
    if (!gFilament.Init(gFilamentNvsName))
    {
        Serial.println("No Filament found.");
        status = false;
    }
    else
    {
        Serial.println("Filament found.");
    }

    // Initialize the spool manager class.
    if (!gSpoolMgr.Init(gSpoolMgrNvsName))
    {
        Serial.println("No SpoolMgr found.");
        status = false;
    }
    else
    {
        Serial.println("SpoolMgr found.");
    }

    // Initialize the length manager class.
    if (!gLengthMgr.Init(gLengthMgrNvsName))
    {
        Serial.println("No LengthMgr found.");
        status = false;
    }
    else
    {
        Serial.println("LengthMgr found.");
    }

    // Initialize the Network class.
    if (!gNetwork.Init(gNetworkNvsName, gNetworkApName, gNetworkServerName))
    {
        Serial.println("Network init failed.");
        status = false;
    }
    else
    {
        // Setup our network handlers.
        Serial.println("Network init succeeded.");
        WebData::InitNetworkHandlers();
    }

    // Restore previously saved state data for all subsystems if any.
    status &= RestoreFromNvs();

    // Return our status.
    return status;
} // End InitObjects().


/////////////////////////////////////////////////////////////////////////////////
// UpdateCurrentLength()
//
// Updates the current filament length if a spool is currently selected.
//
// Updates gCurrentLength.
/////////////////////////////////////////////////////////////////////////////////
static void UpdateCurrentLength()
{
    // Only calculate length if a spool is currently selected.
    if (gSpoolMgr.GetSelectedSpool() != NULL)
    {
        gCurrentLength = SetDecimalPlaces(
            gLengthFactor * gCurrentWeight, gLengthMgr.GetPrecision());
    }
    else
    {
        gCurrentLength = 0.0;
    }
} // End UpdateCurrentLength().


/////////////////////////////////////////////////////////////////////////////////
// UpdateCurrentWeight()
//
// Updates the current scale weight values.  Will not fetch new values from
// the load cell sensor any more frequently than every WEIGHT_UPDATE_PERIOD_MS
// milliseconds.
//
// Updates gCurrentWeight only if the load cell has been calibrated.
// Also updates the current length - gCurrentLength by calling
// UpdateCurrentLength().
/////////////////////////////////////////////////////////////////////////////////
static void UpdateCurrentWeight()
{
    // Don't update the weight too frequently.
    uint32_t currentMillis = millis();
    static uint32_t lastWeightTime = currentMillis - WEIGHT_UPDATE_PERIOD_MS;
    if (currentMillis - lastWeightTime >= WEIGHT_UPDATE_PERIOD_MS)
    {
        // It's time to update the weight value.  If we're calibrated, then
        // read the current weight from the sensor.  Otherwise, weight is 0.0.
        gCurrentWeight = 0.0f;
        if (gLoadCell.IsCalibrated())
        {
            gCurrentWeight =
                SetDecimalPlaces(gLoadCell.ReadWeight(), GetWeightDecimalPlaces());
            UpdateCurrentLength();
        }
        lastWeightTime = currentMillis;
    }
} // End UpdateCurrentWeight().


/////////////////////////////////////////////////////////////////////////////////
// UpdateCurrentEnv()
//
// Updates the temperature and humidity values.  Will not fetch new values from
// the environmental sensor any more frequently than every ENV_UPDATE_PERIOD
// milliseconds.  Also keeps count of any invalid data returned from the sensor.
//
// Updates gCurrentTemperature and gCurrentHumidity when sensor reading are OK.
/////////////////////////////////////////////////////////////////////////////////
static void UpdateCurrentEnv()
{
    // Don't update the environmental data too frequently.
    static const uint32_t ENV_UPDATE_PERIOD = 5000UL;
    uint32_t currentMillis = millis();
    static uint32_t lastEnvTime = currentMillis - ENV_UPDATE_PERIOD;
    if (currentMillis - lastEnvTime >= ENV_UPDATE_PERIOD)
    {
        // Handle temperature.
        float  envSensorReading = 0.0;
        static uint32_t tempreatureNanCount = 0UL;
        envSensorReading = gEnvSensor.GetTemperature();
        if (isnan(envSensorReading))
        {
            Serial.printf("\nTemp NAN: %d\n", ++tempreatureNanCount);
        }
        gCurrentTemperature = envSensorReading;

        // Handle humidity.
        static uint32_t humidityNanCount = 0UL;
        envSensorReading = gEnvSensor.GetHumidity();
        if (isnan(envSensorReading))
        {
            Serial.printf("\nHum NAN: %d\n", ++humidityNanCount);
        }
        gCurrentHumidity = envSensorReading;
        lastEnvTime = currentMillis;
    }
} // UpdateCurrentEnv().


/////////////////////////////////////////////////////////////////////////////////
// HandleMainScreen()
//
// Handles entry to, exit from, and running the main screen.
/////////////////////////////////////////////////////////////////////////////////
static void HandleMainScreen()
{
    const uint32_t SCREEN_UPDATE_TIME = 100;
    static bool firstTime = true;
    uint32_t currentUpdateTime = millis();
    static uint32_t lastUpdateTime = currentUpdateTime - SCREEN_UPDATE_TIME;

    // See if it's time to leave.
    int pbState = gEncStream.read();
    if ((pbState == options->navCodes[enterCmd].ch) ||
        (pbState == options->navCodes[escCmd].ch))
    {
        // Make sure we're allowed to enter the menu system.
        if (WebData::LockLocal())
        {
            // It is time to leave.  Reset and get ready to run the menu system.
            gNavRoot.reset();
            gNavRoot.idleOff();
            gRunningMenu = true;
            gTft.setTextSize(TEXT_SCALE, TEXT_SCALE);
            gTft.fillScreen(GetBgColor());
        }
        else
        {
            gTft.DisplayHVCenteredText("WEB IN USE\n  TRY LATER",
                                        ST7735_RED, ST7735_BLACK, BOX_RADIUS);
            delay(2000);
        }
        firstTime = true;
    }
    // Not time to leave.  If we just returned from displaying the menu system,
    // or if a setting has been changed, or if it's time to update the display,
    // or if the rotary encoder has been incremented, we need to update the display.
    else if (firstTime || gDataUpdated ||
            (currentUpdateTime - lastUpdateTime >= SCREEN_UPDATE_TIME) ||
            (pbState == options->navCodes[upCmd].ch) ||
            (pbState == options->navCodes[downCmd].ch))
    {
        // Do we need to scroll the scrollable data fields?
        int32_t scrollDir = 0;
        if (pbState == options->navCodes[upCmd].ch)
        {
            scrollDir = 1;
            firstTime = true;
        }
        else if (pbState == options->navCodes[downCmd].ch)
        {
            scrollDir = -1;
            firstTime = true;
        }
        if (gDataUpdated)
        {
            // Yes, set firstTime.
            firstTime = true;
        }

        // Time to update the main screen.
        MainScreen::DisplayMainScreen(firstTime, scrollDir);
        lastUpdateTime = currentUpdateTime;
        firstTime = false;
        gDataUpdated = false;
    }
} // End HandleMainScreen().


/////////////////////////////////////////////////////////////////////////////////
// HandleAuxPb()
//
// Monitor the aux pushbutton and perform actions based on the current system
// state.  If the system is currently displaying the menus, then exit to the
// main screen on any aux pushbutton press.
//
// If the system is currently displaying the main screen, then a short press of
// the aux pushbutton will cause the weight units to change.  A long press
// press will cause a tare operation to be completed.
/////////////////////////////////////////////////////////////////////////////////
void HandleAuxPb()
{
    // See if the aux pushbutton has been pushed (and released).
    int auxPbState = gAuxPb.Read();
    if (auxPbState != gAuxPb.BUTTON_CLEAR)
    {
        // Yes - pushbutton was activated.  Were we showing the main screen?
        if (!gRunningMenu)
        {
            // Yes - we were showing the main screen.  A short push commands
            // us to use the next weight units.  A long push indicates a tare
            // operation is required.
            if (auxPbState == gAuxPb.BUTTON_SHORT)
            {
                uint32_t tempScaleUnits = static_cast<uint32_t>(gScaleUnits);
                gScaleUnits = static_cast<WeightUnits>(++tempScaleUnits);
                // Short push - change scale units.
                if (gScaleUnits >= eWuNumUnits)
                {
                    gScaleUnits = eWuGrams;
                }
                SetLoadCellUnits(gScaleUnits);
                UpdateLengthFactorEntry();

                // Note that the units change is immediate so there is no need
                // to display a results screen.
            }
            else
            {
                // Let the user know we're working.
                gTft.DisplayWorkingScreen();

                // Short push, so do a tare.
                bool status = gLoadCell.Tare();

                // Let the user know if we succeeded or not.
                gTft.DisplayResult(status, "TARE COMPLETE", "TARE FAILED",
                                   BOX_RADIUS, 3000UL);
            }
            gDataUpdated = true;
        }
        // The local menu was running.  Any aux pb action causes us to exit
        // to the main screen, so we need to unlock the web as well.
        else
        {
            gTft.FillScreen(MAIN_PAGE_FG_COLOR, MAIN_PAGE_BG_COLOR, BOX_RADIUS);
            WebData::Unlock();
            gRunningMenu = false;
        }
    }
} // End HandleAuxPb().


/////////////////////////////////////////////////////////////////////////////////
// setup()
//
// Standard Arduino setup function.  Here we initialize the system for execution.
/////////////////////////////////////////////////////////////////////////////////
void setup()
{
    // Setup the serial port, and wait for it to be ready.
    Serial.begin(115200);
    while(!Serial);
    delay(100);
    Serial.println("--JMC SCALE--");

    // Delay to allow devices to settle.
    delay(250);

    // Initialize all of our system objects.
    if (InitObjects())
    {
        Serial.println("InitObjects() succeeded.");
    }
    else
    {
        Serial.println("InitObjects() failed.");
    }

    // Clear any aux pushbutton data.
    gAuxPb.Read();

    // Shouldn't be necessary, but just to be safe unlock the web mutex.
    WebData::Unlock();
} // End setup().


/////////////////////////////////////////////////////////////////////////////////
// loop()
//
// Standard Arduino main loop.  Here we mainly select whether to run the menu
// subsystem or the main display screen.
/////////////////////////////////////////////////////////////////////////////////
void loop()
{
    // Number of milliseconds to delay at the end of each loop() execution.
    const uint32_t LOOP_DELAY = 2;

    // Always update the scale weight and environmental data.
    UpdateCurrentWeight();
    UpdateCurrentEnv();

    // Always handle the network.
    gNetwork.Process();

    // If web had the lock and timed out, then clear the lock.
    WebData::HandleWebTimeout();

    // Handle the Aux Pushbutton.  Must be done before net/menu polling.
    HandleAuxPb();

    // Are we running the menu now?
    if (gRunningMenu)
    {
        // Yes, run the menu subsystem.
        gNavRoot.poll();
    }
    else
    {
        // No, display the main screen.
        HandleMainScreen();
    }

    // Delay to allow background to run.
    delay(LOOP_DELAY);
} // End loop().
