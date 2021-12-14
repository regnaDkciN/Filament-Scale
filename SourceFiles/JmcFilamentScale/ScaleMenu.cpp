/////////////////////////////////////////////////////////////////////////////////
// ScaleMenu.cpp
//
// Contains structures, data, and functions to handle the main menu system of
// the filament scale.
//
// History:
// - jmcorbett 10-JUN-2021 Original creation.
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
#include "ScaleMenu.h"          // For our own declarations, etc.
#include "WebData.h"            // for Unlock().
#include "MainScreen.h"         // For MainScreen class.
#include "HslColor.h"           // For RGB565 <=> HSL conversion.


using namespace Menu;           // For ArduinoMenu use.


/////////////////////////////////////////////////////////////////////////////////
// ArduinoMenu related globals and constants.
/////////////////////////////////////////////////////////////////////////////////
static const uint32_t MAX_DEPTH   = 5U;     // Maximum menu depth.

// Characters allowed on name field.
static const char* ALPHANUM =
    " 0123456789.ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
    ",\\|!\"#$%&/()=?~*^+-{}[]""\xf7";
static const char* ALPHANUM_MASK[] = {ALPHANUM};

static uint32_t gScrollSeconds = 0;     // Scroll delay in seconds.


/////////////////////////////////////////////////////////////////////////////////
// The serial input stream.
/////////////////////////////////////////////////////////////////////////////////
serialIn gSerial(Serial);


/////////////////////////////////////////////////////////////////////////////////
// The input chain.
/////////////////////////////////////////////////////////////////////////////////
MENU_INPUTS(gIn, &gEncStream, &gSerial);

// Note: MENU_OUTPUTS and NAVROOT are declared near the bottom of the file due
//       to their dependence on other menu data.


/////////////////////////////////////////////////////////////////////////////////
// Menu related color variables and constants.
/////////////////////////////////////////////////////////////////////////////////
static const uint16_t ST7735_GRAY = RGB565(128,128,128);

static const int NORMAL_COLOR   = 0;
static const int SELECTED_COLOR = 1;
static const int EDITING_COLOR  = 2;
const colorDef<uint16_t> gColorsTable[nColors] MEMMODE =
{
//  {disabled:   normal               selected  }
//    {enabled:  normal,                 selected,                editing    }
    {{(uint16_t)DARK_BLUE,     (uint16_t)DARK_BLUE},
      {(uint16_t)DARK_BLUE,    (uint16_t)ST7735_BLUE,   (uint16_t)ST7735_WHITE}}, // bgColor
    {{(uint16_t)ST7735_YELLOW, (uint16_t)ST7735_GRAY},
      {(uint16_t)ST7735_WHITE, (uint16_t)ST7735_WHITE,  (uint16_t)ST7735_WHITE}}, // fgColor
    {{(uint16_t)ST7735_WHITE,  (uint16_t)ST7735_BLACK},
      {(uint16_t)ST7735_YELLOW,(uint16_t)ST7735_YELLOW, (uint16_t)ST7735_RED}},   // valColor
    {{(uint16_t)ST7735_WHITE,  (uint16_t)ST7735_BLACK},
      {(uint16_t)ST7735_WHITE, (uint16_t)ST7735_YELLOW, (uint16_t)ST7735_YELLOW}},// unitColor
    {{(uint16_t)ST7735_WHITE,  (uint16_t)ST7735_GRAY},
      {(uint16_t)DARK_BLUE,    (uint16_t)ST7735_GREEN,  (uint16_t)ST7735_WHITE}}, // cursorColor
    {{(uint16_t)ST7735_WHITE,  (uint16_t)ST7735_YELLOW},
      {(uint16_t)ST7735_WHITE, (uint16_t)ST7735_RED,    (uint16_t)ST7735_RED}},   // titleColor
};
uint16_t GetBgColor() { return gColorsTable[bgColor].disabled[NORMAL_COLOR]; }
static const uint16_t WARNING_COLOR = (uint16_t)ST7735_RED;
static const uint16_t ENABLED_TEXT_COLOR  = gColorsTable[fgColor].enabled[NORMAL_COLOR];
static const uint16_t DISABLED_TEXT_COLOR = gColorsTable[fgColor].disabled[NORMAL_COLOR];


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
// Miscellaneous menu related functions.
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
// MenuIdle()
//
// Called when exiting the menu system.
/////////////////////////////////////////////////////////////////////////////////
result MenuIdle(menuOut& o, idleEvent e)
{
    // As long as we're not re-entering the menu subsystem.
    if (e != idleEnd)
    {
        // Remember that we're not running the menu system.
        gRunningMenu = false;
        WebData::Unlock();
    }
    return proceed;
} // End MenuIdle().


/////////////////////////////////////////////////////////////////////////////////
// SkipItemDown()
//
// Causes the menu system to skip the current item by moving the cursor down
// one item.  It does this by simulating a down encoder event.
/////////////////////////////////////////////////////////////////////////////////
static result SkipItemDown(eventMask e)
{
    if (e != blurEvent)
    {
        gEncStream.incEncoder();
    }
    return proceed;
} // End SkipItemDown().


/////////////////////////////////////////////////////////////////////////////////
// SkipItemUpDown()
//
// Causes the menu system to skip the current item by moving the cursor up or
// down one item, based on the direction the encoder was last movedn.  For
// example, if the previous encoder direction was UP, then the cursor is moved
// up one more position.  It does this by simulating an up or down encoder event
// as needed.
/////////////////////////////////////////////////////////////////////////////////
static result SkipItemUpDown(eventMask e)
{
    if (e != blurEvent)
    {
        // Was the previous direction UP?
        if (gEncStream.GetLastEncoderDirection() == options->navCodes[downCmd].ch)
        {
            // Yes it was UP.  Bump it one more.
            gEncStream.decEncoder();
        }
        else
        {
            // No, it was DOWN.  Bump it one more.
            gEncStream.incEncoder();
        }
    }
} // End SkipItemUpDown().

/////////////////////////////////////////////////////////////////////////////////
// class WeightField
//
// This class is used to assist in editing weight fields.  Its main function is
// to enable variable min, max, big step, and small step values based on current
// system values.
/////////////////////////////////////////////////////////////////////////////////
template<typename T>
class WeightField : public menuField<T>
{   //https://github.com/neu-rah/ArduinoMenu/blob/master/examples/customField/customField/customField.ino
public:
    menuFieldShadow<T>* m_shadow = NULL;
    WeightField(constMEM menuFieldShadow<T>& shadow) : menuField<T>(shadow)
    {
       m_shadow = &shadow;
    }
    WeightField(
        T &value,
        constText *text,
        constText *units,
        T low,
        T high,
        T step,
        T tune,
        action a = doNothing,
        eventMask e = noEvent,
        styles s = noStyle) :
            WeightField(*new menuFieldShadow<T>
                (value, text, units, low, high, step, tune, a, e, s)) {}

    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len,
        idx_t panelNr = 0) override
    {   // https://github.com/neu-rah/ArduinoMenu/issues/94#issuecomment-290936646
        // Here is where we use units based values for min, max, big and small step.
        ((menuFieldShadow<T>*)m_shadow)->low  = gMinWeight;
        ((menuFieldShadow<T>*)m_shadow)->high = gMaxWeight;
        ((menuFieldShadow<T>*)m_shadow)->step = gBigWeightStep;
        ((menuFieldShadow<T>*)m_shadow)->tune = gSmallWeightStep;

        menuField<T>::reflex = menuField<T>::target();
        idx_t l = prompt::printTo(root, sel, out, idx, len);
        if (l < len)
        {
            bool isEditing = (root.navFocus == this && sel);
            out.setColor(valColor, true, enabledStatus, isEditing);
            out.print(isEditing ? (menuField<T>::tunning ?
                      fieldBase::TunningCursor : fieldBase::NonTunningCursor) : " ");
            l += 2;
            if (l < len)
            {
                //NOTE: this can exceed the limits!
                l += out.print(menuField<T>::reflex, GetWeightDecimalPlaces());
                if (l < len)
                {
                    l += print_P(out, gLoadCell.GetUnitsString(), len);
                }
            }
        }
        return l;
    }

    void setDecimals(idx_t d) {  }
    idx_t getDecimals(void)   { return(GetWeightDecimalPlaces()); }
}; // End class WeightField.

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//////////////////   M E N U S   S T A R T   H E R E   //////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// DISPLAY MENU ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
////////////////////////////// SCALE UNITS MENU /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static result SetScaleUnits(eventMask e)
{
    static WeightUnits lastUnits = eWuBadVal;

    // Only update the units if they have changed.
    if (lastUnits != gScaleUnits)
    {
        SetLoadCellUnits(gScaleUnits);
        UpdateLengthFactor();
        lastUnits = gScaleUnits;
    }
    return proceed;
} // End SetScaleUnits().

