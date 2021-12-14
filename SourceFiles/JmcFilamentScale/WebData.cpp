/////////////////////////////////////////////////////////////////////////////////
// WebData.cpp
//
// Contains functions to handle web requests from the web client.
//
// History:
// - jmcorbett 01-JUN-2021 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////
#include "JmcFilamentScale.h"   // For application related data.
#include "SimpleLock.h"         // For options locking SimpleLock class.
#include <ArduinoJson.h>        // For JSON handling.
#include "WebPages.h"           // For the data of the main web page display.
#include "WebData.h"            // Our own declarations.
#include "MainScreen.h"         // For MainScreen class.
#include "Spool.h"              // For MAX_NAME_SIZE.



/////////////////////////////////////////////////////////////////////////////////
// Web options locking related globals.
/////////////////////////////////////////////////////////////////////////////////
SimpleLock gWebLock;            // Lock to only allow setup changes one source at
                                // a time (web or local).
static uint32_t gWebWdTime = 0; // Last web message receipt time (ms).
static const uint32_t WEB_WD_TIMEOUT_MS = 10000;
                                // Timeout if this amount of milliseconds pass
                                // between web messages. (10 seconds)
// Constants defining web lock ownership values.
static const uint32_t LOCAL_OWNER = 1UL;
static const uint32_t WEB_OWNER   = 2UL;


/////////////////////////////////////////////////////////////////////////////////
// WebData::LockLocal()
//
// Attempt to acquire the local/network options lock for the local device.
//
// Returns:
//      Returns 'true' if the lock was acquired successfully.  Returns 'false'
//      otherwise.
/////////////////////////////////////////////////////////////////////////////////
bool WebData::LockLocal()
{
    return gWebLock.Lock(LOCAL_OWNER);
} // End LockLocal().


/////////////////////////////////////////////////////////////////////////////////
// WebData::IsLocalOwner()
//
// Returns true if owner of the lock is LOCAL_OWNER.
//
// Returns:
//      Returns 'true' if the lock is currently owned by LOCAL_OWNER.
/////////////////////////////////////////////////////////////////////////////////
bool WebData::IsLocalOwner()
{
    return gWebLock.Owner() == LOCAL_OWNER;
}


/////////////////////////////////////////////////////////////////////////////////
// WebData::Unlock()
//
// Releases the local/network options lock.
/////////////////////////////////////////////////////////////////////////////////
void WebData::Unlock()
{
    gWebLock.Unlock();
} // End Unlock().


/////////////////////////////////////////////////////////////////////////////////
// HandleWebTimeout()
//
// The web lock is used to insure that a web client and the local user cannot
// attempt to modify any options at the same time.  When the web client has an
// options page displayed, it takes ownership of the web lock.  This prevents the
// local user from entering the options menu.  If the web client goes away while
// owning the web lock, this could prevent the local user from ever being able
// to enter the options menu.  To prevent this, each time an update messsage is
// received from a web client, we remember the time at which it arrives.  The
// main loop calls this function every iteration to verify that the client
// connection is still alive.  If too much time has passed, then it is assumed
// that the connection is lost, so if the web client was the owner of the lock,
// it is unlocked.
/////////////////////////////////////////////////////////////////////////////////
void WebData::HandleWebTimeout()
{
    // If the web owned our options mutex and we haven't seen a message from
    // it for a while, then unlock the mutex.
    if ((gWebLock.Owner() == WEB_OWNER) &&
        ((millis() - gWebWdTime) > WEB_WD_TIMEOUT_MS))
    {
        gWebLock.Unlock();
    }
} // End HandleWebTimeout().


