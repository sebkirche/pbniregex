$PBExportHeader$n_tooltip.sru
$PBExportComments$Non-Visual tooltip control object
forward
global type n_tooltip from nonvisualobject
end type
type rect from structure within n_tooltip
end type
type toolinfo from structure within n_tooltip
end type
type point from structure within n_tooltip
end type
type msg from structure within n_tooltip
end type
type size from structure within n_tooltip
end type
type str_logfont from structure within n_tooltip
end type
type dllversioninfo from structure within n_tooltip
end type
type initcommoncontrolsex from structure within n_tooltip
end type
end forward

type rect from structure
	long		left
	long		top
	long		right
	long		bottom
end type

type toolinfo from structure
	long		cbsize
	long		uflags
	long		hwnd
	long		uid
	RECT		rect
	long		hinstance
	long		lpsztext
end type

type point from structure
	long		x
	long		y
end type

type msg from structure
	long		hwnd
	long		message
	long		wparam
	long		lparam
	long		time
	point		pt
end type

type size from structure
	long		l_x
	long		l_y
end type

type str_logfont from structure
    long        	lfheight
    long        	lfwidth
    long        	lfescapement
    long       	lforientation
    long        	lfweight
    character     lfitalic
    character     lfunderline
    character     lfstrikeout
    character     lfcharset
    character     lfoutprecision
    character     lfclipprecision
    character     lfquality
    character     lfpitchandfamily
    character     lffacename[32]
end type

type dllversioninfo from structure
	ulong		cbSize
	ulong		dwMajorVersion
	ulong		dwMinorVersion
	ulong		dwBuildNumber
	ulong		dwPlatformID
end type

type initcommoncontrolsex from structure
	ulong		dwSize
	ulong		dwICC
end type

global type n_tooltip from nonvisualobject autoinstantiate
end type

type prototypes
//System functions
Function ulong DllGetVersion(Ref DLLVERSIONINFO pdvi) Library "comctl32"
Function ulong LoadLibrary(Ref String lpszDllName) Library "kernel32" Alias for "LoadLibraryW"
Function boolean FreeLibrary(ulong hModule) Library "kernel32"
Function ulong GetModuleHandle(string lpModuleName) Library "kernel32.dll" Alias for "GetModuleHandleW"
Function ulong GetProcAddress(ulong hModule, String lpProcName) Library "kernel32"  Alias for "GetProcAddress;ansi"
Function boolean InitCommonControlsEx(ref initcommoncontrolsex lpInitCtrls) Library "comctl32"

// ToolTip Functions
SubRoutine InitCommonControls() library "comctl32.dll"
Function long CreateWindowEx(ulong dwExStyle, string ClassName, long WindowName, ulong dwStyle, ulong X, ulong Y, ulong nWidth, ulong nHeight, ulong hWndParent, ulong hMenu, ulong hInstance, ulong lpParam) library "user32.dll" alias for "CreateWindowExW"
Function integer DestroyWindow(long hWnd) library "user32.dll"
Function integer ToolTipMsg(long hWnd, long uMsg, long wParam, REF TOOLINFO ToolInfo) library "user32.dll" Alias For "SendMessageW"//;Ansi"
Function integer RelayMsg(long hWnd, long uMsg, long wParam, REF MSG Msg) library "user32.dll" Alias For "SendMessageW"//;Ansi"

// Memory handling functions
Function long LocalAlloc(long Flags, long Bytes) library "kernel32.dll"
Function long LocalFree(long MemHandle) library "kernel32.dll"
Function long lstrcpy( long Destination, string Source) library "kernel32.dll" Alias For "lstrcpy;Ansi"

Function uLong SendMessageString( uLong hwnd, uLong Msg, uLong wParam, String lpzString ) Library "user32.dll" Alias For "SendMessageW"//;Ansi"

Function uLong CreateFontIndirectA( Ref str_Logfont lplf ) Library 'gdi32.dll' alias for "CreateFontIndirectW"//;Ansi"
Function Boolean DeleteObject( uLong hObject ) Library "gdi32.dll"

end prototypes

type variables

/* Wrapper for system ToolTips

	Author	Rui Cruz (and for a part Aart Onkenhout)
	Date	10-05-2001
	
	Adapted for unicode PowerBuilder By Sébastien Kirche - 07-2013

	Availabl eonline at http://pbsite.milente.nl/pbtip015.html
*/

Private:

