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
XPLMWindowID 				flight_plan_window_ = NULL;
void								draw_file_flight_plan_window(XPLMWindowID in_window_id, void * in_refcon);
PPL::Texture* 			flight_plan_texture;

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
XPLMCursorStatus	courser_box_maker(XPLMWindowID in_window_id, int x, int y, void * in_refcon)
{
	int left, top, right, bottom, width, height;
	XPLMGetWindowGeometry(in_window_id,&left, &top, &right, &bottom);
	left += content_inset;
	top -= content_inset;
	right -= content_inset;
	bottom += content_inset;
	width = right-left;
	height = top-bottom;
	if(draw_bounding_box)
	{
		indexes[2]=(float)(x-left)/(float)width;
		indexes[3]=(float)(y-bottom)/(float)height;
	}
	return xplm_CursorDefault;
}
int					mouse_box_maker(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon)
{
	if(is_down=xplm_MouseDown)
	{
		int left, top, right, bottom, width, height;
		XPLMGetWindowGeometry(in_window_id,&left, &top, &right, &bottom);
		left += content_inset;
		top -= content_inset;
		right -= content_inset;
		bottom += content_inset;
		width = right-left;
		height = top-bottom;
		indexes[0+draw_bounding_box*2]=(float)(x-left)/(float)width;
		indexes[1+draw_bounding_box*2]=(float)(y-bottom)/(float)height;
		draw_bounding_box = !draw_bounding_box;
		return 1;
	}
	else
	{
		return 0;
	}
}

int loadDDS(const char * imagepath);


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
	XPLMDestroyWindow(flight_plan_window_);
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
		int left, top, right, bottom;
		XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);

		// Create Window
		left += window_inset; top -= window_inset; right -= window_inset; bottom += window_inset;
		XPLMCreateWindow_t params;
		params.structSize = sizeof(params);
		params.left = left;
		params.top = top;
		params.right = right;
		params.bottom = bottom;
		params.visible = 1;
		params.drawWindowFunc = draw_file_flight_plan_window;
		params.handleMouseClickFunc = mouse_box_maker;
		params.handleRightClickFunc = dummy_mouse_handler;
		params.handleMouseWheelFunc = dummy_wheel_handler;
		params.handleKeyFunc = dummy_key_handler;
		params.handleCursorFunc = courser_box_maker;
		params.refcon = NULL;
		params.layer = xplm_WindowLayerFloatingWindows;
		params.decorateAsFloatingWindow = 1;

		flight_plan_window_ = XPLMCreateWindowEx(&params);
		XPLMSetWindowPositioningMode(flight_plan_window_, xplm_WindowPositionFree, -1);
		XPLMSetWindowGravity(flight_plan_window_, 0, 1, 0, 1); // As the X-Plane window resizes, keep our size constant, and our left and top edges in the same place relative to the window's left/top
		XPLMSetWindowResizingLimits(flight_plan_window_, 200, 200, 0, 0); // Limit resizing our window: maintain a minimum width/height of 200 boxels and a max width/height of 500
		XPLMSetWindowTitle(flight_plan_window_, "File Flight Plan");
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

	XPLMSetGraphicsState(0,1,0,0,1,0,0);
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
	l = round(fmin(indexes[0],indexes[2])*width+left);
	r = round(fmax(indexes[0],indexes[2])*width+left);
	b = round(fmin(indexes[1],indexes[3])*height+bottom);
	t = round(fmax(indexes[1],indexes[3])*height+bottom);
	float green[4] = {0.0,1.0,0.0,1.0};
	glColor4fv(green);
	glBegin(GL_LINE_LOOP);
		 glVertex2f(l, b);
		 glVertex2f(r, b);
		 glVertex2f(r, t);
		 glVertex2f(l, t);
	glEnd();

	char scratch_buffer[150];
	float col_white[] = {1.0, 1.0, 1.0};
	sprintf(scratch_buffer, "Coordinates ( %f0.5 , %f0.5 ) ( %f0.5 , %f0.5 )", indexes[0], indexes[1], indexes[2], indexes[3]);
	XPLMDrawString(col_white, left, bottom-10, scratch_buffer, NULL, xplmFont_Proportional);
}
// </editor-fold> --------------------------------------------------------------



// <editor-fold> Internal Functions --------------------------------------------
// </editor-fold> --------------------------------------------------------------