TOGGLE(gScaleUnits, ScaleUnitsMenu, " Weight: ", doNothing, noEvent, wrapStyle
    , VALUE("g ", eWuGrams,     SetScaleUnits, enterEvent)
    , VALUE("kg", eWuKiloGrams, SetScaleUnits, enterEvent)
    , VALUE("oz", eWuOunces,    SetScaleUnits, enterEvent)
    , VALUE("lb", eWuPounds,    SetScaleUnits, enterEvent)
); // End ScaleUnitsMenu.


/////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// LENGTH UNITS //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// UpdateLengthFactorEntry()
//
// Updates the current length units, then if a spool is selected update
// our length factor.  Otherwise, save a value of 0.0.
/////////////////////////////////////////////////////////////////////////////////
void UpdateLengthFactorEntry()
{
    gLengthMgr.SetUnits(gLengthUnits);
    UpdateLengthFactor();
}

result UpdateLengthFactorEntryMenu()
{
    UpdateLengthFactorEntry();
    return proceed;
} // End UpdateLengthFactorEntry().

TOGGLE(gLengthUnits, LengthUnitsMenu, " Length: ", doNothing, noEvent, wrapStyle
    , VALUE(UNIT_STRING_MM, luMm, UpdateLengthFactorEntryMenu, enterEvent)
    , VALUE(UNIT_STRING_CM, luCm, UpdateLengthFactorEntryMenu, enterEvent)
    , VALUE(UNIT_STRING_M,  luM,  UpdateLengthFactorEntryMenu, enterEvent)
    , VALUE(UNIT_STRING_IN, luIn, UpdateLengthFactorEntryMenu, enterEvent)
    , VALUE(UNIT_STRING_FT, luFt, UpdateLengthFactorEntryMenu, enterEvent)
    , VALUE(UNIT_STRING_YD, luYd, UpdateLengthFactorEntryMenu, enterEvent)
); // End LengthUnitsMenu.


/////////////////////////////////////////////////////////////////////////////////
//////////////////////////// TEMPERATURE UNITS //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static result SetTemperatureUnits()
{
    gEnvSensor.SetTempScale(gTemperatureUnits);
    return proceed;
} // End SetTemperatureUnits().

// Would like to use gEnvSensor.GetTempScaleString() for string values here,
// but can't find a way to make it work.
TOGGLE(gTemperatureUnits, TemperatureUnitsMenu, " Temp.:  ", doNothing, noEvent, wrapStyle
    , VALUE("\xf7""F", eTempScaleF, SetTemperatureUnits, enterEvent)
    , VALUE("\xf7""C", eTempScaleC, SetTemperatureUnits, enterEvent)
);


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// BACKLIGHT ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static result SetTftLitePercent()
{
    gTft.SetBacklightPercent(gBacklightPercent);
    return proceed;
} // End SetTftLitePercent().


/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// SCROLL DELAY ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static result SetScrollDelay()
{
    MainScreen::SetScrollDelayMs(1000 * gScrollSeconds);
}

/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// DISPLAY MENU ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
class DisplayMenuOverride : public menu
{
public:
    DisplayMenuOverride(constMEM menuNodeShadow& shadow):menu(shadow) {}
    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t p)
      override
    {
        if(idx < 0)
        {   // Display title menu item.
            menu::printTo(root, sel, out, idx, len, p);
        }
        else
        {   // Display selection menu item.
            out.printRaw((constText*)F(" DISPLAY    " RIGHT_ARROW), len);
        }
        return idx;
    } // End printTo().
}; // End class DisplayMenuOverride.


static result UpdateScrollDelay(eventMask e)
{
    gScrollSeconds = MainScreen::GetScrollDelayMs() / 1000;
    return SkipItemUpDown(e);
}

static result DisableDisplayItems();
altMENU(DisplayMenuOverride, DisplayMenu, "   DISPLAY",
        DisableDisplayItems, enterEvent, noStyle,
        (Menu::_menuData | Menu::_canNav)
    , SUBMENU(ScaleUnitsMenu)
    , SUBMENU(LengthUnitsMenu)
    , SUBMENU(TemperatureUnitsMenu)
    , OP(" Backlight:", SkipItemUpDown, anyEvent)
    , FIELD(gBacklightPercent, "        ", "%", 5, 100, 5, 0, SetTftLitePercent,
            enterEvent, noStyle)
    , OP(" Scroll Wait:", UpdateScrollDelay, anyEvent)
    , FIELD(gScrollSeconds, "     ", " sec", 0, MainScreen::MAX_SCROLL_DELAY_SEC,
            MainScreen::SCROLL_DELAY_STEP_SEC, 0, SetScrollDelay, enterEvent, noStyle)
    , EXIT(BACK_STRING)
); // End DisplayMenu.


static result DisableDisplayItems()
{
    return proceed;
} // End DisableDisplayItems().


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// SCALE MENU ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
////////////////////////////// CALIBRATE DONE MENU //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static result HandleCalibrationSetLoadDone()
{
    // Show the user that we're working on the problem.
    gTft.DisplayWorkingScreen();

    // Perform the calibration operation.
    bool status = gLoadCell.Calibrate(0, gCalibrateWeight);

    // Let the user know if we succeeded or not.
    gTft.DisplayResult(status, "CAL COMPLETE", " CAL FAILED", BOX_RADIUS, 3000UL);

    // Setup to return to the home menu.
    gNavRoot.reset();

    // Make sure the screen background gets reset.
    gTft.fillScreen(GetBgColor());

    // Return our status.
    return quit;
}  // End HandleCalibrationSetLoadDone.


/////////////////////////////////////////////////////////////////////////////////
////////////////////////// SET CALIBRATION WEIGHT MENU //////////////////////////
/////////////////////////////////////////////////////////////////////////////////
class SetCalibrationWeightMenuOverride : public menu
{
public:
  SetCalibrationWeightMenuOverride(constMEM menuNodeShadow& shadow):menu(shadow) {}
  Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t p)
    override
    {
        if(idx < 0)
        {   // Display title menu item.
            char buf[SCREEN_CHAR_WIDTH + 1];
            snprintf(buf, sizeof(buf), "SET LOAD (%s)",
                    &(gLoadCell.GetUnitsString()[1]));
            out.printRaw(buf, len);
        }
        else
        {   // Display selection menu item.
            out.printRaw((constText*)F("       Ready" RIGHT_ARROW), len);
        }
      return idx;
    } // End printTo().
};  // End SetCalibrationWeightMenuOverride.