// Misc Constants
Constant string TOOLTIPS_CLASS		= 'tooltips_class32'
Constant ulong CW_USEDEFAULT		= 2147483648
Constant long WM_USER 		= 1024
Constant long WS_EX_TOPMOST		= 8
Constant long  WM_SETFONT          	= 48
Constant long  WM_GETFONT          	= 49

// ToolTip Messages
Constant long TTM_SETDELAYTIME = WM_USER + 3
Constant long TTM_ADDTOOL 		= WM_USER + 4
Constant long TTM_ADDTOOLW = WM_USER + 50
Constant long TTM_DELTOOL = WM_USER + 5
Constant long TTM_DELTOOLW = WM_USER + 51
Constant long TTM_NEWTOOLRECT	= WM_USER + 6
Constant long TTM_NEWTOOLRECTW = WM_USER + 52
Constant long TTM_RELAYEVENT 		= WM_USER + 7
Constant long TTM_UPDATETIPTEXT	= WM_USER + 12
Constant long TTM_UPDATETIPTEXTW = WM_USER + 57
Constant long TTM_TRACKACTIVATE	= WM_USER + 17
Constant long TTM_TRACKPOSITION	= WM_USER + 18
Constant long TTM_SETTIPBKCOLOR	= WM_USER + 19
Constant long TTM_SETTIPTEXTCOLOR	= WM_USER + 20
Constant long TTM_GETDELAYTIME = WM_USER + 21
Constant long TTM_SETMAXTIPWIDTH	= WM_USER + 24
Constant long TTM_GETMAXTIPWIDTH	= WM_USER + 25
Constant long TTM_SETTITLEA = WM_USER + 32
Constant long TTM_SETTITLEW = WM_USER + 33

// Public variables and Constants
Public:
long hWndTT	// Tooltip control window handle
long ToolID = 1	// Tooltip internal ID

// Tooltip flags
Constant integer TTF_CENTERTIP 		= 2
Constant integer TTF_RTLREADING	= 4
Constant integer TTF_SUBCLASS		= 16
Constant integer TTF_TRACK		= 32
Constant integer TTF_ABSOLUTE		= 128
Constant integer TTF_TRANSPARENT	= 256
Constant integer TTF_DI_SETITEM		= 32768
Constant integer TTS_BALLOON = 64

//	Title Constants
Constant integer TTI_NONE = 0
Constant integer TTI_INFO = 1
Constant integer TTI_WARNING = 2
Constant integer TTI_ERROR = 3

//	Delaytime constants
Constant integer TTDT_AUTOMATIC = 0
Constant integer TTDT_RESHOW = 1
Constant integer TTDT_AUTOPOP = 2
Constant integer TTDT_INITIAL = 3

uLong		iul_Handle[]

end variables
forward prototypes
public subroutine of_setfont (long hfont)
public subroutine of_settipposition (integer ai_x, integer ai_y)
public subroutine of_settrack (dragobject ado_object, integer ai_uid, boolean ab_status)
public subroutine of_updatetiprect (dragobject ado_object, long al_uid, long al_left, long al_top, long al_right, long al_bottom)
public function long of_getfont ()
public subroutine of_relaymsg (dragobject ado_object)
public subroutine of_setmaxwidth (long al_maxwidth)
public function integer of_removetool (dragobject ado_object, integer ai_toolid)
public subroutine of_settipbkcolor (unsignedlong aul_color)
public subroutine of_settiptextcolor (unsignedlong aul_color)
public subroutine of_settiptitle (integer ai_icon, string as_title)
public subroutine of_settiptext (dragobject ado_object, long al_uid, long al_tiptext)
public subroutine of_settiptext (dragobject ado_object, long al_uid, string as_tiptext)
public subroutine of_setdelaytime (integer ai_duration, integer ai_milliseconds)
public function integer of_addtool (dragobject ado_object, string as_tiptext, integer ai_flags)
public subroutine of_setfont (string as_facename, integer ai_height, integer ai_weight, boolean ab_italic, boolean ab_underline, boolean ab_strikeout)
private function unsignedlong of_createfont (string as_facename, integer ai_height, integer ai_weight, boolean ab_italic, boolean ab_underline, boolean ab_strikeout)
public function integer of_getdelaytime (integer ai_duration)
public function boolean is_extendedcomctrls ()
end prototypes

public subroutine of_setfont (long hfont);/*****************************************************************************

	Function:		of_setfont

	Description:	Sets the font used in the tooltip window

	Returns:			(none)

	Arguments:		

	Use:				

******************************************************************************/