/////////////////////////////////////////////////////////////////////////////////
// Rgb565ToHexString()
//
// Converts an RGB565 color value to a hex string suitable for javascript use.
//
// Arguments:
//   color - this is the RGB565 color value to be converted.
//
// Returns:
//   Returns a pointer to the static buffer that contains the converted color
//   hexadecimal string.
//
// Note: This function returns a pointer to a static buffer, and so is not
// reentrant.
/////////////////////////////////////////////////////////////////////////////////
static char *Rgb565ToHexString(uint16_t color)
{
    static char buf[8];
    uint8_t r, g, b;
    r = ((color >> 11) << 3) | 7;
    g = ((color >> 5)  << 2) | 3;
    b = (color << 3) | 7;
    sprintf(buf, "#%02x%02x%02x", r, g, b);
    return buf;
} // End Rgb565ToHexString().


/////////////////////////////////////////////////////////////////////////////////
// HexStringToRgb565()
//
// Arguments:
//   pColor - This is a pointer to the hex color string to be converted.  Note
//            that it is assumed that this string is of the form of a javascript
//            color (i.e. #hhhhhh).
//
// Returns:
//   Returns the RGB565 value corresponding to the referenced color string.
/////////////////////////////////////////////////////////////////////////////////
static uint16_t HexStringToRgb565(const char *pColor)
{
    uint32_t val = strtol(pColor + 1, NULL, 16);
    return static_cast<uint16_t>(MYRGB565((val >> 16) & 0xff, (val >> 8) & 0xff, val & 0xff));
} // End HexStringToRgb565().


/////////////////////////////////////////////////////////////////////////////////
// HandleNotFound()
//
// Handles receipt of unknown network messages by displaying an error message
// on the network client.
/////////////////////////////////////////////////////////////////////////////////
static void HandleNotFound()
{
    // Setup the "not found" message along with the unknown URI and method.
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += gNetwork.uri();
    message += "\nMethod: ";
    message += (gNetwork.method() == HTTP_GET) ? "GET" : "POST";

    // Collect any arguments to the request.
    message += "\nArguments: ";
    message += gNetwork.args();
    message += "\n";
    for (uint8_t i = 0; i < gNetwork.args(); i++)
    {
        message += " " + gNetwork.argName(i) + ": " + gNetwork.arg(i) + "\n";
    }

    // Send diagnostic to the monitor.
    Serial.println("WEB PAGE NOT FOUND");
    Serial.println(message);

    // Send the data to the client.
    gNetwork.send(404, "text/plain", message);
} // End HandleNotFound().


/////////////////////////////////////////////////////////////////////////////////
// HandleLockOptions()
//
// Called when the client requests that the options be locked.  Checks if option
// settings are currently locked.  Replies to the request with a "LOCKED" status
// of true if options were not previously locked, and gives the client exclusive
// use of the setting/changing of the options.  Replies with a "LOCKED" status
// of false if options were already locked, and denies the client use of
// setting/changing of the options.
/////////////////////////////////////////////////////////////////////////////////
static void HandleLockOptions()
{
    String webPage;
    DynamicJsonDocument doc(100);
    bool locked = gWebLock.Lock(WEB_OWNER);
    doc["LOCKED"] = locked;
    serializeJson(doc, webPage);
    gNetwork.send(200, "text/html", webPage);
} // End HandleLockOptions().


/////////////////////////////////////////////////////////////////////////////////
// HandleUnlockOptions()
//
// Unlocks the setting/changing of options so that others may change them.
/////////////////////////////////////////////////////////////////////////////////
static void HandleUnlockOptions()
{
    gWebLock.Unlock();
    gNetwork.send(200, "text/html");
} // End HandleUnlockOptions().


/////////////////////////////////////////////////////////////////////////////////
// HandleRoot()
//
// Called when the client connects to the server.  Sends the root page to the
// client.
/////////////////////////////////////////////////////////////////////////////////
static void HandleRoot()
{
    Serial.println("--------------------------- GOT A HIT! ---------------------");
    gNetwork.send(200, "text/html", gRootPage);
} // End HandleRoot().