static result HandleCalibrationEmpty(eventMask e);
altMENU(SetCalibrationWeightMenuOverride, SetCalibrationWeightMenu, "  SET LOAD",
        HandleCalibrationEmpty, enterEvent, noStyle, (Menu::_menuData | Menu::_canNav)
    , OP("Add Load and",  SkipItemDown, anyEvent)
    , OP("Set Weight.",   SkipItemDown, anyEvent)
    , altFIELD(WeightField, gCalibrateWeight, " ", "", gMinWeight, gMaxWeight,
               gBigWeightStep, gSmallWeightStep, doNothing, noEvent, noStyle)
    , OP("",              SkipItemUpDown, anyEvent)
    , OP("       Ready" RIGHT_ARROW, HandleCalibrationSetLoadDone, enterEvent)
    , EXIT("<Cancel")
); // End SetCalibrationWeightMenu.

static result HandleCalibrationEmpty(eventMask e)
{
    // Show the user that we're working on the problem.
    gTft.DisplayWorkingScreen();

    // Perform the tare operation.  Retry a few times if needed.
    bool success = false;
    const uint16_t MAX_ZERO_RETRIES = 5;
    for (uint16_t i = 0; !success && (i < MAX_ZERO_RETRIES); i++)
    {
        success = gLoadCell.Tare();
    }

    // Let the user know if we succeeded or not.
    gTft.DisplayResult(success, "ZERO COMPLETE", "ZERO FAILED", BOX_RADIUS, 3000UL);

    // If we succeeded then continue to the next screen.
    result status = proceed;
    if (success)
    {
        // Disable the non-selectable menu items.
        SetCalibrationWeightMenu[0].disable();
        SetCalibrationWeightMenu[1].disable();
        SetCalibrationWeightMenu[3].disable();

        // We were successful.  Setup to display the Save/NoSave menu and bump
        // the cursor down to the first valid choice.
        if (e != selBlurEvent)
        {
            gEncStream.incEncoder();
        }
    }
    else
    {
        // The tare failed.  Setup to return to the home menu.  That is, we
        // will bypass the TareDoneMenu and go straight to the home menu.
        gNavRoot.reset();
        status = quit;
    }

    // Make sure the screen background gets reset.
    gTft.fillScreen(GetBgColor());

    // Return our status.
    return status;
} // End HandleCalibrationEmpty().


/////////////////////////////////////////////////////////////////////////////////
//////////////////////// CALIBRATE CONFIRM EMPTY MENU ///////////////////////////
/////////////////////////////////////////////////////////////////////////////////
class CalibrateEmptyMenuOverride : public menu
{
public:
    CalibrateEmptyMenuOverride(constMEM menuNodeShadow& shadow):menu(shadow) {}
    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t p)
      override
    {
        if(idx < 0)
        {   // Display title menu item.
            menu::printTo(root, sel, out, idx, len, p);
        }
        else
        {   // Display selection menu item.
            out.printRaw((constText*)F(" CALIBRATE  " RIGHT_ARROW), len);
        }
        return idx;
    } // End printTo().
}; // End class CalibrateEmptyMenuOverride.

static result DisableConfirmScalEmptyItems(eventMask e);
altMENU(CalibrateEmptyMenuOverride, CalibrateEmptyMenu, " EMPTY SCALE",
        DisableConfirmScalEmptyItems, enterEvent, noStyle,
        (Menu::_menuData | Menu::_canNav)
    , OP("Remove Load.",  SkipItemDown, anyEvent)
    , OP("Press Select",  SkipItemDown, anyEvent)
    , OP("When Ready.",   SkipItemDown, anyEvent)
    , OP("",              SkipItemDown, anyEvent)
    , SUBMENU(SetCalibrationWeightMenu)
    , EXIT("<Cancel")
); // End CalibrateEmptyMenu.

static result DisableConfirmScalEmptyItems(eventMask e)
{
    // Disable the non-selectable menu items.
    CalibrateEmptyMenu[0].disable();
    CalibrateEmptyMenu[1].disable();
    CalibrateEmptyMenu[2].disable();
    CalibrateEmptyMenu[3].disable();
    // If we enter this item, bump the encoder to move down one space.
    if (e != selBlurEvent)
    {
        gEncStream.incEncoder();
    }
    return proceed;
} // End DisableConfirmScalEmptyItems().


/////////////////////////////////////////////////////////////////////////////////
///////////////////////////// TARE EMPTY MENU ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static result HandleReadyForTare()
{
    // Show the user that we're working on the problem.
    gTft.DisplayWorkingScreen();

    // Perform the tare operation.  Retry a few times if needed.
    bool success = false;
    const uint16_t MAX_TARE_RETRIES = 5;
    for (uint16_t i = 0; !success && (i < MAX_TARE_RETRIES); i++)
    {
        success = gLoadCell.Tare();
    }

    // Let the user know if we succeeded or not.
    gTft.DisplayResult(success, "TARE COMPLETE", "TARE FAILED", BOX_RADIUS, 3000UL);

    // Make sure the screen background gets reset.
    gTft.fillScreen(GetBgColor());

    // Return our status.
    return quit;
} // End HandleReadyForTare().


class TareMenuOverride : public menu
{
public:
    TareMenuOverride(constMEM menuNodeShadow& shadow):menu(shadow) {}
    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t p)
      override
    {
        if(idx < 0)
        {   // Display title menu item.
            menu::printTo(root, sel, out, idx, len, p);
        }
        else
        {   // Display selection menu item.
            out.printRaw((constText*)F(" TARE (ZERO)" RIGHT_ARROW), len);
        }
        return idx;
    } // End printTo().
}; // End class TareMenuOverride.

static result DisableTareEmptyItems(eventMask e);
altMENU(TareMenuOverride, TareMenu, " TARE (ZERO)",
        DisableTareEmptyItems, enterEvent, noStyle,  (Menu::_menuData | Menu::_canNav)
    , OP("Remove Load.",  SkipItemDown, anyEvent)
    , OP("Press Select",  SkipItemDown, anyEvent)
    , OP("When Ready.",   SkipItemDown, anyEvent)
    , OP("",              SkipItemDown, anyEvent)
    , OP("       Ready" RIGHT_ARROW, HandleReadyForTare, enterEvent)
    , EXIT("<Cancel")
); // End TareMenu.

static result DisableTareEmptyItems(eventMask e)
{
    // Disable the non-selectable menu items.
    TareMenu[0].disable();
    TareMenu[1].disable();
    TareMenu[2].disable();
    TareMenu[3].disable();
    // If we enter this item, bump the encoder to move down one space.
    if (e != selBlurEvent)
    {
        gEncStream.incEncoder();
    }
    return proceed;
} // End DisableTareEmptyItems().


/////////////////////////////////////////////////////////////////////////////////
///////////////////////////// SCALE GAIN MENU ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static result UpdateScaleGain()
{
    gLoadCell.SetGain(gScaleGain);
    return proceed;
} // End UpdateScaleGain().

TOGGLE(gScaleGain, ScaleGainMenu, " Gain:  ", doNothing, noEvent, wrapStyle
    , VALUE("x 64" , 64,  UpdateScaleGain, enterEvent)
    , VALUE("x 128", 128, UpdateScaleGain, enterEvent)
); // End ScaleGainMenu.


