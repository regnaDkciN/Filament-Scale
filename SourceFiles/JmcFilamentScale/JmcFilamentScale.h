/////////////////////////////////////////////////////////////////////////////////
// JmcFilamentScale.h
//
// This file contains constants and extern declarations for data that is shared
// by JmcFilamentScale.ino (the filament scale's main code file]).
//
// History:
// - jmcorbett 01-JUN-2021 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////
#if !defined JMCFILAMENTSCALE_H
#define JMCFILAMENTSCALE_H

#include "EnvSensor.h"          // For the EnvSensor class (temp and humidity).
#include "LoadCell.h"           // For the LoadCell sensor class.
#include "Filament.h"           // For filament density table.
#include "SpoolManager.h"       // For spool management class.
#include "LengthManager.h"      // For length management class.
#include "Display.h"            // For Display class.
#include "Network.h"            // For Network/server (wifi) class.
#include "ESP32EncoderStream.h" // For encoder w/pushbutton.


// Convert red, green, and blue 8-bit values into a single 16-bit rgb value used
// by the TFT display.
#define MYRGB565(r, g, b) (((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3)))


/////////////////////////////////////////////////////////////////////////////////
// SpoolData
//
// This structure contains data that assists in sharing and controlling filament
// spools.
/////////////////////////////////////////////////////////////////////////////////
struct SpoolData
{
    char         m_Name[Spool::MAX_NAME_SIZE + 1]; // Spool name string.
    FilamentType m_Type;                           // Type of filament.
    float        m_Density;                        // Density of filament.
    float        m_Diameter;                       // Filament diameter.
    float        m_SpoolWeight;                    // Empty spool weight.
    uint16_t     m_Color;                          // Spool filament color.
    bool         m_SelectedOnEntry;                // Was selected on entry.
    bool         m_SelectedOnExit;                 // Is selected on exit.
};

//namespace JmcFilamentScale
//{
    const size_t NUMBER_SPOOLS = 15;
    const size_t MAX_STRING_LENGTH = 30;

    const uint16_t DARK_BLUE = 8; //RGB565(0, 0, 64);
    const uint16_t MAIN_PAGE_BG_COLOR = (uint16_t)DARK_BLUE;
    const uint16_t MAIN_PAGE_FG_COLOR = (uint16_t)ST7735_WHITE;
    const int16_t  BOX_RADIUS  = 8;      // Radius of displayed boxes.
    const uint32_t WEIGHT_UPDATE_PERIOD_MS = 200UL; // Weight poll Update period.
    const uint32_t AVG_SAMPLES_DEFAULT_MS  = 2500;  // Default averaging time.
    const uint32_t AVG_SAMPLES_MAX_MS      = 5000;  // Max averaging time.
    const uint32_t AVG_SAMPLES_MAX         =
                   (AVG_SAMPLES_MAX_MS + WEIGHT_UPDATE_PERIOD_MS / 2) / WEIGHT_UPDATE_PERIOD_MS;
    const uint32_t AVG_SAMPLES_MIN         = 1;
    const uint32_t AVG_SAMPLES_BIG_STEP    = AVG_SAMPLES_MAX / 5;
    const uint32_t AVG_SAMPLES_SMALL_STEP  = AVG_SAMPLES_BIG_STEP <= 1 ? 0.0 : 1;

    const uint32_t TEXT_SCALE  = 2U;        // Scale of text to use for menus.
    const uint32_t GFX_WIDTH   = 160U;      // Display width in pixels.
    const uint32_t GFX_HEIGHT  = 128U;      // Display height in pixels.
    const uint32_t FONT_WIDTH  = 6U;        // Font width in pixels.
    const uint32_t FONT_HEIGHT = 9U;        // Font height in pixels.
    const uint16_t SCREEN_CHAR_WIDTH = GFX_WIDTH / FONT_WIDTH / TEXT_SCALE;

    // Access Point related globals and constants.  Note that gNetworkApIpAddr is set
    // to the default address used by the WiFi manager.  No attempt is made to change
    // the default.  It is present here only to allow the display to show it.
    static const char *gNetworkApName = "ScaleWifiSetup";
    static const IPAddress gNetworkApIpAddr(192, 168, 4, 1);

    extern const char * &rNetworkServerName;;
    extern SpoolManager<NUMBER_SPOOLS> gSpoolMgr;
    extern LoadCell gLoadCell;
    extern LengthManager gLengthMgr;
    extern EnvSensor gEnvSensor;
    extern TempScale gTemperatureUnits;
    extern Filament gFilament;
    extern Network gNetwork;
    extern Display gTft;
    extern ESP32EncoderStream gEncStream;

    extern float gCurrentWeight;
    extern float gCurrentLength;
    extern float gCurrentTemperature;
    extern float gCurrentHumidity;
    extern WeightUnits gScaleUnits;
    extern LengthUnits gLengthUnits;
    extern uint32_t gBacklightPercent;
    extern double gCalibrateWeight;
    extern uint32_t gScaleAveragingSamples;
    extern uint8_t gScaleGain;
    extern SpoolData gWorkingSpoolData; // Spool data currently being worked om.
    extern float  gWorkingFilamentDensity;
    extern bool gRunningMenu;
    extern bool gDataUpdated;
    extern double gMinWeight;
    extern double gMaxWeight;
    extern double gBigWeightStep;
    extern double gSmallWeightStep;
    extern char   gWorkingFilamentType[Filament::TYPE_STRING_MAX_SIZE];



    char *AddCommas(double val, int prec, char *pBuf, int bufSize);
    int GetWeightDecimalPlaces();
    void SetLoadCellUnits(WeightUnits units);
    double GetMaxScaleWeight();
    void SaveSpoolOffset();
    void UpdateLengthFactor();
    void UpdateLengthFactorEntry();
    bool SaveToNvs();
    bool RestoreFromNvs();
    void ResetNvs();
    void UpdateLengthFactorEntry();
    void RestartSystem();
//}




#endif // JMCFILAMENTSCALE_H