Send( hWndTT, WM_SETFONT, hFont, 1 )

end subroutine

public subroutine of_settipposition (integer ai_x, integer ai_y);/*****************************************************************************

	Function:		of_settipposition

	Description:	This function sets the position of a tracking tooltip.

	Returns:			(none)

	Arguments:		Integer		ai_X 	=> X position of the tooltip
										ai_Y	=> Y position of the tooltip

	Notes: 			The tooltip control chooses where the tooltip will be displayed (typically near
			 			the tool) unless the TTF_ABSOLUTE flags is specified when adding the tool

	Use:				

******************************************************************************/

Send( hWndTT, TTM_TRACKPOSITION, 0, Long( ai_X, ai_Y ) )

end subroutine

public subroutine of_settrack (dragobject ado_object, integer ai_uid, boolean ab_status);/*****************************************************************************

	Function:		of_settrack

	Description:	This function (de)activates a tracking tooltip, this kind of 
						tooltip can be repositioned on the screen using the 
						of_settipposition function.

	Returns:			(none)

	Arguments:		DragObject		ado_object	: Object registered on the toolwindow control
						Long				al_uID	 	: Internal ID of the object
						Boolean			ab_status	: True to activate tracking, False to deactivate tracking

	Use:				

******************************************************************************/

TOOLINFO ToolInfo

ToolInfo.cbSize	= 40
ToolInfo.hWnd		= Handle( ado_Object )
ToolInfo.uID		= ai_uID

If ab_Status Then 
	ToolTipMsg( hWndTT, TTM_TRACKACTIVATE, 1, ToolInfo )
Else
	ToolTipMsg( hWndTT, TTM_TRACKACTIVATE, 0, ToolInfo )
End If
end subroutine

public subroutine of_updatetiprect (dragobject ado_object, long al_uid, long al_left, long al_top, long al_right, long al_bottom);/*****************************************************************************

	Function:		of_updatetiprect

	Description:	Updates the tip rectangle

	Returns:			(none)

	Arguments:		DragObject		ado_object	: Object registered on the toolwindow control
						Long				al_uID	 	: Internal ID of the object
						Long				al_Left		: Left coordinate of rectangle
						Long				al_top		: Top coordinate of rectangle
						Long				al_right		: Right coordinate of rectangle
						Long				al_bottom	: Bottom coordinate of rectangle

	Use:				

******************************************************************************/

TOOLINFO ToolInfo

ToolInfo.hWnd	= Handle( ado_Object )
ToolInfo.uID	= al_uID

ToolInfo.Rect.Left	= al_Left
ToolInfo.Rect.Top		= al_Top
ToolInfo.Rect.Right	= al_Right
ToolInfo.Rect.Bottom	= al_Bottom

ToolTipMsg( hWndTT, TTM_NEWTOOLRECT, 0, ToolInfo )

end subroutine

public function long of_getfont ();/*****************************************************************************

	Function:		of_getfont

	Description:	Gets the font used in the tooltip window

	Returns:			(none)

	Arguments:		

	Use:				Call from mousemove-event for control

******************************************************************************/

uLong			lul_Font

lul_Font = Send( hWndTT, WM_GETFONT, 0, 0 )

Return lul_Font

end function

public subroutine of_relaymsg (dragobject ado_object);/*****************************************************************************

	Function:		of_relaymsg

	Description:	This function will send the control message to the 
						toolwindow control.

	Returns:			(none)

	Arguments:		DragObject		ado_Object

	Use:				Call from mousemove-event for control

******************************************************************************/

MSG Msg

Msg.hWnd		= Handle( ado_Object )
Msg.Message	= 512	// WM_MOUSEMOVE
Msg.WParam 	= Message.WordParm
Msg.LParam 	= Message.LongParm

RelayMsg( hWndTT, TTM_RELAYEVENT, 0, Msg )

end subroutine

public subroutine of_setmaxwidth (long al_maxwidth);/*****************************************************************************

	Function:		of_setmaxwidth

	Description:	Sets the maximum tooltip width. If the text is longer it will
						splitted over more than one line.

	Returns:			(none)

	Arguments:		Long 		al_MaxWidth

	Use:				Call to set the maximum width.

-------------------------------------------------------------------------------

	Auteur:	Aart Onkenhout

	Revision History
	--------------------
	Date			Version
	15-05-2000	1.0		Initial version

******************************************************************************/