/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// SCALE MENU ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static result SetRunningAverage(eventMask e)
{
    gLoadCell.SetAverageInterval(gScaleAveragingSamples);
    return proceed;
} // End SetRunningAverage().


class EnterScaleMenuOverride : public menu
{
public:
    EnterScaleMenuOverride(constMEM menuNodeShadow& shadow) : menu(shadow) { }
    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t p)
        override
    {
        if (idx < 0)
        {   // Display title menu item.
            menu::printTo(root, sel, out, idx, len, p);
        }
        else
        {   // Display selection menu item.
            out.printRaw((constText *)F(" SCALE      " RIGHT_ARROW), len);
        }
        return idx;
    } // End printTo().
}; // End class EnterScaleMenuOverride.

altMENU(EnterScaleMenuOverride, ScaleMenu, "SCALE OPTIONS", doNothing, noEvent,
        noStyle, (Menu::_menuData | Menu::_canNav)
    , SUBMENU(TareMenu)
    , SUBMENU(CalibrateEmptyMenu)
    , FIELD(gScaleAveragingSamples, " Avg:     ", "", AVG_SAMPLES_MIN,
               AVG_SAMPLES_MAX, AVG_SAMPLES_BIG_STEP, AVG_SAMPLES_SMALL_STEP,
               SetRunningAverage, anyEvent, noStyle)
    , SUBMENU(ScaleGainMenu)
    , OP("", SkipItemUpDown, anyEvent)
    , EXIT(BACK_STRING)
); // End ScaleMenu.


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
///////////////////////////// SPOOL TABLE MENU //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static void DeselectWorkingSpool()
{
    gSpoolMgr.DeselectSpool();
    gWorkingSpoolData.m_SelectedOnEntry = false;
} // End DeselectWorkingSpool().

static void SelectWorkingSpool(size_t index)
{
    gSpoolMgr.SelectSpool(index);
    gWorkingSpoolData.m_SelectedOnEntry = true;
} // End SelectWorkingSpool().

// Save the edited data record
static result ChangeColor();
static result SaveWorkingSpoolInfo(eventMask e, navNode& nav)
{
    navNode& nn = nav.root->path[nav.root->level - 1];
    idx_t n = nn.sel; // Get selection of previous level

    // If this spool was previously selected, but is no longer selected,
    // then deselect the current spool.
    if (gWorkingSpoolData.m_SelectedOnEntry &&
       !gWorkingSpoolData.m_SelectedOnExit)
    {
        DeselectWorkingSpool();
    }
    // If this spool was not previously selected, but now is selected,
    // then select the current spool.
    else if (!gWorkingSpoolData.m_SelectedOnEntry &&
             gWorkingSpoolData.m_SelectedOnExit)
    {
        SelectWorkingSpool(n);
    }

    gWorkingSpoolData.m_SelectedOnEntry = gWorkingSpoolData.m_SelectedOnExit;
    Spool *pTargetSpool = gSpoolMgr.GetSpool(n);

    // Before we save the (possibly) new spool name, we need to remove any
    // trailing spaces from it.
    size_t nameLength = strlen(gWorkingSpoolData.m_Name);

    if (nameLength)
    {
        while (gWorkingSpoolData.m_Name[--nameLength] == ' ')
        { }
        gWorkingSpoolData.m_Name[++nameLength] = '\0';
    }
    pTargetSpool->SetName(gWorkingSpoolData.m_Name);

    pTargetSpool->SetType(gWorkingSpoolData.m_Type);
    pTargetSpool->SetDensity(gWorkingSpoolData.m_Density);
    pTargetSpool->SetDiameter(gWorkingSpoolData.m_Diameter);
    pTargetSpool->SetSpoolWeight(gWorkingSpoolData.m_SpoolWeight);
    pTargetSpool->SetColor(gWorkingSpoolData.m_Color);

    // Update the length factor used to calculate length given wieght.
    UpdateLengthFactor();

    SaveSpoolOffset();

    return quit;
} // End SaveWorkingSpoolInfo().

static result UpdateSpoolDensity(eventMask e)
{
    // Set the working density to the default for the selected filament type.
    gWorkingSpoolData.m_Density = Filament::GetDensity(gWorkingSpoolData.m_Type);
    return proceed;
} // End UpdateSpoolDensity().

TOGGLE(gWorkingSpoolData.m_Type, EditTypeMenu, "Type  : ",
       doNothing, noEvent, wrapStyle
    , VALUE(FILAMENT_STRING_ABS,    eFtAbs,   UpdateSpoolDensity, anyEvent)
    , VALUE(FILAMENT_STRING_ASA,    eFtAsa,   UpdateSpoolDensity, anyEvent)
    , VALUE(FILAMENT_STRING_COPPER, eFtCopr,  UpdateSpoolDensity, anyEvent)
    , VALUE(FILAMENT_STRING_HIPS,   eFtHips,  UpdateSpoolDensity, anyEvent)
    , VALUE(FILAMENT_STRING_NYLON,  eFtNylon, UpdateSpoolDensity, anyEvent)
    , VALUE(FILAMENT_STRING_PETG,   eFtPetg,  UpdateSpoolDensity, anyEvent)
    , VALUE(FILAMENT_STRING_PLA,    eFtPla,   UpdateSpoolDensity, anyEvent)
    , VALUE(FILAMENT_STRING_PMMA,   eFtPmma,  UpdateSpoolDensity, anyEvent)
    , VALUE(FILAMENT_STRING_POLYC,  eFtPlyC,  UpdateSpoolDensity, anyEvent)
    , VALUE(FILAMENT_STRING_PVA,    eFtPva,   UpdateSpoolDensity, anyEvent)
    , VALUE(FILAMENT_STRING_TPE,    eFtTpe,   UpdateSpoolDensity, anyEvent)
    , VALUE(FILAMENT_STRING_TPU,    eFtTpu,   UpdateSpoolDensity, anyEvent)
    , VALUE(FILAMENT_STRING_USER1,  eFtUser1, UpdateSpoolDensity, anyEvent)
    , VALUE(FILAMENT_STRING_USER2,  eFtUser2, UpdateSpoolDensity, anyEvent)
    , VALUE(FILAMENT_STRING_USER3,  eFtUser3, UpdateSpoolDensity, anyEvent)
); // End EditTypeMenu.

TOGGLE(gWorkingSpoolData.m_SelectedOnExit, SelectSpoolMenu, "Select: ",
        doNothing, noEvent, wrapStyle
    , VALUE("Yes", true,  doNothing, noEvent)
    , VALUE("No",  false, doNothing, noEvent)
); // End SelectSpoolMenu.



/////////////////////////////////////////////////////////////////////////////////
//           B E G I N   O F   C O L O R   P I C K E R   C O D E
// WARNING: The following code contains some major kludges.  I couldn't figure
//          a clean way to display the filament color within the ArduinoMenu
//          system.  As a result, the following section of code related to
//          displaying and modifying the filament color contains some major
//          hacks.  I've noted some of the biggest ones with comments.
/////////////////////////////////////////////////////////////////////////////////
static uint32_t gHue = 0;   // Hue of current spool's filament color.
static uint32_t gSat = 0;   // Saturation of the current spool's filament color.
static uint32_t gLum = 0;   // Luminosity of the current spool's filament color.
static HslColor gHsl;       // The HSL object representing the current spool color.