/////////////////////////////////////////////////////////////////////////////////
// HandleMainPageData()
//
// Called when the client requests a refresh of the main page data.  Updates all
// of the relevant fields of the main page based on current values.
/////////////////////////////////////////////////////////////////////////////////
static void HandleMainPageData()
{
    // Remember the current time for handling the web watchdog.
    gWebWdTime = millis();

    String webPage;
    DynamicJsonDocument doc(1024);

    // NET WEIGHT
    doc["WEIGHT"]           = gCurrentWeight;
    doc["WEIGHT_UNITS"]     = gLoadCell.GetUnitsString();
    doc["WEIGHT_PRECISION"] = GetWeightDecimalPlaces();

    // TEMPERATURE
    if (isnan(gCurrentTemperature))
    {
        doc["TEMPERATURE"]  = "-";
    }
    else
    {
        doc["TEMPERATURE"]  = gCurrentTemperature;
    }
    doc["TEMPERATURE_UNITS"]     = &gEnvSensor.GetTempScaleString()[1];
    doc["TEMPERATURE_PRECISION"] = gTemperatureUnits == eTempScaleF ? 0 : 1;

    // HUMIDITY
    if (isnan(gCurrentHumidity))
    {
        doc["HUMIDITY"] = "-";
    }
    else
    {
        doc["HUMIDITY"] = gCurrentHumidity;
    }

    // UP TIME
    doc["UPTIME"] = millis();

    // IP ADDRESS
    IPAddress ip = WiFi.localIP();
    String ipString = String(ip[0]) + String(".") +
                      String(ip[1]) + String(".") +
                      String(ip[2]) + String(".") +
                      String(ip[3]);
    doc["IP_ADDRESS"] = ipString;

    // WEB ID
    doc["WEB_ID"] = String(rNetworkServerName) + ".local";

    // SIGNAL STRENGTH
    doc["SIGNAL_STRENGTH"] = WiFi.RSSI();

    // If a spool is currently selected then send the corresponding fields.
    Spool *pSelectedSpool = gSpoolMgr.GetSelectedSpool();
    doc["SPOOL_SELECTED"] = (pSelectedSpool != NULL);
    if (pSelectedSpool != NULL)
    {
        // SPOOL RELATED VALUES
        char buf[Filament::TYPE_LSTRING_MAX_SIZE];
        FilamentType type = pSelectedSpool->GetType();
        doc["SPOOL_WEIGHT"]      = pSelectedSpool->GetSpoolWeight();
        doc["FILEMANT_TYPE"]     = gFilament.GetTypeLString(type, buf);
        doc["FILAMENT_DIAMETER"] = pSelectedSpool->GetDiameter();
        doc["SPOOL_NAME"]        = pSelectedSpool->GetName();
        doc["FILAMENT_DENSITY"]  = pSelectedSpool->GetDensity();

        // LENGTH
        doc["LENGTH"]           = gCurrentLength;
        doc["LENGTH_UNITS"]     = gLengthMgr.GetUnitsString();
        doc["LENGTH_PRECISION"] = gLengthMgr.GetPrecision();

        // FILAMENT COLOR
        doc["FILAMENT_COLOR"]   = Rgb565ToHexString(pSelectedSpool->GetColor());
    }

    serializeJson(doc, webPage);
    gNetwork.send(200, "text/html", webPage);
} // End HandleMainPageData().


/////////////////////////////////////////////////////////////////////////////////
// SendDisplayFormData()
//
// Called when the client opens the Display Options form.  Sends current values
// of all form data to the client.
/////////////////////////////////////////////////////////////////////////////////
static void SendDisplayFormData()
{
    DynamicJsonDocument doc(256);
    bool locked = gWebLock.Lock(WEB_OWNER);
    doc["LOCKED"] = locked;
    doc["WEIGHT_UNITS"]        = gLoadCell.GetUnits();
    doc["LENGTH_UNITS"]        = gLengthMgr.GetSelected();
    doc["TEMPERATURE_UNITS"]   = gEnvSensor.GetTempScale();
    doc["BRIGHTNESS"]          = gTft.GetBacklightPercent();
    doc["SCROLL_DELAY_S"]      = MainScreen::GetScrollDelayMs() / 1000;
    doc["MAX_SCROLL_DELAY_S"]  = MainScreen::MAX_SCROLL_DELAY_SEC;
    doc["SCROLL_DELAY_STEP_S"] = MainScreen::SCROLL_DELAY_STEP_SEC;

    String webPage;
    serializeJson(doc, webPage);
    gNetwork.send(200, "text/html", webPage);
} // End SendDisplayFormData().


