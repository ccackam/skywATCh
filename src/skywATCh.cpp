// Downloaded from https://developer.x-plane.com/code-sample/hello-world-sdk-3/

#include "SDK/CHeaders/XPLM/XPLMDisplay.h"
#include "SDK/CHeaders/XPLM/XPLMGraphics.h"
#include "SDK/CHeaders/XPLM/XPLMMenus.h"
#include "SDK/CHeaders/Widgets/XPWidgets.h"
#include "SDK/CHeaders/Widgets/XPStandardWidgets.h"
#include <string>
#include <stdio.h>

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

#ifndef XPLM300
	#error This is made to be compiled against the XPLM300 SDK
#endif

int 					menu_index;
XPLMMenuID 		menu_container;
std::string	 	plugin_name = "skywATCh";
void 					menu_handler(void *, void *);

static XPLMWindowID 			flight_plan_window;
void											draw(XPLMWindowID in_window_id, void * in_refcon);
int												handle_mouse(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon);

int												dummy_mouse_handler(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon) { return 0; }
XPLMCursorStatus					dummy_cursor_status_handler(XPLMWindowID in_window_id, int x, int y, void * in_refcon) { return xplm_CursorDefault; }
int												dummy_wheel_handler(XPLMWindowID in_window_id, int x, int y, int wheel, int clicks, void * in_refcon) { return 0; }
void											dummy_key_handler(XPLMWindowID in_window_id, char key, XPLMKeyFlags flags, char virtual_key, void * in_refcon, int losing_focus) { }

static XPLMWidgetID 			pop_widget_id;

static const char * pop_out_label = "Undock Window";
static const char * pop_in_label = "Dock Window";
static float pop_button_ltrb[4];

static int	coord_in_rect(float x, float y, float * bounds_ltrb)  { return ((x >= bounds_ltrb[0]) && (x < bounds_ltrb[2]) && (y < bounds_ltrb[1]) && (y >= bounds_ltrb[3])); }

PLUGIN_API int XPluginStart(char * outName, char * outSig, char *	outDesc)
{
	strcpy(outName, plugin_name.c_str());
	strcpy(outSig, ("ccackam."+plugin_name).c_str());
	strcpy(outDesc, "A IFR focused ATC Program.");

	menu_index = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "skywATCh", 0, 0);
	menu_container = XPLMCreateMenu("skywATCh", XPLMFindPluginsMenu(), menu_index, menu_handler, NULL);
	int flight_plan_index = XPLMAppendMenuItem(menu_container, "File Flight Plan", (void *)"File Flight Plan", 1);

	return (flight_plan_index>=0);
}

PLUGIN_API void	XPluginStop(void)
{
	XPLMDestroyMenu(menu_container);
	XPLMDestroyWindow(flight_plan_window);
	flight_plan_window = NULL;
}

PLUGIN_API void XPluginDisable(void){}
PLUGIN_API int  XPluginEnable(void){return 1;}
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam){}

void menu_handler(void * in_menu_ref, void * in_item_ref)
{
	if(!strcmp((const char *)in_item_ref, "File Flight Plan"))
	{
		int global_desktop_bounds[4]; // left, top, right, bottom
		XPLMGetScreenBoundsGlobal(&global_desktop_bounds[0], &global_desktop_bounds[1], &global_desktop_bounds[2], &global_desktop_bounds[3]);

		XPLMCreateWindow_t params;
		params.structSize = sizeof(params);
		params.left = global_desktop_bounds[0] + 50;
		params.bottom = global_desktop_bounds[3] + 150;
		params.right = global_desktop_bounds[0] + 350;
		params.top = global_desktop_bounds[3] + 450;
		params.visible = 1;
		params.drawWindowFunc = draw;
		params.handleMouseClickFunc = handle_mouse;
		params.handleRightClickFunc = dummy_mouse_handler;
		params.handleMouseWheelFunc = dummy_wheel_handler;
		params.handleKeyFunc = dummy_key_handler;
		params.handleCursorFunc = dummy_cursor_status_handler;
		params.refcon = NULL;
		params.layer = xplm_WindowLayerFloatingWindows;
		params.decorateAsFloatingWindow = 1;

		flight_plan_window = XPLMCreateWindowEx(&params);
		XPLMSetWindowPositioningMode(flight_plan_window, xplm_WindowPositionFree, -1);
		XPLMSetWindowGravity(flight_plan_window, 0, 1, 0, 1);
		XPLMSetWindowResizingLimits(flight_plan_window, 200, 200, NULL, NULL);
		XPLMSetWindowTitle(flight_plan_window, "File Flight Plan");
	}
}