Send( hWndTT, TTM_SETMAXTIPWIDTH, 0, UnitsToPixels( al_MaxWidth, xUnitsToPixels! ) )

Return

end subroutine

public function integer of_removetool (dragobject ado_object, integer ai_toolid);/*****************************************************************************

	Function:		of_removetool

	Description:	Unregisters a control within the tooltip control

	Returns:			(none)

	Arguments:		DragObject		ado_Object	Object to unregister within the tooltip control
						Integer			ai_ToolId	Tool Id (returned by of_AddTool)

	Use:				

-------------------------------------------------------------------------------

	Auteur:	Aart Onkenhout

	Revision History
	--------------------
	Date			Version
	29-06-2001	1.0		Initial version

******************************************************************************/

TOOLINFO ToolInfo
Integer	li_Width, li_Height

ToolInfo.cbSize 	= 40
ToolInfo.uFlags 	= 16	//Flags 
ToolInfo.hWnd		= Handle( ado_Object )
ToolInfo.hInstance= 0 // Not used 
ToolInfo.uID		= ai_ToolID
iul_Handle[ToolID] = ToolInfo.hWnd

ToolTipMsg( hWndTT, TTM_DELTOOL, 0, ToolInfo )

Return 1

end function

public subroutine of_settipbkcolor (unsignedlong aul_color);/*****************************************************************************

	Function:		of_settipbkcolor

	Description:	Sets the backgroundcolor of the tooltip-window

	Returns:			(none)

	Arguments:		uLong		aul_Color	

	Use:				Call with the desired color

-------------------------------------------------------------------------------

	Auteur:	Aart Onkenhout

	Revision History
	--------------------

	Date			Version
	10-05-2001	1.0		Initial version

******************************************************************************/

Send( hWndTT, TTM_SETTIPBKCOLOR, aul_Color, 0 )

end subroutine

public subroutine of_settiptextcolor (unsignedlong aul_color);/*****************************************************************************

	Function:		of_settiptextcolor

	Description:	Sets the textcolor

	Returns:			(none)

	Arguments:		uLong		aul_Color	

	Use:				Call with the desired color

-------------------------------------------------------------------------------

	Auteur:	Aart Onkenhout

	Revision History
	--------------------
	Date			Version
	10-05-2001	1.0		Initial version

******************************************************************************/

Send( hWndTT, TTM_SETTIPTEXTCOLOR, aul_Color, 0 )
end subroutine

public subroutine of_settiptitle (integer ai_icon, string as_title);/*****************************************************************************

	Function:		of_settiptitle

	Description:	Sets the title of the tooltip

	Returns:			(none)

	Arguments:		Integer		ai_Icon
						Values:		TTI_NONE = 0
										TTI_INFO = 1
										TTI_WARNING = 2
										TTI_ERROR = 3
						String		as_Title

	Use:				Call with the desired title and icon.

-------------------------------------------------------------------------------

	Auteur:	Aart Onkenhout

	Revision History
	--------------------
	Date			Version
	10-05-2001	1.0		Initial version

******************************************************************************/

SendMessageString( hWndTT, TTM_SETTITLEW, ai_Icon, as_Title )

end subroutine

public subroutine of_settiptext (dragobject ado_object, long al_uid, long al_tiptext);/*****************************************************************************

	Function:		of_settiptext

	Description:	Sets the new text for a tool window

	Returns:			(none)

	Arguments:		DragObject		ado_object	: Object registered on the toolwindow control
						Long				al_uID	 	: Object ID
						Long				al_text		: Tooltip text

	Use:				

******************************************************************************/

TOOLINFO ToolInfo

ToolInfo.cbSize 	= 40
ToolInfo.hWnd		= Handle( ado_Object )
ToolInfo.uID		= al_uID
ToolInfo.lpszText	= al_TipText

ToolTipMsg( hWndTT, TTM_UPDATETIPTEXT, 0, ToolInfo )

end subroutine

public subroutine of_settiptext (dragobject ado_object, long al_uid, string as_tiptext);/*****************************************************************************

	Function:		of_settiptext

	Description:	Sets the new text for a tool window

	Returns:			(none)

	Arguments:		DragObject		ado_object	: Object registered on the toolwindow control
						Long				al_uID	 	: Object ID
						Long				as_tiptext	: Tooltip text


	Use:				

******************************************************************************/

Long	lpszText

lpszText = LocalAlloc( 0, LenA( as_TipText ) )
lStrCpy( lpszText, as_TipText )
of_SetTipText( ado_Object, al_uId, lpszText )