/////////////////////////////////////////////////////////////////////////////////
// SaveDisplayFormData()
//
// Called when the client wants to save any changes made to the display options.
// Updates the display options as requested by the client.
/////////////////////////////////////////////////////////////////////////////////
static void SaveDisplayFormData()
{
    uint16_t response = 200;    // Assume OK response.

    StaticJsonDocument<300> JsonDoc;
    DeserializationError error = deserializeJson(JsonDoc, gNetwork.arg("plain"));
    if (error)
    {
        Serial.print("deserializeJson() failed with code ");
        Serial.println(error.c_str());
        response = 400; // BAD REQUEST response.
    }
    else
    {
        // Update our display parameters.
        gScaleUnits = static_cast<WeightUnits>(JsonDoc["weightUnitsData"]);
        SetLoadCellUnits(gScaleUnits);

        gLengthUnits = static_cast<LengthUnits>(JsonDoc["lengthUnitsData"]);
        UpdateLengthFactorEntry();

        gTemperatureUnits = static_cast<TempScale>(JsonDoc["tempUnitsData"]);
        gEnvSensor.SetTempScale(gTemperatureUnits);

        gBacklightPercent = static_cast<uint32_t>(JsonDoc["brightnessData"]);
        gTft.SetBacklightPercent(gBacklightPercent);

        MainScreen::SetScrollDelayMs(static_cast<uint32_t>(JsonDoc["scrollDelayData"]) * 1000);
    }

    // Send a response to the client.
    gNetwork.send(response, "text/html");

    // Let the system know that data has changed.
    gDataUpdated = true;

    // Unlock our mutex.
    gWebLock.Unlock();
} // End SaveDisplayFormData().


/////////////////////////////////////////////////////////////////////////////////
// SendScaleFormData()
//
// Called when the client opens the Scale Options form.  Sends current values
// of all form data to the client.
/////////////////////////////////////////////////////////////////////////////////
static void SendScaleFormData()
{
    String webPage;
    DynamicJsonDocument doc(256);

    bool locked = gWebLock.Lock(WEB_OWNER);
    doc["LOCKED"] = locked;

    doc["WEIGHT_PRECISION"] = GetWeightDecimalPlaces();
    doc["MAX_WEIGHT"]       = GetMaxScaleWeight();
    doc["WEIGHT_UNITS"]     = gLoadCell.GetUnitsString();
    doc["CALIBRATE_WEIGHT"] = gCalibrateWeight;
    doc["AVG_SAMPLES"]      = gScaleAveragingSamples;
    doc["AVG_SAMPLES_MAX"]  = AVG_SAMPLES_MAX;
    doc["LOAD_CELL_GAIN"]   = gScaleGain;

    serializeJson(doc, webPage);
    gNetwork.send(200, "text/html", webPage);
} // End SendScaleFormData().


