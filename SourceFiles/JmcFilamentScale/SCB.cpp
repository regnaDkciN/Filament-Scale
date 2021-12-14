/////////////////////////////////////////////////////////////////////////////////
// SCB.cpp
//
// Contains the methods for the SCB (Screen Control Block) structure.
//
// History:
// - jmcorbett 07-JUN-2021 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////
#include "JmcFilamentScale.h"       // For main screen related data.
#include "SCB.h"                    // For SCB definition.
#include "HslColor.h"               // For RGB565 contrast color method.


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//////////////////    STRING GENERATING METHODS   ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
bool SCB::NetWeightStrings(char *pBuf, size_t bufSize, int what)
{
    switch (what)
    {
    case eHeader:
        if (gSpoolMgr.GetSelectedSpool() == NULL)
        {
            snprintf(pBuf, bufSize, "Weight (%s)", gLoadCell.GetUnitsString() + 1);
        }
        else
        {
            snprintf(pBuf, bufSize, "Net Weight (%s)", gLoadCell.GetUnitsString() + 1);
        }
        break;

    case eMain:
        if (gLoadCell.IsCalibrated())
        {
            m_MainFgColor = MAIN_PAGE_FG_COLOR;
            AddCommas(gCurrentWeight, GetWeightDecimalPlaces(), pBuf, bufSize);
        }
        else
        {
            m_MainFgColor = ST7735_RED;
            strlcpy(pBuf, "--CALIBRATE--", bufSize);
        }
        break;

    default:
        break;
    }
    return true;
} // End NetWeightStrings().




bool SCB::LengthStrings(char *pBuf, size_t bufSize, int what)
{
    bool status = false;
    Spool *pSelectedSpool = gSpoolMgr.GetSelectedSpool();
    if (pSelectedSpool != NULL)
    {
        status = true;
        switch (what)
        {
        case eHeader:
            snprintf(pBuf, bufSize, "Length (%s)", gLengthMgr.GetUnitsString());
            break;

        case eMain:
            if ((gLoadCell.IsCalibrated()) && (gSpoolMgr.GetSelectedSpool() != NULL))
            {
                m_MainFgColor = MAIN_PAGE_FG_COLOR;
                AddCommas(gCurrentLength, gLengthMgr.GetPrecision(), pBuf, bufSize);
            }
            else
            {
                m_MainFgColor = ST7735_RED;
                strlcpy(pBuf, "***", bufSize);
            }
            break;

        default:
            break;
        }
    }
    return status;
} // End LengthStrings().


bool SCB::GrossWeightStrings(char *pBuf, size_t bufSize, int what)
{
    bool status = false;
    Spool *pSelectedSpool = gSpoolMgr.GetSelectedSpool();
    if (pSelectedSpool != NULL)
    {
        status = true;
        switch (what)
        {
        case eHeader:
            snprintf(pBuf, bufSize, "Gross Weight (%s)", gLoadCell.GetUnitsString() + 1);
            break;

        case eMain:
            if (gLoadCell.IsCalibrated())
            {
                m_MainFgColor = MAIN_PAGE_FG_COLOR;
                AddCommas(gCurrentWeight + pSelectedSpool->GetSpoolWeight(),
                         GetWeightDecimalPlaces(), pBuf, bufSize);
            }
            else
            {
                m_MainFgColor = ST7735_RED;
                strlcpy(pBuf, "--CALIBRATE--", bufSize);
            }
            break;

        default:
            break;
        }
    }
    return status;
} // End GrossWeightStrings().


bool SCB::TemperatureStrings(char *pBuf, size_t bufSize, int what)
{
    switch (what)
    {
    case eHeader:
        snprintf(pBuf, bufSize, "Temp. (%s)", gEnvSensor.GetTempScaleString());
        break;

    case eMain:
        if (!isnan(gCurrentTemperature))
        {
            m_MainFgColor = MAIN_PAGE_FG_COLOR;
            snprintf(pBuf, bufSize, "%.*f",
                (gTemperatureUnits == eTempScaleF ? 0 : 1), gCurrentTemperature);
        }
        else
        {
            m_MainFgColor = ST7735_RED;
            strlcpy(pBuf, "-", bufSize);
        }
        break;

    default:
        break;
    }
    return true;
} // End TemperatureStrings().


bool SCB::HumidityStrings(char *pBuf, size_t bufSize, int what)
{
    switch (what)
    {
    case eHeader:
        snprintf(pBuf, bufSize, "Hum. (%%)");
        break;

    case eMain:
        if (!isnan(gCurrentHumidity))
        {
            m_MainFgColor = MAIN_PAGE_FG_COLOR;
            snprintf(pBuf, bufSize, "%.0f", gCurrentHumidity);
        }
        else
        {
            m_MainFgColor = ST7735_RED;
            strlcpy(pBuf, "-", bufSize);
        }
        break;

    default:
        break;
    }
    return true;
} // End HumidityStrings().