LocalFree( lpszText )
end subroutine

public subroutine of_setdelaytime (integer ai_duration, integer ai_milliseconds);/*****************************************************************************

	Function:		of_setdelaytime

	Description:	Sets the initial, pop-up, and reshow durations for a ToolTip control.

	Returns:			(none)

	Arguments:		Integer 		ai_Duration
						Possible values:
							TTDT_AUTOPOP: 		Set the length of time a ToolTip window remains visible if the 
													pointer is stationary within a tool's bounding rectangle. To 
													return the autopop delay time to its default value, set iTime to -1.
							TTDT_INITIAL: 		Set the length of time a pointer must remain stationary within
													a tool's bounding rectangle before the ToolTip window appears. 
													To return the initial delay time to its default value, set iTime to -1.
							TTDT_RESHOW:		Set the length of time it takes for subsequent ToolTip windows 
													to appear as the pointer moves from one tool to another. 
													To return the reshow delay time to its default value, set iTime to -1.
							TTDT_AUTOMATIC:	Set all three delay times to default proportions. The autopop time 
													will be ten times the initial time and the reshow time will be one 
													fifth the initial time. If this flag is set, use a positive value 
													of iTime to specify the initial time, in milliseconds. Set iTime 
													to a negative value to return all three delay times to their 
													default values.
						
						Integer		ai_Milliseconds

	Use:				IMPORTANT: call before adding tools by using of_AddTool!

-------------------------------------------------------------------------------

	Auteur:	Aart Onkenhout

	Revision History
	--------------------
	Date			Version
	03-02-2005	1.0		Initial version

******************************************************************************/
Send( hWndTT, TTM_SETDELAYTIME, ai_Duration, Long( ai_Milliseconds, 0 ) )

end subroutine

public function integer of_addtool (dragobject ado_object, string as_tiptext, integer ai_flags);/*****************************************************************************

	Function:		of_AddTool

	Description:	Registers a control within the tooltip control

	Returns:			(none)

	Arguments:		DragObject		ado_Object	Object to register within the tooltip control
						String			as_TipText	Tooltip Text
						Integer			ai_Flags		Tool creation flags

	Use:				Call from mousemove-event for control

******************************************************************************/

TOOLINFO ToolInfo
Integer	li_Width, li_Height

ToolInfo.cbSize 	= 40
ToolInfo.uFlags 	= ai_Flags	//TTF_SUBCLASS	//Flags 
ToolInfo.hWnd		= Handle( ado_Object )
ToolInfo.hInstance= 0 // Not used 
ToolInfo.uID		= ToolID
iul_Handle[ToolID] = ToolInfo.hWnd
ToolID++
ToolInfo.lpszText	= LocalAlloc( 0, LenA( as_TipText ) )
POST LocalFree( ToolInfo.lpszText ) // Free Allocated Memory
lStrCpy( ToolInfo.lpszText, as_tiptext )

//	Define the object as a rectangle
ToolInfo.Rect.Left	= 0
ToolInfo.Rect.Top 	= 0
ToolInfo.Rect.Right	= UnitsToPixels( ado_Object.Width, XUnitsToPixels! )
ToolInfo.Rect.Bottom	= UnitsToPixels( ado_Object.Height, YUnitsToPixels! )

If ToolTipMsg( hWndTT, TTM_ADDTOOL, 0, ToolInfo ) = 0 Then
	MessageBox( "Error", "Cannot register object in the toolwindow control!", StopSign!, Ok! )
	Return( -1 )
End If

Return ( ToolID - 1 )

end function

public subroutine of_setfont (string as_facename, integer ai_height, integer ai_weight, boolean ab_italic, boolean ab_underline, boolean ab_strikeout);uLong		lul_Font

lul_Font = of_CreateFont( as_FaceName, ai_Height, ai_Weight, ab_Italic, ab_Underline, ab_StrikeOut )

If lul_Font > 0 Then
	of_SetFont( lul_Font )
	DeleteObject( lul_Font )
End If

end subroutine

private function unsignedlong of_createfont (string as_facename, integer ai_height, integer ai_weight, boolean ab_italic, boolean ab_underline, boolean ab_strikeout);uLong			lul_Font
str_logfont	lstr_lf

lstr_lf.lfFaceName = as_FaceName
lstr_lf.lfWeight= ai_Weight
lstr_lf.lfHeight= ai_Height