class ColorMenuOverride : public menu
{
public:
    ColorMenuOverride(constMEM menuNodeShadow& shadow):menu(shadow) {}

    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t p)
      override
    {
        if(idx < 0)
        {   // Display title menu item.
            menu::printTo(root, sel, out, idx, len, p);
            /////////////////////////////////////////////////////////////////////
            // When we display the color selection menu for the first time we
            // need to update our color box.
            /////////////////////////////////////////////////////////////////////
            ChangeColor();
        }
        else
        {   // Display selection menu item.
            out.printRaw((constText*)F("Color       " RIGHT_ARROW), len);
        }
        return idx;
    } // End printTo().
}; // End class ColorMenuOverride.

static result DisableColorItems();

/////////////////////////////////////////////////////////////////////////////////
// This function displays the filament color box on the color selection screen.
// It partially bypasses the menu system, but is called each time the user changes
// an HSL parameter (as well as other times).
/////////////////////////////////////////////////////////////////////////////////
static result ChangeColor()
{
    // Get the current HSL values which may have been changed by the user.
    gHsl.SetHue((float)gHue);
    gHsl.SetSat((float)gSat);
    gHsl.SetLum((float)gLum);

    // Generate the corresponding RGB565 color and update our working color.
    uint16_t bgColor = gHsl.ToRgb565();
    gWorkingSpoolData.m_Color = bgColor;

    // Display the color box on lines 4 and 5 of the currently displayed menu.
    int w = gTft.width();
    int h = gTft.height();
    int x0 = 4;
    int y0 = 4 * h / 7 + 4;
    int xw = w - 8;
    int yh = 2 * h / 7 - 10;
    gTft.fillRoundRect(x0, y0, xw, yh, 8, bgColor);
    gTft.drawRoundRect(x0, y0, xw, yh, 8, 0xffff);

    return proceed;
}

/////////////////////////////////////////////////////////////////////////////////
// Skip past the color box, then re-draw it since the menu system will have already
// overwritten it.
/////////////////////////////////////////////////////////////////////////////////
static result SkipColor(eventMask e)
{
    SkipItemUpDown(e);
    ChangeColor();
    return proceed;
}

/////////////////////////////////////////////////////////////////////////////////
// On exit from the color picker menu, exit.  On other actions, update the color
// box.  This is needed since when the exit menu item gets or loses focus, this
// method gets called in addition to when the exit is selected.
/////////////////////////////////////////////////////////////////////////////////
static result UpdateColorOrExit(eventMask e)
{
    if (e == enterEvent)
    {
        return quit;
    }
    return ChangeColor();
}

/////////////////////////////////////////////////////////////////////////////////
// The color picker menu.  Note that items 4 and 5 are empty OP items.  These two
// lines get overwritten any tim ChangeColor() is executed.
/////////////////////////////////////////////////////////////////////////////////
altMENU(ColorMenuOverride, EditColorMenu, "    COLOR",
        DisableColorItems, enterEvent, noStyle,
        (Menu::_menuData | Menu::_canNav)
    , FIELD(gHue, " Hue: ", "", 0, 360, 10, 1, ChangeColor, anyEvent, noStyle)
    , FIELD(gSat, " Sat: ", "", 0, 100, 5, 1,  ChangeColor, anyEvent, noStyle)
    , FIELD(gLum, " Lum: ", "", 0, 100, 5, 1,  ChangeColor, anyEvent, noStyle)
    , OP("", SkipColor, anyEvent)
    , OP("", SkipColor, anyEvent)
    , OP(BACK_STRING, UpdateColorOrExit, anyEvent)
);

/////////////////////////////////////////////////////////////////////////////////
// This function is called on entry to the color picker menu and thus needs to
// update the color box.
/////////////////////////////////////////////////////////////////////////////////
static result DisableColorItems()
{
    EditColorMenu[3].disable();
    EditColorMenu[4].disable();
    return ChangeColor();
}

/////////////////////////////////////////////////////////////////////////////////
//              E N D   O F   C O L O R   P I C K E R   C O D E
/////////////////////////////////////////////////////////////////////////////////



static result DisableSpoolInfoEditItems();
MENU(SpoolInfoEditMenu, " SPOOL INFO", DisableSpoolInfoEditItems, enterEvent, noStyle
    , SUBMENU(SelectSpoolMenu)
    , OP("Spool ID:", SkipItemUpDown, anyEvent)
    , EDIT("", gWorkingSpoolData.m_Name, ALPHANUM_MASK, doNothing, noEvent, wrapStyle)
    , altFIELD(WeightField, gWorkingSpoolData.m_SpoolWeight, "Wt:", "g",
            0.0, 999.9, 10.0, 0.1, doNothing, noEvent, noStyle)
    , SUBMENU(EditColorMenu)
    , SUBMENU(EditTypeMenu)
    , OP("Density:", SkipItemUpDown, anyEvent)
    , altFIELD(decPlaces<2>::menuField, gWorkingSpoolData.m_Density,
            "  ", " g/cc", Filament::MIN_DENSITY, Filament::MAX_DENSITY,
            0.20, 0.01, doNothing, noEvent, noStyle)
    , altFIELD(decPlaces<2>::menuField, gWorkingSpoolData.m_Diameter, "Dia mm:", "",
            1.00, 4.00, 0.20, 0.01, doNothing, noEvent, noStyle)
    , OP(BACK_STRING, SaveWorkingSpoolInfo, enterEvent)
); // End SpoolInfoEditMenu.

static result DisableSpoolInfoEditItems()
{
    // Disable the non-selectable menu items.
    SpoolInfoEditMenu[1].disable();
    SpoolInfoEditMenu[6].disable();
    return ChangeColor();
}


// Customized print for Spool Info table menu items.
// Menu system wil use this to print the list of all spools.
struct SpoolInfoMenu : UserMenu
{
    using UserMenu::UserMenu;

    // Customizing the print of spool items (len is the availabe space).
    Used printItem(menuOut& out, int idx, int len) override
    {
        if (idx < NUMBER_SPOOLS)
        {
            // Display the name of the spool using its color as the background.
            char buf[SCREEN_CHAR_WIDTH + 2];

            // Determine if this spool is selected and setup an indicating character.
            const char *pSelChar =
                (idx == gSpoolMgr.GetSelectedSpoolIndex()) ? RIGHT_ARROW : " ";
            // Display the indicating character in the normal menu colors.
            len = out.printText(pSelChar, len);

            // Determine the spool's color and generate a contrasting foreground
            // color.
            Spool *pSpool = gSpoolMgr.GetSpool(idx);
            uint16_t bgColor = pSpool->GetColor();
            uint16_t fgColor = HslColor::Contrast(bgColor);
            gTft.setTextColor(fgColor, bgColor);

            // Generate and display the spool name string using the spool's color
            // as the background color and a contrasting color as the foreground.
            // Extend the name of the spool to the entire screen width so that
            // the background color will fill the remainder of the display line.
            snprintf(buf, sizeof(buf), "%-13s", gSpoolMgr.GetSpool(idx)->GetName());
            return len ? out.printText(buf, len) : 0;
        }
        else
        {
            return out.printText(BACK_STRING, len);
        }
    } // End printItem().

    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t p)
        override
    {
        if (idx < 0)
        {   // Display title menu item.
            return UserMenu::printTo(root, sel, out, idx, len, p);
        }
        else if (root.navFocus != this)
        {
            return out.printRaw(" SPOOL      " RIGHT_ARROW, len);
        }
        else if (backTitle && (idx == sz() - 1))
        {
            return out.printText(backTitle, len);
        }
        else
        {   // Display selection menu item.
            return this->printItem(out, out.tops[root.level] + idx, len);
        }
    } // End printTo().
}; // End struct SpoolInfoMenu.