/////////////////////////////////////////////////////////////////////////////////
// SaveScaleFormData()
//
// Called when the client wants to save any changes made to the scale options.
// Updates the display options as requested by the client.
/////////////////////////////////////////////////////////////////////////////////
static void SaveScaleFormData()
{
    uint16_t response = 200;    // Assume OK response.

    StaticJsonDocument<300> JsonDoc;
    DeserializationError error = deserializeJson(JsonDoc, gNetwork.arg("plain"));
    if (error)
    {
        Serial.print("deserializeJson() failed with code ");
        Serial.println(error.c_str());
        response = 400; // BAD REQUEST response.
    }
    else
    {
        // Set the selected spool's data per the request.
        gCalibrateWeight = static_cast<double>(JsonDoc["calWeightData"]);
        gScaleAveragingSamples = static_cast<uint32_t>(JsonDoc["avgSamples"]);
        gLoadCell.SetAverageInterval(gScaleAveragingSamples);
        uint8_t gain = static_cast<uint8_t>(JsonDoc["scaleGain"]);
        if (gain != gLoadCell.GetGain())
        {
            gScaleGain = gain;
            gLoadCell.SetGain(gain);
        }
    }

    // Send a response to the client.
    gNetwork.send(response, "text/html");

    // Let the system know that data has changed.
    gDataUpdated = true;

    // Unlock our mutex.
    gWebLock.Unlock();
} // End SaveScaleFormData().


/////////////////////////////////////////////////////////////////////////////////
// HandleDoTare()
//
// Called when the client requests a Tare operation.  Performs the Tare operation
// and returns the success/failre results to the client.
/////////////////////////////////////////////////////////////////////////////////
static void HandleDoTare()
{
    bool result = gLoadCell.Tare();
    String webPage;
    DynamicJsonDocument doc(100);
    doc["TARE_RESULT"] = result;
    serializeJson(doc, webPage);
    gNetwork.send(200, "text/html", webPage);
} // End HandleDoTare().


/////////////////////////////////////////////////////////////////////////////////
// HandleDoScaleCalibrate()
//
// Called when the client requests a scale calibration operation.  Performs the
// calibration and returns the success/failre results to the client.
/////////////////////////////////////////////////////////////////////////////////
static void HandleDoScaleCalibrate()
{
    uint16_t response = 200;    // Assume OK response.

    StaticJsonDocument<300> JsonDoc;
    DeserializationError error = deserializeJson(JsonDoc, gNetwork.arg("plain"));
    if (error)
    {
        Serial.print("deserializeJson() failed with code ");
        Serial.println(error.c_str());
        response = 400; // BAD REQUEST response.
        // Send a response to the client.
        gNetwork.send(response, "text/html");

    }
    else
    {
        // Set the selected spool's data per the request.
        gCalibrateWeight = static_cast<double>(JsonDoc["calWeightData"]);

        // Perform the calibration operation.
        bool success = gLoadCell.Calibrate(0, gCalibrateWeight);
        if (!success)
        {
            Serial.print("Calibration failed");
        }
        String webPage;
        DynamicJsonDocument doc(100);
        doc["CAL_RESULT"] = success;
        serializeJson(doc, webPage);
        gNetwork.send(response, "text/html", webPage);
    }
} // End HandleDoScaleCalibrate().


