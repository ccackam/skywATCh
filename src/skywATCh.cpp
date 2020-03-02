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
#include <math.h>

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
#include "XPWidgetUtils.h"
#include "XPLMPlugin.h"
#include "XPLMGraphics.h"
#include "XPLMDisplay.h"
#include "XPLMMenus.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "texture.h"

#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

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

// Flight Plan Window
XPLMWindowID 				flight_plan_window = NULL;
void								draw_file_flight_plan_window(XPLMWindowID in_window_id, void * in_refcon);
PPL::Texture* 			flight_plan_texture;
XPWidgetID 					flight_plan_widgets[22];
float 							flight_plan_widget_vertices[22][4] =  {{0.002632,0.790000,0.027193,0.820000}, // Box 1a
																									 			   {0.002632,0.751667,0.027193,0.790000}, // Box 1b
																												   {0.002632,0.715000,0.027193,0.751667}, // Box 1c
																												   {0.077193,0.715000,0.221053,0.810000}, // Box 2
																											     {0.221053,0.715000,0.376316,0.810000}, // Box 3
																											     {0.376316,0.745000,0.476316,0.810000}, // Box 4
																											     {0.476316,0.715000,0.674561,0.828333}, // Box 5
																											     {0.674561,0.715000,0.780702,0.791667}, // Box 6a
																											     {0.780702,0.715000,0.884211,0.791667}, // Box 6b
																											     {0.884211,0.715000,0.998246,0.805000}, // Box 7
																											     {0.002632,0.471667,0.998246,0.680000}, // Box 8
																											     {0.002632,0.291667,0.210526,0.420000}, // Box 9
																											     {0.210526,0.291667,0.304386,0.403333}, // Box 10a
																											     {0.304386,0.291667,0.392105,0.403333}, // Box 10b
																											     {0.392105,0.291667,0.998246,0.431667}, // Box 11
																											     {0.002632,0.111667,0.088596,0.230000}, // Box 12a
																											     {0.088596,0.111667,0.175439,0.230000}, // Box 12b
																											     {0.175439,0.111667,0.392105,0.256667}, // Box 13
																											     {0.392105,0.198333,0.888596,0.258333}, // Box 14
																											     {0.888596,0.111667,0.998246,0.233333}, // Box 15
																											     {0.002632,0.005000,0.243737,0.081667}, // Box 16
																										 	     {0.392105,0.111667,0.888596,0.171667}};// Box 17


// Settings Window
static XPWidgetID 			settings_window = NULL;

// Settings

// Internal Settings
int window_inset = 100;
int content_inset = 50;

// Flight Plan Info

// General Functions
int					dummy_mouse_handler(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon) { return 0; }
XPLMCursorStatus	dummy_cursor_status_handler(XPLMWindowID in_window_id, int x, int y, void * in_refcon) { return xplm_CursorDefault; }
int					dummy_wheel_handler(XPLMWindowID in_window_id, int x, int y, int wheel, int clicks, void * in_refcon) { return 0; }
void				dummy_key_handler(XPLMWindowID in_window_id, char key, XPLMKeyFlags flags, char virtual_key, void * in_refcon, int losing_focus) { }

// Utilities
bool draw_bounding_box = 0;
float indexes[4] = {0,0,0,0};
// TODO: Switch Debouncing
// TODO: Clean up texturing Code
// TODO: make measuring box green

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

	flight_plan_texture = new PPL::Texture("/Users/kameroneves/Library/Application Support/Steam/steamapps/common/X-Plane 11/Resources/plugins/skywATCh/Materials/FAA_Flight_Plan_Form.bmp",1);

	// Confirm plugin loaded correctly
	return (flight_plan_index>=0) && (settings_index>=0);
}

PLUGIN_API void	XPluginStop(void)
{
	XPLMDestroyMenu(menu_container);
	XPLMDestroyWindow(flight_plan_window);
	flight_plan_texture->~Texture();
}

PLUGIN_API void XPluginDisable(void){}
PLUGIN_API int  XPluginEnable(void){return 1;}
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam){}
// </editor-fold> --------------------------------------------------------------



// <editor-fold> Callbacks -----------------------------------------------------
void menu_handler(void * in_menu_ref, void * in_item_ref)
{
	if(!strcmp((const char *)in_item_ref, "File Flight Plan"))
	{
		// Get general Parameters
		int left, top, right, bottom, l, t, r, b, width, height;
		XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
		left += content_inset;
		top -= content_inset;
		right -= content_inset;
		bottom += content_inset;
		width = right-left;
		height = top-bottom;


		// Create Window
		left += window_inset; top -= window_inset; right -= window_inset; bottom += window_inset;
		XPLMCreateWindow_t window_params;
		window_params.structSize = sizeof(window_params);
		window_params.left = left;
		window_params.top = top;
		window_params.right = right;
		window_params.bottom = bottom;
		window_params.visible = 1;
		window_params.drawWindowFunc = draw_file_flight_plan_window;
		window_params.handleMouseClickFunc = dummy_mouse_handler;
		window_params.handleRightClickFunc = dummy_mouse_handler;
		window_params.handleMouseWheelFunc = dummy_wheel_handler;
		window_params.handleKeyFunc = dummy_key_handler;
		window_params.handleCursorFunc = dummy_cursor_status_handler;
		window_params.refcon = NULL;
		window_params.layer = xplm_WindowLayerFloatingWindows;
		window_params.decorateAsFloatingWindow = 1;

		flight_plan_window = XPLMCreateWindowEx(&window_params);
		XPLMSetWindowPositioningMode(flight_plan_window, xplm_WindowPositionFree, -1);
		XPLMSetWindowGravity(flight_plan_window, 0, 1, 0, 1); // As the X-Plane window resizes, keep our size constant, and our left and top edges in the same place relative to the window's left/top
		XPLMSetWindowResizingLimits(flight_plan_window, 200, 200, 0, 0); // Limit resizing our window: maintain a minimum width/height of 200 boxels and a max width/height of 500
		XPLMSetWindowTitle(flight_plan_window, "File Flight Plan");
	}
}

void draw_file_flight_plan_window(XPLMWindowID in_window_id, void * in_refcon)
{
	int left, top, right, bottom, l, t, r, b, width, height;
	XPLMGetWindowGeometry(in_window_id, &left, &top, &right, &bottom);

	left += content_inset;
	top -= content_inset;
	right -= content_inset;
	bottom += content_inset;
	width = right-left;
	height = top-bottom;

	XPLMSetGraphicsState(0,1,0,0,1,1,0);
	int blend_src, blend_dst;
	glGetIntegerv(GL_BLEND_SRC, &blend_src);
	glGetIntegerv(GL_BLEND_DST, &blend_dst);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1,1,1,1);

	XPLMBindTexture2d(flight_plan_texture->id(), 0);
  glBegin(GL_QUADS);
	{
	  glTexCoord2f(0,1); glVertex2f(left, top);
	  glTexCoord2f(1,1); glVertex2f(right, top);
	  glTexCoord2f(1,0); glVertex2f(right, bottom);
	  glTexCoord2f(0,0); glVertex2f(left, bottom);
	}
  glEnd();
}
// </editor-fold> --------------------------------------------------------------



// <editor-fold> Internal Functions --------------------------------------------
// </editor-fold> --------------------------------------------------------------
 // Box