static result CopySpoolInfoToWorking(eventMask e, navNode& nav);
SpoolInfoMenu SpoolTableMenu("    SPOOL", NUMBER_SPOOLS, BACK_STRING,
                             SpoolInfoEditMenu, CopySpoolInfoToWorking, enterEvent, noStyle);

static result CopySpoolInfoToWorking(eventMask e, navNode& nav)
{
    if (nav.target == &SpoolTableMenu) // only if we are on SpoolTableMenu
    {
        // Copy the specifid spool data to our working structure.
        Spool *pSpool = gSpoolMgr.GetSpool(nav.sel);

        // The name field is special in that we want it to be exactly the same
        // length as the edit field size to be able to work with ArduinoMenu.
        // So we pad it here with spaces if needed.  Any trailing spaces will be
        // removed when we copy the working data back to the Spool instance.
        char *pName = pSpool->GetName();
        size_t nameLength = strlen(pName);
        strlcpy(gWorkingSpoolData.m_Name, pSpool->GetName(), Spool::MAX_NAME_SIZE + 1);
        while (nameLength < Spool::MAX_NAME_SIZE)
        {
            gWorkingSpoolData.m_Name[nameLength++] = ' ';
        }
        gWorkingSpoolData.m_Name[Spool::MAX_NAME_SIZE] = '\0';

        gWorkingSpoolData.m_Type = pSpool->GetType();
        gWorkingSpoolData.m_Density = pSpool->GetDensity();
        gWorkingSpoolData.m_Diameter = pSpool->GetDiameter();
        gWorkingSpoolData.m_SpoolWeight = pSpool->GetSpoolWeight();
        gWorkingSpoolData.m_Color = pSpool->GetColor();
        gWorkingSpoolData.m_SelectedOnEntry = (nav.sel == gSpoolMgr.GetSelectedSpoolIndex());
        gWorkingSpoolData.m_SelectedOnExit  = gWorkingSpoolData.m_SelectedOnEntry;

        gHsl.SetFromRgb565(gWorkingSpoolData.m_Color);
        gHue = (uint32_t)gHsl.GetHue();
        gSat = (uint32_t)gHsl.GetSat();
        gLum = (uint32_t)gHsl.GetLum();
    }
    return proceed;
} // End CopySpoolInfoToWorking().


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// FILAMENT MENU //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////// DENSITY TABLE MENU //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
class DensityTableMenuOverride : public menu
{
public:
    DensityTableMenuOverride(constMEM menuNodeShadow& shadow) : menu(shadow) { }
    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t p)
        override
    {
        char buf[SCREEN_CHAR_WIDTH + 1];
        if (idx < 0)
        {   // Display title menu item.
            snprintf(buf, sizeof(buf), " %s DENSITY",
                    gWorkingFilamentType);
        }
        else
        {   // Display selection menu item.
            snprintf(buf, sizeof(buf), " %s",
                    gWorkingFilamentType);
        }
        out.printRaw(buf, len);
        return idx;
    } // End printTo().
}; // End class DensityTableMenuOverride.

// Save the edited data record
static result SaveWorkingDensityInfo(eventMask e, navNode& nav)
{
    // Get selection of previous level.
    navNode& nn = nav.root->path[nav.root->level - 1];
    idx_t n = nn.sel;

    // Set the (possibly) new density.
    gFilament.SetDensity(static_cast<FilamentType>(n), gWorkingFilamentDensity);

    // Update the length factor used to calculate length given wieght.
    UpdateLengthFactor();

    return quit;
} // End SaveWorkingDensityInfo().

altMENU(DensityTableMenuOverride, FilamentInfoEditMenu, "   DENSITY", doNothing,
        noEvent, noStyle, (Menu::_menuData | Menu::_canNav)
    , altFIELD(decPlaces<2>::menuField, gWorkingFilamentDensity,
            "  ", " g/cc", Filament::MIN_DENSITY, Filament::MAX_DENSITY,
            0.20, 0.01, doNothing, noEvent, noStyle)
    , OP("", SkipItemUpDown, anyEvent)
    , OP("", SkipItemUpDown, anyEvent)
    , OP("", SkipItemUpDown, anyEvent)
    , OP("", SkipItemUpDown, anyEvent)
    , OP(BACK_STRING, SaveWorkingDensityInfo, enterEvent)
); // End FilamentInfoEditMenu.

struct FilamentInfoMenu : UserMenu
{
    using UserMenu::UserMenu;

    // Customizing the print of user menu item (len is the availabe space).
    Used printItem(menuOut& out, int idx, int len) override
    {
        // Print the filament type entry.
        if (idx < Filament::GetNumberFilaments())
        {
            char buf[SCREEN_CHAR_WIDTH + 1];
            char typeBuf[Filament::TYPE_STRING_MAX_SIZE];
            Spool *pSelectedSpool = gSpoolMgr.GetSelectedSpool();
            bool selected = (pSelectedSpool != NULL) &&
                            (idx == (int)pSelectedSpool->GetType());
            const char selectChar = (selected ? *RIGHT_ARROW : ' ');
            // Print the filament type preceeded by the selection character.
            snprintf(buf, sizeof(buf), "%c%s", selectChar,
                gFilament.GetTypeString(static_cast<FilamentType>(idx), typeBuf));
            return len ? out.printText(buf, len) : 0;
        }
        else
        {
            return out.printText(BACK_STRING, len);
        }
    } // End printItem().

    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t p)
        override
    {
        if (idx < 0)
        {   // Display title menu item.
            return UserMenu::printTo(root, sel, out, idx, len, p);
        }
        else if (root.navFocus != this)
        {
            return out.printRaw(" FILAMENT   " RIGHT_ARROW, len);
        }
        else if (backTitle && (idx == sz() - 1))
        {
            return out.printText(backTitle, len);
        }
        else
        {   // Display selection menu item.
            return this->printItem(out, out.tops[root.level] + idx, len);
        }
    } // End printTo().
}; // End struct FilamentInfoMenu.

static result CopyFilamentDensityInfoToWorking(eventMask e, navNode& nav);
FilamentInfoMenu FilamentDensityMenu("   DENSITY", Filament::GetNumberFilaments(),
                    BACK_STRING, FilamentInfoEditMenu, CopyFilamentDensityInfoToWorking,
                    enterEvent, noStyle);

static result CopyFilamentDensityInfoToWorking(eventMask e, navNode& nav)
{
    // Copy the data only if we are on the FilamentDensityMenu.
    if (nav.target == &FilamentDensityMenu)
    {
        FilamentType type = static_cast<FilamentType>(nav.sel);
        gFilament.GetTypeString(type, gWorkingFilamentType, sizeof(gWorkingFilamentType));
        gWorkingFilamentDensity = gFilament.GetDensity(type);
    }
    return proceed;
} // End CopyFilamentDensityInfoToWorking().



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// NETWORK MENU ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static const size_t NUM_NETWORK_ROWS = 5U;
struct NetworkInfoMenu : UserMenu
{
    using UserMenu::UserMenu;