/////////////////////////////////////////////////////////////////////////////////
// SendSpoolFormData()
//
// Called when the client opens the Spool Options form.  Sends current values
// of all form data to the client.
/////////////////////////////////////////////////////////////////////////////////
static void SendSpoolFormData()
{
    String webPage;
    DynamicJsonDocument doc(1800);
    bool locked = gWebLock.Lock(WEB_OWNER);
    doc["LOCKED"] = locked;

    Spool *pSpool = gSpoolMgr.GetSelectedSpool();
    uint32_t startSpoolIndex = gSpoolMgr.GetSelectedSpoolIndex();
    bool spoolIsSelected = true;
    if (pSpool == NULL)
    {
        startSpoolIndex = 0;
        spoolIsSelected = false;
    }

    doc["WEIGHT_PRECISION"] = GetWeightDecimalPlaces();
    doc["MAX_WEIGHT"]       = GetMaxScaleWeight();
    doc["WEIGHT_UNITS"]     = gLoadCell.GetUnitsString();
    doc["START_SPOOL"]      = startSpoolIndex;
    doc["SPOOL_SELECTED"]   = spoolIsSelected;
    doc["MAX_NAME_LEN"]     = Spool::MAX_NAME_SIZE;
    doc["MAX_DENSITY"]      = Filament::MAX_DENSITY;
    doc["MIN_DENSITY"]      = Filament::MIN_DENSITY;

    doc.createNestedArray("FILAMENT_TYPES");
    doc.createNestedArray("SPOOL_NAMES");
    doc.createNestedArray("SPOOL_WEIGHTS");
    doc.createNestedArray("FILAMENT_DIAMETERS");
    doc.createNestedArray("DENSITY");
    doc.createNestedArray("COLORS");

    for (size_t i = 0; i < NUMBER_SPOOLS; i++)
    {
        Spool *pS = gSpoolMgr.GetSpool(i);
        doc["FILAMENT_TYPES"][i]     = pS->GetType();
        doc["SPOOL_DENSITY"][i]      = pS->GetDensity();
        doc["SPOOL_NAMES"][i]        = pS->GetName();
        doc["SPOOL_WEIGHTS"][i]      = pS->GetSpoolWeight();
        doc["FILAMENT_DIAMETERS"][i] = pS->GetDiameter();
        doc["COLORS"][i]             = Rgb565ToHexString(pS->GetColor());
    }

    // Send our density table data.
    for (size_t i = 0; i < static_cast<int>(eFtCount); i++)
    {
        doc["DENSITY"][i] =
            gFilament.GetDensity(static_cast<FilamentType>(i));
    }

    serializeJson(doc, webPage);
    gNetwork.send(200, "text/html", webPage);
} // End SendSpoolFormData().


/////////////////////////////////////////////////////////////////////////////////
// SaveSpoolFormData()
//
// Called when the client wants to save any changes made to the spool options.
// Updates the display options as requested by the client.
/////////////////////////////////////////////////////////////////////////////////
static void SaveSpoolFormData()
{
    uint16_t response = 200;    // Assume OK response.

    StaticJsonDocument<300> JsonDoc;
    DeserializationError error = deserializeJson(JsonDoc, gNetwork.arg("plain"));
    if (error)
    {
        Serial.print("deserializeJson() failed with code ");
        Serial.println(error.c_str());
        response = 400; // BAD REQUEST response.
    }
    else
    {
        // Point to the selected spool.
        uint32_t thisSpoolIndex = JsonDoc["spoolIndex"];
        Spool *pSpool = gSpoolMgr.GetSpool(thisSpoolIndex);

        uint32_t selectedSpoolIndex = gSpoolMgr.GetSelectedSpoolIndex();
        bool thisSpoolSelected = JsonDoc["spoolSelected"];
        if (thisSpoolSelected)
        {
            gSpoolMgr.SelectSpool(thisSpoolIndex);
        }
        else if (thisSpoolIndex == selectedSpoolIndex)
        {
            gSpoolMgr.DeselectSpool();
        }

        // Set the selected spool's data per the request.

        // Before we save the (possibly) new spool name, we need to remove any
        // trailing spaces from it.
        strlcpy(gWorkingSpoolData.m_Name, (const char *)JsonDoc["spoolIdData"],
                Spool::MAX_NAME_SIZE + 1);
        size_t nameLength = strlen(gWorkingSpoolData.m_Name);
        if (nameLength)
        {
            while (gWorkingSpoolData.m_Name[--nameLength] == ' ')
            {}
            gWorkingSpoolData.m_Name[++nameLength] = '\0';
        }
        pSpool->SetName(gWorkingSpoolData.m_Name);

        float spoolWeight = JsonDoc["spoolWeightData"];
        pSpool->SetSpoolWeight(spoolWeight);
        gLoadCell.SetOffset(spoolWeight);
        gWorkingSpoolData.m_SpoolWeight = spoolWeight;

        FilamentType filamentType =
            static_cast<FilamentType>(JsonDoc["filamentTypeData"]);
        pSpool->SetType(filamentType);
        gWorkingSpoolData.m_Type = filamentType;

        float density = JsonDoc["spoolDensity"];
        pSpool->SetDensity(density);
        gWorkingSpoolData.m_Density = density;

        float filamentDiameter = JsonDoc["filamentDiaData"];
        pSpool->SetDiameter(filamentDiameter);
        gWorkingSpoolData.m_Diameter = filamentDiameter;

        char buf[8];
        strlcpy(buf, (const char *)JsonDoc["colorData"], sizeof(buf));
        pSpool->SetColor(HexStringToRgb565(buf));
        gWorkingSpoolData.m_Color = pSpool->GetColor();

        // Update the spool offset just in case it changed.
        SaveSpoolOffset();

        // Update the length factor.
        UpdateLengthFactor();
    }

    // Let the system know that data has changed.
    gDataUpdated = true;

    // Send a response to the client.
    gNetwork.send(response, "text/html");
} // End SaveSpoolFormData().


