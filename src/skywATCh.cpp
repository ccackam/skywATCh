// <editor-fold> Legal ---------------------------------------------------------
/*
skywATCh ACT program for X-Plane 11.

Copyright (c) 2020 Kameron Eves

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
// </editor-fold> --------------------------------------------------------------



// <editor-fold> Headers -------------------------------------------------------
// Global Libraries
#include <string>
#include <stdio.h>

// Operating System specific
#if IBM
	#include <windows.h>
#endif
#if LIN
	#include <GL/gl.h>
#elif __GNUC__
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

// SDK Libraries
#include "XPLMMenus.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"

// SDK Version
#ifndef XPLM300
	#error This is made to be compiled against the XPLM300 SDK
#endif
// </editor-fold> --------------------------------------------------------------



// <editor-fold> Gloabl Variables ----------------------------------------------
// Info
std::string	 	plugin_name = "skywATCh";

// Menu
int 					menu_index;
XPLMMenuID 		menu_container;
void 					menu_handler(void *, void *);

// Widgets
static XPWidgetID 			flight_plan_window = NULL;
static XPWidgetID 			settings_window = NULL;
int 										window_callback(XPWidgetMessage inMessage,XPWidgetID inWidget,intptr_t inParam1,intptr_t inParam2);

// </editor-fold> --------------------------------------------------------------



// <editor-fold> SDK Required Functions ----------------------------------------
PLUGIN_API int XPluginStart(char * outName, char * outSig, char *	outDesc)
{
	// Plugin info for SDK
	strcpy(outName, plugin_name.c_str());
	strcpy(outSig, ("ccackam."+plugin_name).c_str());
	strcpy(outDesc, "An improved ATC Program.");

	// Create Menu
	/*
	- skywATCh
	  - File Flight Plan
		- Settings
	*/
	menu_index = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "skywATCh", 0, 0);
	menu_container = XPLMCreateMenu("skywATCh", XPLMFindPluginsMenu(), menu_index, menu_handler, NULL);
	int flight_plan_index = XPLMAppendMenuItem(menu_container, "File Flight Plan", (void *)"File Flight Plan", 1);
	int settings_index = XPLMAppendMenuItem(menu_container, "Settings", (void *)"Settings", 1);

	// Confirm plugin loaded correctly
	return (flight_plan_index>=0) && (settings_index>=0);
}

PLUGIN_API void	XPluginStop(void)
{
	XPLMDestroyMenu(menu_container);
	XPDestroyWidget(flight_plan_window,1);
}

PLUGIN_API void XPluginDisable(void){}
PLUGIN_API int  XPluginEnable(void){return 1;}
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam){}
// </editor-fold> --------------------------------------------------------------



// <editor-fold> Callbacks -----------------------------------------------------
void menu_handler(void * in_menu_ref, void * in_item_ref)
{
	// Determin which menu item was selected.
	if(!strcmp((const char *)in_item_ref, "File Flight Plan"))
	{
		// Get Window Bounds
		int left, top, right, bottom, width, height;
		XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
		width = right - left;
		height = top - bottom;

		// Create window and set properties
		flight_plan_window = XPCreateWidget(left+100,top-100,right-100,bottom+100,1,"File Flight Plan",1,0,xpWidgetClass_MainWindow);
		XPSetWidgetProperty(flight_plan_window,xpProperty_MainWindowType,xpMainWindowStyle_MainWindow);
		XPSetWidgetProperty(flight_plan_window,xpProperty_MainWindowHasCloseBoxes,1);
		XPAddWidgetCallback(flight_plan_window,window_callback);
	}
	else if(!strcmp((const char *)in_item_ref, "Settings"))
	{

	}
}

int window_callback(XPWidgetMessage inMessage,XPWidgetID inWidget,intptr_t inParam1,intptr_t inParam2)
{
	int return_value = 0;

	// For message command to close window
	if(inMessage==xpMessage_CloseButtonPushed)
	{
		XPDestroyWidget(inWidget,1); // Close the window
		return_value = 1; // Indicate message was processed
	}

	return return_value;
}
// </editor-fold> --------------------------------------------------------------



// <editor-fold> Internal Functions --------------------------------------------

// </editor-fold> --------------------------------------------------------------