    // Customizing the print of user menu item (len is the availabe space).
    Used printItem(menuOut& out, int idx, int len) override
    {
        int newLen = 0;
        uint16_t txtColor = 0;
        gTft.GetTextColor(txtColor);
        uint8_t textXSize = 0;
        uint8_t textYSize = 0;
        gTft.GetTextSize(textXSize, textYSize);
        if (len && (idx <= NUM_NETWORK_ROWS))
        {
            // We don't want any entry on this menu other than the EXIT entry to
            // be able to be selected.  In order to prevent selection, we bump the
            // the cursor down for each valid entry.  This leaves the cursor
            // on the EXIT entry.  Any time the user tries to move the cursor
            // above the EXIT entry, this menu is called, which moves it back
            // sown, thus preventing the cursor from ending up on any entry but
            // the EXIT entry.
            gEncStream.incEncoder();

            char buf[2 * SCREEN_CHAR_WIDTH + 1];
            IPAddress ip;
            String netString;
            int strength = 0;
            if (gNetwork.IsConnected())
            {
                switch(idx)
                {
                case 0: // Network Name: label.
                    gTft.setTextColor(DISABLED_TEXT_COLOR);
                    strlcpy(buf, "Network Name:", SCREEN_CHAR_WIDTH + 1);
                    newLen = out.printText(buf, len);
                    break;

                case 1: // Network Name value string.
                    netString = String(rNetworkServerName) + String(".local");
                    snprintf(buf, sizeof(buf), "%*s", SCREEN_CHAR_WIDTH * 2, netString.c_str());
                    gTft.setTextSize(textXSize / 2, textYSize);
                    newLen = out.printText(buf, 2 * SCREEN_CHAR_WIDTH);
                    break;

                case 2: // IP Address: label.
                    gTft.setTextColor(DISABLED_TEXT_COLOR);
                    strlcpy(buf, "IP Address:", SCREEN_CHAR_WIDTH + 1);
                    newLen = out.printText(buf, len);
                    break;

                case 3: // IP Address value.
                    ip = WiFi.localIP();
                    netString = String(ip[0]) + String(".") +
                                String(ip[1]) + String(".") +
                                String(ip[2]) + String(".") +
                                String(ip[3]);
                    snprintf(buf, sizeof(buf), "%*s", SCREEN_CHAR_WIDTH * 2, netString.c_str());
                    gTft.setTextSize(textXSize / 2, textYSize);
                    newLen = out.printText(buf, 2 * SCREEN_CHAR_WIDTH);
                    break;

                case 4: // Signal Strength: label.
                    gTft.setTextColor(DISABLED_TEXT_COLOR);
                    strength = min(max(2 * (WiFi.RSSI() + 100), 0), 100);
                    snprintf(buf, sizeof(buf), "Signal:  %d%%", strength);
                    newLen = out.printText(buf, len);
                    break;

                case 5: // Back label.  For some reason printTo() doesn't handle correctly.
                    newLen = out.printText(backTitle, len);
                    break;

                default: // Default - return 0.
                    break;
                }
            }
            else  // Network not connected, show access point info.
            {
                switch(idx)
                {
                case 0: // Network Name: label.
                    gTft.setTextColor(DISABLED_TEXT_COLOR);
                    strlcpy(buf, "AP Name:", SCREEN_CHAR_WIDTH + 1);
                    newLen = out.printText(buf, len);
                    break;

                case 1: // Network Name value string.
                    netString = String(gNetworkApName);
                    snprintf(buf, sizeof(buf), "%*s", SCREEN_CHAR_WIDTH * 2, netString.c_str());
                    gTft.setTextSize(textXSize / 2, textYSize);
                    newLen = out.printText(buf, 2 * SCREEN_CHAR_WIDTH);
                    break;

                case 2: // IP Address: label.
                    gTft.setTextColor(DISABLED_TEXT_COLOR);
                    strlcpy(buf, "AP IP Address:", SCREEN_CHAR_WIDTH + 1);
                    newLen = out.printText(buf, len);
                    break;

                case 3: // IP Address value.
                    ip = gNetworkApIpAddr;
                    netString = String(ip[0]) + String(".") +
                                String(ip[1]) + String(".") +
                                String(ip[2]) + String(".") +
                                String(ip[3]);
                    snprintf(buf, sizeof(buf), "%*s", SCREEN_CHAR_WIDTH * 2, netString.c_str());
                    gTft.setTextSize(textXSize / 2, textYSize);
                    newLen = out.printText(buf, 2 * SCREEN_CHAR_WIDTH);
                    break;

                case 4: // Signal Strength: label.
                    gTft.setTextColor(DISABLED_TEXT_COLOR);
                    newLen = out.printText(" ", len);
                    break;

                case 5: // Back label.  For some reason printTo() doesn't handle correctly.
                    newLen = out.printText(backTitle, len);
                    break;

                default: // Default - return 0.
                    break;
                }
            }
        }

        // Restore entry settings and return the (possibly) new length.
        gTft.setTextSize(textXSize, textYSize);
        gTft.setTextColor(txtColor);
        return newLen;
    } // End printItem().

    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t p)
        override
    {
        if (idx < 0)
        {
            // Display title menu item.
            return UserMenu::printTo(root, sel, out, idx, len, p);
        }
        else if (root.navFocus != this)
        {
            return out.printRaw(" NETWORK    " RIGHT_ARROW, len);
        }
        else
        {
            // Display selection menu item.
            return this->printItem(out, out.tops[root.level] + idx, len);
        }
    } // End printTo().
}; // End struct FilamentInfoMenu.

// Dummy empty menu used when a submenu is required to be specified, but
// no submenu is needed.  For example in the declaration of NetworkMenu.
MENU(NullMenu, "", doNothing, noEvent, noStyle
    , OP("", doNothing, noEvent)
    , EXIT("")
);

/////////////////////////////////////////////////////////////////////////////////
// NetworkMenu
//
// This menu is special for two reasons:
//    1. It is simply a status page that contains no data to be modified.
//    2. Some of the data on this page is too long to be displayed on a single
//       line in the text resolution used by the rest of the menu system.  For
//       this reason, it mixes text resolutions on several lines and thus
//       requires special print handling.
/////////////////////////////////////////////////////////////////////////////////
NetworkInfoMenu NetworkMenu("   NETWORK", NUM_NETWORK_ROWS,
                            BACK_STRING, NullMenu, noAction, noEvent, noStyle);


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////// SAVE/RESTORE MENU ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
////////////////////////// CONFIRM RESTART MENU /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
class RestartMenuOverride : public menu
{
public:
    RestartMenuOverride(constMEM menuNodeShadow& shadow):menu(shadow) {}
    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t p)
      override
    {
        if(idx < 0)
        {   // Display title menu item.
            menu::printTo(root, sel, out, idx, len, p);
        }
        else
        {   // Display selection menu item.
            out.printRaw((constText*)F(" RESTART    " RIGHT_ARROW), len);
        }
        return idx;
    } // End printTo().
}; // End class RestartMenuOverride.

static result DoRestart()
{
    RestartSystem();
    return quit;
} // End DoRestart().

static result DontDoRestart()
{
    return quit;
} // End DontDoRestart().

altMENU(RestartMenuOverride, RestartMenu, "CONFRM RESTRT", doNothing, noEvent, noStyle,
        (Menu::_menuData | Menu::_canNav)
    , OP(" Restart: YES", DoRestart,     enterEvent)
    , OP(" Restart: NO" , DontDoRestart, enterEvent)
); // End RestartMenu().