/////////////////////////////////////////////////////////////////////////////////
// SendDensityFormData()
//
// Called when the client opens the Filament Options form.  Sends current values
// of all form data to the client.
/////////////////////////////////////////////////////////////////////////////////
static void SendDensityFormData()
{
    String webPage;
    DynamicJsonDocument doc(400);
    bool locked = gWebLock.Lock(WEB_OWNER);
    doc["LOCKED"] = locked;
    doc["MAX_DENSITY"] = Filament::MAX_DENSITY;
    doc["MIN_DENSITY"] = Filament::MIN_DENSITY;
    Spool *pSpool = gSpoolMgr.GetSelectedSpool();
    FilamentType type = eFtPla;
    if (pSpool != NULL)
    {
        type = pSpool->GetType();
    }
    doc["FILEMANT_TYPE"] = type;
    for (int i = 0; i < static_cast<int>(eFtCount); i++)
    {
        doc["DENSITY"][i] =
            gFilament.GetDensity(static_cast<FilamentType>(i));
    }

    serializeJson(doc, webPage);
    gNetwork.send(200, "text/html", webPage);
} // End SendDensityFormData().


/////////////////////////////////////////////////////////////////////////////////
// SaveDensityFormData()
//
// Called when the client wants to save any changes made to the filament options.
// Updates the display options as requested by the client.
/////////////////////////////////////////////////////////////////////////////////
static void SaveDensityFormData()
{
    uint16_t response = 200;    // Assume OK response.

    StaticJsonDocument<300> JsonDoc;
    DeserializationError error = deserializeJson(JsonDoc, gNetwork.arg("plain"));
    if (error)
    {
        Serial.print("deserializeJson() failed with code ");
        Serial.println(error.c_str());
        response = 400; // BAD REQUEST response.
    }
    else
    {
        // Get the selected filament type.
        FilamentType type =
            static_cast<FilamentType>(JsonDoc["filamentTypeData"]);
        float newDensity = JsonDoc["densityData"];

        Spool *pSpool = gSpoolMgr.GetSelectedSpool();
        if (pSpool && (pSpool->GetType() == type))
        {
            gWorkingFilamentDensity = newDensity;
            gFilament.SetDensity(type, newDensity);

            // Update the length factor.
            UpdateLengthFactor();
        }
        else
        {
            // Set the new density.
            gFilament.SetDensity(type, newDensity);
        }
    }

    // Let the system know that data has changed.
    gDataUpdated = true;

    // Send a successful response to the client.
    gNetwork.send(response, "text/html");
} // End SaveDensityFormData().




/////////////////////////////////////////////////////////////////////////////////
// HandleDoSave()
//
// Called when the client requests a save operation.  Performs
// the save and returns the success/failre results to the client.
/////////////////////////////////////////////////////////////////////////////////
static void HandleDoSave()
{
    bool result = SaveToNvs();
    String webPage;
    DynamicJsonDocument doc(100);
    doc["SAVE_RESULT"] = result;
    serializeJson(doc, webPage);
    gNetwork.send(200, "text/html", webPage);

    if (result)
    {
        // Unlock our mutex.
        gWebLock.Unlock();
    }
} // End HandleDoSave().