void draw(XPLMWindowID in_window_id, void * in_refcon)
{

	float col_white[] = {1.0, 1.0, 1.0};
	char scratch_buffer[150];

	XPLMSetGraphicsState(0,0,0,0,1,1,0);

	int char_height;
	XPLMGetFontDimensions(xplmFont_Proportional, NULL, &char_height, NULL);

	int is_popped_out = XPLMWindowIsPoppedOut(in_window_id);
	const char * pop_label = is_popped_out ? pop_in_label : pop_out_label;

	int l, t, r, b;
	XPLMGetWindowGeometry(in_window_id, &l, &t, &r, &b);

	// Draw pop in/out button
	{
		pop_button_ltrb[1] = t - 10;
		pop_button_ltrb[2] = r - 10;
		pop_button_ltrb[3] = pop_button_ltrb[1] - (1.25f * char_height);
		pop_button_ltrb[0] = pop_button_ltrb[2] - XPLMMeasureString(xplmFont_Proportional, pop_label, strlen(pop_label));

		float green[] = {0.0, 1.0, 0.0, 1.0};
		glColor4fv(green);
		glBegin(GL_LINE_LOOP);
		{
			glVertex2i(pop_button_ltrb[0], pop_button_ltrb[1]);
			glVertex2i(pop_button_ltrb[2], pop_button_ltrb[1]);
			glVertex2i(pop_button_ltrb[2], pop_button_ltrb[3]);
			glVertex2i(pop_button_ltrb[0], pop_button_ltrb[3]);
		}
		glEnd();

		XPLMDrawString(col_white, pop_button_ltrb[0], pop_button_ltrb[3] + 4, (char *)pop_label, NULL, xplmFont_Proportional);
	}

	// Draw a bunch of informative text
	{
		// Set the y position for the first bunch of text we'll draw to a little below the buttons
		int y = pop_button_ltrb[3] - 2 * char_height;

		// Display the total global desktop bounds
		{
			int global_desktop_lbrt[4];
			XPLMGetScreenBoundsGlobal(&global_desktop_lbrt[0], &global_desktop_lbrt[3], &global_desktop_lbrt[2], &global_desktop_lbrt[1]);
			sprintf(scratch_buffer, "Global desktop bounds: (%d, %d) to (%d, %d)", global_desktop_lbrt[0], global_desktop_lbrt[1], global_desktop_lbrt[2], global_desktop_lbrt[3]);
			XPLMDrawString(col_white, l, y, scratch_buffer, NULL, xplmFont_Proportional);
			y -= 1.5 * char_height;
		}

		// Display our bounds
		if(XPLMWindowIsPoppedOut(in_window_id)) // we are in our own first-class window, rather than "floating" within X-Plane's own window
		{
			int window_os_bounds[4];
			XPLMGetWindowGeometryOS(in_window_id, &window_os_bounds[0], &window_os_bounds[3], &window_os_bounds[2], &window_os_bounds[1]);
			sprintf(scratch_buffer, "OS Bounds: (%d, %d) to (%d, %d)", window_os_bounds[0], window_os_bounds[1], window_os_bounds[2], window_os_bounds[3]);
			XPLMDrawString(col_white, l, y, scratch_buffer, NULL, xplmFont_Proportional);
			y -= 1.5 * char_height;
		}
		else
		{
			int global_bounds[4];
			XPLMGetWindowGeometry(in_window_id, &global_bounds[0], &global_bounds[3], &global_bounds[2], &global_bounds[1]);
			sprintf(scratch_buffer, "Window bounds: %d %d %d %d", global_bounds[0], global_bounds[1], global_bounds[2], global_bounds[3]);
			XPLMDrawString(col_white, l, y, scratch_buffer, NULL, xplmFont_Proportional);
			y -= 1.5 * char_height;
		}

		// Display whether we're in front of our our layer
		{
			sprintf(scratch_buffer, "In front? %s", XPLMIsWindowInFront(in_window_id) ? "Y" : "N");
			XPLMDrawString(col_white, l, y, scratch_buffer, NULL, xplmFont_Proportional);
			y -= 1.5 * char_height;
		}

		// Display the mouse's position info text
		{
			int mouse_global_x, mouse_global_y;
			XPLMGetMouseLocationGlobal(&mouse_global_x, &mouse_global_y);
			sprintf(scratch_buffer, "Draw mouse (global): %d %d\n", mouse_global_x, mouse_global_y);
			XPLMDrawString(col_white, l, y, scratch_buffer, NULL, xplmFont_Proportional);
			y -= 1.5 * char_height;
		}
	}
}

int	handle_mouse(XPLMWindowID in_window_id, int x, int y, XPLMMouseStatus is_down, void * in_refcon)
{
	if(is_down == xplm_MouseDown)
	{
		if (!XPLMIsWindowInFront(in_window_id))
		{
			XPLMBringWindowToFront(in_window_id);
		}
		else if(coord_in_rect(x, y, pop_button_ltrb)) // user clicked the pop-in/pop-out button
		{
			XPLMSetWindowPositioningMode(in_window_id, XPLMWindowIsPoppedOut(in_window_id) ? xplm_WindowPositionFree : xplm_WindowPopOut, 0);
		}
	}
	return 1;
}