/////////////////////////////////////////////////////////////////////////////////
//////////////////////////// CONFIRM RESET MENU /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
class ResetMenuOverride : public menu
{
public:
    ResetMenuOverride(constMEM menuNodeShadow& shadow):menu(shadow) {}
    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t p)
      override
    {
        if(idx < 0)
        {   // Display title menu item.
            menu::printTo(root, sel, out, idx, len, p);
        }
        else
        {   // Display selection menu item.
            out.printRaw((constText*)F(" RESET DATA " RIGHT_ARROW), len);
        }
        return idx;
    } // End printTo().
}; // End class ResetMenuOverride.

static result DoReset()
{
    ResetNvs();
    return quit;
} // End DoReset().

static result DontDoReset()
{
    return quit;
} // End DontDoReset().

altMENU(ResetMenuOverride, ResetMenu, "CONFIRM RESET", doNothing, noEvent, noStyle,
        (Menu::_menuData | Menu::_canNav)
    , OP(" Reset: *YES*", DoReset,     enterEvent)
    , OP(" Reset:  NO" , DontDoReset, enterEvent)
); // End ResetMenu.


/////////////////////////////////////////////////////////////////////////////////
////////////////////////// CONFIRM RESET NET MENU ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
class ResetNetMenuOverride : public menu
{
public:
    ResetNetMenuOverride(constMEM menuNodeShadow& shadow):menu(shadow) {}
    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t p)
      override
    {
        if(idx < 0)
        {   // Display title menu item.
            menu::printTo(root, sel, out, idx, len, p);
        }
        else
        {   // Display selection menu item.
            out.printRaw((constText*)F(" RESET NET  " RIGHT_ARROW), len);
        }
        return idx;
    } // End printTo().
}; // End class ResetNetMenuOverride.

static result DoResetNet()
{
    gNetwork.ResetCredentials();
    delay(1000);
    RestartSystem();
    return quit;
} // End DoReset().

static result DontDoResetNet()
{
    return quit;
} // End DontDoReset().

altMENU(ResetNetMenuOverride, ResetNetMenu, "CONFIRM RESET", doNothing, noEvent, noStyle,
        (Menu::_menuData | Menu::_canNav)
    , OP(" Reset: *YES*", DoResetNet,     enterEvent)
    , OP(" Reset:  NO" ,  DontDoResetNet, enterEvent)
); // End ResetMenu.


/////////////////////////////////////////////////////////////////////////////////
////////////////////////// CONFIRM RESTORE MENU /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
class RestoreMenuOverride : public menu
{
public:
    RestoreMenuOverride(constMEM menuNodeShadow& shadow):menu(shadow) {}
    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t p)
      override
    {
        if(idx < 0)
        {   // Display title menu item.
            menu::printTo(root, sel, out, idx, len, p);
        }
        else
        {   // Display selection menu item.
            out.printRaw((constText*)F(" RESTORE    " RIGHT_ARROW), len);
        }
        return idx;
    } // End printTo().
}; // End class RestoreMenuOverride.

static result DoRestore()
{
    gTft.DisplayResult(RestoreFromNvs(), "DATA RESTORED", "RSTORE FAILED", BOX_RADIUS, 2000);
    return quit;
} // End DoRestore().

static result DontDoRestore()
{
    return quit;
} // End DontDoRestore().

altMENU(RestoreMenuOverride, RestoreMenu, "CONFIRM RESTR", doNothing, noEvent, noStyle,
        (Menu::_menuData | Menu::_canNav)
    , OP(" Restore: YES", DoRestore,     enterEvent)
    , OP(" Restore: NO" , DontDoRestore, enterEvent)
); // End RestoreMenu.


/////////////////////////////////////////////////////////////////////////////////
//////////////////////////// CONFIRM SAVE MENU //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
class SaveMenuOverride : public menu
{
public:
    SaveMenuOverride(constMEM menuNodeShadow& shadow):menu(shadow) {}
    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t p)
      override
    {
        if(idx < 0)
        {   // Display title menu item.
            menu::printTo(root, sel, out, idx, len, p);
        }
        else
        {   // Display selection menu item.
            out.printRaw((constText*)F(" SAVE       " RIGHT_ARROW), len);
        }
        return idx;
    } // End printTo().
}; // End class SaveMenuOverride.

static result DoSave()
{
    gTft.DisplayResult(SaveToNvs(), "DATA SAVED", "SAVE FAILED", BOX_RADIUS, 2000);
    return quit;
} // End DoSave().

static result DontDoSave()
{
    return quit;
} // End DontDoSave().

altMENU(SaveMenuOverride, SaveMenu, " CONFIRM SAVE", doNothing, noEvent, noStyle,
        (Menu::_menuData | Menu::_canNav)
    , OP(" Save: YES", DoSave,     enterEvent)
    , OP(" Save: NO" , DontDoSave, enterEvent)
); // End SaveMenu.


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////// SAVE/RESTORE MENU ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
class EnterSaveRestoreMenuOverride : public menu
{
public:
    EnterSaveRestoreMenuOverride(constMEM menuNodeShadow& shadow) : menu(shadow) { }
    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t p)
        override
    {
        if (idx < 0)
        {   // Display title menu item.
            menu::printTo(root, sel, out, idx, len, p);
        }
        else
        {   // Display selection menu item.
            out.printRaw((constText *)F(" SAVE/RSTR  " RIGHT_ARROW), len);
        }
        return idx;
    } // End printTo().
}; // End class EnterScaleMenuOverride.

altMENU(EnterSaveRestoreMenuOverride, SaveRestoreMenu, " SAVE/RESTORE",
        doNothing, enterEvent, noStyle,
        (Menu::_menuData | Menu::_canNav)
    , SUBMENU(SaveMenu)
    , SUBMENU(RestoreMenu)
    , SUBMENU(RestartMenu)
    , SUBMENU(ResetNetMenu)
    , SUBMENU(ResetMenu)
    , EXIT(BACK_STRING)
); // End SaveRestoreMenu.



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// MAIN MENU ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
MENU(MainMenu, "SETUP/OPTIONS", doNothing, noEvent, noStyle
    , SUBMENU(DisplayMenu)
    , SUBMENU(ScaleMenu)
    , OBJ(SpoolTableMenu)
    , OBJ(FilamentDensityMenu)
    , OBJ(NetworkMenu)
    , SUBMENU(SaveRestoreMenu)
    , EXIT(BACK_STRING)
); // End MainMenu.



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//////////////////////   M E N U S   E N D   H E R E   //////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////
// The output chain.
/////////////////////////////////////////////////////////////////////////////////
MENU_OUTPUTS(gOut, MAX_DEPTH
    , ADAGFX_OUT(gTft, gColorsTable, FONT_WIDTH * TEXT_SCALE,
    FONT_HEIGHT * TEXT_SCALE, {0, 0, GFX_WIDTH / (TEXT_SCALE * FONT_WIDTH) + 2,
    GFX_HEIGHT / (TEXT_SCALE * FONT_HEIGHT)})
  ,SERIAL_OUT(Serial)
);


/////////////////////////////////////////////////////////////////////////////////
// The root of our menu navigation.
/////////////////////////////////////////////////////////////////////////////////
NAVROOT(gNavRoot, MainMenu, MAX_DEPTH, gIn, gOut);


void InitScaleMenus()
{
    // Point a function to be used when menu is suspended.
    gNavRoot.idleTask = MenuIdle;

    // Allow the menu system to handle bursts of up to 5 input events.
    // Makes some menus more responsive.
    gNavRoot.inputBurst = 5;

    // Initialize our edit cursor icons.
    fieldBase::TunningCursor    = MODIFY_FINE_ICON;
    fieldBase::NonTunningCursor = MODIFY_COARSE_ICON;
}