/////////////////////////////////////////////////////////////////////////////////
// HandleDoRestore()
//
// Called when the client requests a restore operation.  Performs
// the  restore and returns the success/failre results to the client.
/////////////////////////////////////////////////////////////////////////////////
static void HandleDoRestore()
{
    bool result = RestoreFromNvs();
    String webPage;
    DynamicJsonDocument doc(100);
    doc["RESTORE_RESULT"] = result;
    serializeJson(doc, webPage);
    gNetwork.send(200, "text/html", webPage);

    if (result)
    {
        // Unlock our mutex.
        gWebLock.Unlock();
        // Let the system know that data has changed.
        gDataUpdated = true;
    }
} // End HandleDoRestore().


/////////////////////////////////////////////////////////////////////////////////
// HandleDoRestart()
//
// Called when the client requests a restart operation.  Returns a successful
// response to the client and initiates the restart.
/////////////////////////////////////////////////////////////////////////////////
static void HandleDoRestart()
{
    gNetwork.send(200, "text/html");
    delay(500);
    ESP.restart();
} // End HandleDoRestart().


/////////////////////////////////////////////////////////////////////////////////
// HandleDoReset()
//
// Called when the client requests a reset operation.  Returns a successful
// response to the client and initiates the reset.
/////////////////////////////////////////////////////////////////////////////////
static void HandleDoReset()
{
    gNetwork.send(200, "text/html");
    delay(500);
    ResetNvs();
} // End HandleDoReset().


/////////////////////////////////////////////////////////////////////////////////
// HandleDoResetNet()
//
// Called when the client requests a reset net operation.  Returns a successful
// response to the client and initiates the reset.
/////////////////////////////////////////////////////////////////////////////////
static void HandleDoResetNet()
{
    gNetwork.send(200, "text/html");
    delay(1000);
    gNetwork.ResetCredentials();
    delay(1000);
    RestartSystem();
} // End HandleDoResetNet().


/////////////////////////////////////////////////////////////////////////////////
// InitNetworkHandlers()
//
// Called at power-up to initialize the handling of client requests.  Sets up
// a handler for each of the URLs that are expected to be received from the
// network client.
/////////////////////////////////////////////////////////////////////////////////
void WebData::InitNetworkHandlers()
{
    // MAIN PAGE
    gNetwork.on("/", HandleRoot);
    gNetwork.on("/getMainPageData", HandleMainPageData);

    // DISPLAY OPTIONS FORM
    gNetwork.on("/getDisplayFormData", SendDisplayFormData);
    gNetwork.on("/updateDisplayData", SaveDisplayFormData);

    // SCALE OPTIONS FORM
    gNetwork.on("/getScaleFormData", SendScaleFormData);
    gNetwork.on("/updateScaleData", SaveScaleFormData);
    gNetwork.on("/doTare",  HandleDoTare);
    gNetwork.on("/doScaleCalibrate", HandleDoScaleCalibrate);

    // SPOOL OPTIONW FORM
    gNetwork.on("/getSpoolFormData", SendSpoolFormData);
    gNetwork.on("/updateSpoolData", SaveSpoolFormData);

    // FILAMENT (DENSITY) OPTIONS FORM
    gNetwork.on("/getDensityFormData", SendDensityFormData);
    gNetwork.on("/updateDensityData", SaveDensityFormData);

    // SAVE / RESTORE / RESET OPTIONS FORM
    gNetwork.on("/doSave", HandleDoSave);
    gNetwork.on("/doRestore", HandleDoRestore);
    gNetwork.on("/doRestart", HandleDoRestart);
    gNetwork.on("/doReset", HandleDoReset);
    gNetwork.on("/doResetNet", HandleDoResetNet);

    // UTILITIES
    gNetwork.on("/lockOptions", HandleLockOptions);
    gNetwork.on("/unlockOptions", HandleUnlockOptions);
    gNetwork.onNotFound(HandleNotFound);
} // End InitNetworkHandlers().