lstr_Lf.lfPitchAndFamily = '1'
lstr_Lf.lfClipPrecision = Char(2)
lstr_Lf.lfOutPrecision = Char(1)
lstr_Lf.lfQuality = Char(1)

If ab_Italic Then
	lstr_lf.lfItalic = Char(255)
End If
If ab_UnderLine Then
	lstr_lf.lfUnderline = Char(255)
End If
If ab_StrikeOut Then
	lstr_lf.lfStrikeOut = Char(255)
End If

lul_Font = CreateFontIndirectA( lstr_lf )

Return lul_Font

end function

public function integer of_getdelaytime (integer ai_duration);/*****************************************************************************

	Function:		of_setdelaytime

	Description:	Sets the initial, pop-up, and reshow durations for a ToolTip control.

	Returns:			Integer

	Arguments:		Integer 		ai_Duration
						Possible values:
							TTDT_AUTOPOP: 		Set the length of time a ToolTip window remains visible if the 
													pointer is stationary within a tool's bounding rectangle. To 
													return the autopop delay time to its default value, set iTime to -1.
							TTDT_INITIAL: 		Set the length of time a pointer must remain stationary within
													a tool's bounding rectangle before the ToolTip window appears. 
													To return the initial delay time to its default value, set iTime to -1.
							TTDT_RESHOW:		Set the length of time it takes for subsequent ToolTip windows 
													to appear as the pointer moves from one tool to another. 
													To return the reshow delay time to its default value, set iTime to -1.
						
-------------------------------------------------------------------------------

	Auteur:	Aart Onkenhout

	Revision History
	--------------------
	Date			Version
	25-02-2005	1.0		Initial version

******************************************************************************/
Return Send( hWndTT, TTM_GETDELAYTIME, ai_Duration, 0 )

end function

public function boolean is_extendedcomctrls ();
// Local Variables
ulong llDLL, llDllVersion, llResult
String lsLibraryName, lsFunctionName
DLLVERSIONINFO lstrVersionInfo
boolean lb_ret = false

// Constants for Platform ID
// 0x00000001 = Windows 95
Constant long DLLVER_PLATFORM_WINDOWS = 1
// 0x00000002 = Windows NT
Constant long DLLVER_PLATFORM_NT = 2

// Set Library name and Function name
// for reference arguments
lsLibraryName = "comctl32.dll"
lsFunctionName = "DllGetVersion"

// Load DLL into memory
llDLL = LoadLibrary( lsLibraryName )

// Did library load successfully ?
IF (llDLL = 0) THEN
	MessageBox( "LoadLibrary Failed", "Could not load library: " + lsLibraryName )
ELSE
	// Find DllGetVersion entry point
	llDllVersion = GetProcAddress(llDLL, lsFunctionName)
	// Is DllGetVersion supported ?
	IF (llDllVersion = 0) THEN
		MessageBox( "GetProcAddress Failed", lsFunctionName + " is not supported in library: " + lsLibraryName )
	ELSE
		// Set size before calling Win32 API function
		lstrVersionInfo.cbsize = 20
		llResult = DllGetVersion(lstrVersionInfo)
		// Check Version information
//		MessageBox( "Major Version", lstrVersionInfo.dwMajorVersion )
//		MessageBox( "Minor Version", lstrVersionInfo.dwMinorVersion )
//		MessageBox( "Build", lstrVersionInfo.dwBuildNumber )
//		MessageBox( "Platform", lstrVersionInfo.dwPlatformID )
		
		if lstrVersionInfo.dwMajorVersion >= 6 then lb_ret = true
	END IF
	
	// Cleanup and Unload library
	FreeLibrary( llDLL )
END IF

return lb_ret

end function

on n_tooltip.create
call super::create
TriggerEvent( this, "constructor" )
end on

on n_tooltip.destroy
TriggerEvent( this, "destructor" )
call super::destroy
end on

event constructor;
//Under Comctl32.dll version 6.0 and later, InitCommonControls does nothing. 
//Applications must explicitly register all common controls through InitCommonControlsEx.
if is_extendedcomctrls( ) then
	constant ulong ICC_BAR_CLASSES = 4
	initcommoncontrolsex iccex
	iccex.dwsize = 8
	iccex.dwICC = ICC_BAR_CLASSES
	InitCommonControlsEx(iccex)
else
	InitCommonControls()
end if

hWndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, 0, TTF_CENTERTIP + 1, &
         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,  &
         0, 0, Handle(GetApplication()),0)

end event

event destructor;DestroyWindow(hWndTT)
end event