bool SCB::SpoolIdStrings(char *pBuf, size_t bufSize, int what)
{
    Spool *pSelectedSpool = gSpoolMgr.GetSelectedSpool();
    switch (what)
    {
    case eHeader:
        strlcpy(pBuf, "Spool ID", bufSize);
        break;

    case eMain:
        if (pSelectedSpool != NULL)
        {
            m_MainFgColor = MAIN_PAGE_FG_COLOR;
            strlcpy(pBuf, pSelectedSpool->GetName(), bufSize);
        }
        else
        {
            m_MainFgColor = ST7735_RED;
            strlcpy(pBuf, "-- NONE --", bufSize);
        }
        break;

    default:
        break;
    }
    return true;
} // End SpoolIdStrings().


bool SCB::SpoolWeightStrings(char *pBuf, size_t bufSize, int what)
{
    bool status = false;
    Spool *pSelectedSpool = gSpoolMgr.GetSelectedSpool();
    if (pSelectedSpool != NULL)
    {
        status = true;
        switch (what)
        {
        case eHeader:
            snprintf(pBuf, bufSize, "Spool Weight (%s)", gLoadCell.GetUnitsString() + 1);
            break;

        case eMain:
            AddCommas(pSelectedSpool->GetSpoolWeight(),
                      GetWeightDecimalPlaces(), pBuf, bufSize);
            break;

        default:
            break;
        }
    }
    return status;
} // End SpoolWeightStrings().


bool SCB::FilamentTypeStrings(char *pBuf, size_t bufSize, int what)
{
    bool status = false;
    Spool *pSelectedSpool = gSpoolMgr.GetSelectedSpool();
    if (pSelectedSpool != NULL)
    {
        status = true;
        switch (what)
        {
        case eHeader:
            strlcpy(pBuf, "Fil Type", bufSize);
            break;

        case eMain:
            gFilament.GetTypeLString(pSelectedSpool->GetType(), pBuf, bufSize);
            break;

        default:
            break;
        }
    }
    return status;
} // End FilamentTypeStrings().


bool SCB::FilamentDiaStrings(char *pBuf, size_t bufSize, int what)
{
    bool status = false;
    Spool *pSelectedSpool = gSpoolMgr.GetSelectedSpool();
    if (pSelectedSpool != NULL)
    {
        status = true;
        switch (what)
        {
        case eHeader:
            strlcpy(pBuf, "Dia (mm)", bufSize);
            break;

        case eMain:
            snprintf(pBuf, bufSize, "%1.2f", pSelectedSpool->GetDiameter());
            break;

        default:
            break;
        }
    }
    return status;
} // End FilamentDiaStrings().


bool SCB::FilamentDensityStrings(char *pBuf, size_t bufSize, int what)
{
    bool status = false;
    Spool *pSelectedSpool = gSpoolMgr.GetSelectedSpool();
    if (pSelectedSpool != NULL)
    {
        status = true;
        switch (what)
        {
        case eHeader:
            strlcpy(pBuf, "Dens (g/cc)", bufSize);
            break;

        case eMain:
            snprintf(pBuf, bufSize, "%1.2f", pSelectedSpool->GetDensity());
            break;

        default:
            break;
        }
    }
    return status;
} // End FilamentDensityStrings().


bool SCB::FilamentColorStrings(char *pBuf, size_t bufSize, int what)
{
    bool status = false;
    Spool *pSelectedSpool = gSpoolMgr.GetSelectedSpool();
    if (pSelectedSpool != NULL)
    {
        m_BgColor = pSelectedSpool->GetColor();
        m_HeaderFgColor = HslColor::Contrast(m_BgColor);
        m_OutlineFgColor = m_HeaderFgColor;

        status = true;
        switch (what)
        {
        case eHeader:
            strlcpy(pBuf, "Color", bufSize);
            break;

        case eMain:
            strlcpy(pBuf, " ", bufSize);
            break;

        default:
            break;
        }
    }
    return status;
} // End FilamentColorStrings().


bool SCB::UptimeStrings(char *pBuf, size_t bufSize, int what)
{
    uint32_t time;
    uint32_t seconds;
    uint32_t minutes;
    uint32_t hours;

    switch (what)
    {
    case eHeader:
        strlcpy(pBuf, "Up Time", bufSize);
        break;

    case eMain:
        time    = millis();
        seconds = time / 1000;
        minutes = (seconds / 60) % 60;
        hours   = seconds / 3600;
        seconds = seconds % 60;
        snprintf(pBuf, bufSize, "%01u:%02u:%02u", hours, minutes, seconds);
        break;

    default:
        break;
    }
    return true;
} // End UptimeStrings().


bool SCB::NetworkNameStrings(char *pBuf, size_t bufSize, int what)
{
    switch (what)
    {
    case eHeader:
        strlcpy(pBuf, "Net Name", bufSize);
        break;

    case eMain:
        if (gNetwork.IsConnected())
        {
            m_MainFgColor = MAIN_PAGE_FG_COLOR;
            snprintf(pBuf, bufSize, "%s.local", rNetworkServerName);
        }
        else
        {
            m_MainFgColor = ST7735_RED;
            strlcpy(pBuf, "OFFLINE", bufSize);
        }
        break;

    default:
        break;
    }
    return true;
} // End NetworkNameStrings().



