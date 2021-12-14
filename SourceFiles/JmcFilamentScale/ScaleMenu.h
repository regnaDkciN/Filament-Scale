/////////////////////////////////////////////////////////////////////////////////
// ScaleMenu.h
//
// Contains the data relating to local menu handling for the filament scale.
//
// History:
// - jmcorbett 10-JUN-2021 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////
#if !defined SCALEMENU_H
#define SCALEMENU_H

#define LEFT_ARROW  "\x11"              // Graphical left arrow.
#define RIGHT_ARROW "\x10"              // Graphical right arrow.
#define BACK_STRING LEFT_ARROW "Back"   // String for menu back display.
#define MODIFY_COARSE_ICON "\x13"       // Icon to denote coarse editing in process.
#define MODIFY_FINE_ICON   "!"          // Icon to denote fine editint in process.

extern navRoot gNavRoot;                // Forward reference for our navRoot.


uint16_t GetBgColor();                  // Returns the RGB565 background color value.
void     InitScaleMenus();              // Initialize our menus.


#endif // SCALEMENU_H