bool SCB::IpAddrStrings(char *pBuf, size_t bufSize, int what)
{
    bool status = gNetwork.IsConnected();
    if (status)
    {
        IPAddress ip = WiFi.localIP();

        switch (what)
        {
        case eHeader:
            strlcpy(pBuf, "IP Addr", bufSize);
            break;

        case eMain:
            m_MainFgColor = MAIN_PAGE_FG_COLOR;
            snprintf(pBuf, bufSize, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
            break;

        default:
            break;
        }
    }
    return status;
} // End IpAddrStrings().


bool SCB::SignalStrengthStrings(char *pBuf, size_t bufSize, int what)
{
    bool status = gNetwork.IsConnected();
    if (status)
    {
        int strength = WiFi.RSSI();
        #define DARK_GREEN 0x7be0
        #define FULL_BAR  '\xDA'
        #define EMPTY_BAR '\xB0'
        char bars[bufSize];

        if (strength >= -79)
        {
            m_MainFgColor = ST7735_GREEN;
            snprintf(bars, bufSize, "%c%c%c%c", FULL_BAR, FULL_BAR, FULL_BAR, FULL_BAR);
        }
        else if (strength >= -89)
        {
            m_MainFgColor = DARK_GREEN;
            snprintf(bars, bufSize, "%c%c%c%c", FULL_BAR, FULL_BAR, FULL_BAR, EMPTY_BAR);
        }
        else if (strength >= -99)
        {
            m_MainFgColor = ST7735_YELLOW;
            snprintf(bars, bufSize, "%c%c%c%c", FULL_BAR, FULL_BAR, EMPTY_BAR, EMPTY_BAR);
        }
        else if (strength >= -109)
        {
            m_MainFgColor = ST7735_ORANGE;
            snprintf(bars, bufSize, "%c%c%c%c", FULL_BAR, EMPTY_BAR, EMPTY_BAR, EMPTY_BAR);
        }
        else
        {
            m_MainFgColor = ST7735_RED;
            snprintf(bars, bufSize, "%c%c%c%c", EMPTY_BAR, EMPTY_BAR, EMPTY_BAR, EMPTY_BAR);
        }

        switch (what)
        {
        case eHeader:
            strlcpy(pBuf, "WiFi Signal (dBm)", bufSize);
            break;

        case eMain:
            snprintf(pBuf, bufSize, "%s   %d", bars, strength);
            break;

        default:
            break;
        }
    }
    return status;
} // End SignalStrengthStrings().


bool SCB::ApNetworkNameStrings(char *pBuf, size_t bufSize, int what)
{
    bool status = !gNetwork.IsConnected();
    if (status)
    {
        switch (what)
        {
        case eHeader:
            strlcpy(pBuf, "Access Point Net Name", bufSize);
            break;

        case eMain:
            m_MainFgColor = MAIN_PAGE_FG_COLOR;
            snprintf(pBuf, bufSize, "%s", gNetworkApName);
            break;

        default:
            break;
        }
    }
    return status;
} // End ApNetworkNameStrings().



bool SCB::ApIpAddrStrings(char *pBuf, size_t bufSize, int what)
{
    bool status = !gNetwork.IsConnected();
    if (status)
    {
        IPAddress ip = gNetworkApIpAddr;
        switch (what)
        {
        case eHeader:
            strlcpy(pBuf, "Access Point IP Addr", bufSize);
            break;

        case eMain:
            m_MainFgColor = MAIN_PAGE_FG_COLOR;
            snprintf(pBuf, bufSize, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
            break;

        default:
            break;
        }
    }
    return status;
} // End ApIpAddrStrings().



/////////////////////////////////////////////////////////////////////////////////
// DisplayABox()
//
// Displays a portion of a box as specified by its only argument.
//
// Arguments:
//      what - Specifies what portion of the box, if any, is to be displayed.
/////////////////////////////////////////////////////////////////////////////////
void SCB::DisplayABox(WhatToDisplay what)
{
    char buf[MAX_STRING_LENGTH * 2 + 1];

    CallDisplayFunction(buf, sizeof(buf), what);

    // If the box is requested, or if the box background color has changed,
    // then display the box.
    if ((what == eBox) || (m_LastBgColor != m_BgColor))
    {
        gTft.DisplayBox(m_Line, m_Side,
            m_OutlineFgColor, m_BgColor, BOX_RADIUS);
        m_LastBgColor = m_BgColor;
    }
    if (what == eHeader)
    {
        gTft.DisplayBoxHeader(buf, m_Line, m_Side,
            m_HeaderFgColor, m_BgColor, BOX_RADIUS);
    }
    else if (what == eMain)
    {
        gTft.DisplayBoxMain(buf,  m_Line, m_Side,
                            m_MainFgColor, m_BgColor, BOX_RADIUS);
    }
} // End DisplayABox().
