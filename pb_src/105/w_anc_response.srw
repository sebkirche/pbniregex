$PBExportHeader$w_anc_response.srw
$PBExportComments$Ancestor of response windows
forward
global type w_anc_response from window
end type
type st_point from structure within w_anc_response
end type
type st_minmaxinfo from structure within w_anc_response
end type
end forward

type st_point from structure
long lx
long ly
end type

type st_minmaxinfo from structure
st_point ptreserverd
st_point ptmaxsize
st_point ptmaxposition
st_point ptmintracksize
st_point ptmaxtracksize
end type

global type w_anc_response from window
integer width = 2505
integer height = 1384
boolean titlebar = true
string title = "Untitled"
boolean controlmenu = true
windowtype windowtype = response!
long backcolor = 67108864
string icon = "AppIcon!"
boolean center = true
event ue_getminmaxinfo pbm_getminmaxinfo
end type
global w_anc_response w_anc_response

type prototypes
FUNCTION long GetWindowLong(long hwnd,long nIndex) LIBRARY "user32.dll" ALIAS FOR "GetWindowLongW"
FUNCTION long SetWindowLong(long hwnd,long nIndex,long dwNewLong) LIBRARY "user32.dll" ALIAS FOR "SetWindowLongW"
FUNCTION ulong GetSystemMenu(ulong hWnd, BOOLEAN bRevert)  Library "USER32"
FUNCTION boolean DeleteMenu( ulong hMenu, uint uPosition, uint uFlags ) LIBRARY "user32.dll"
FUNCTION boolean DrawMenuBar( ulong hWnd ) LIBRARY "user32.dll"
subroutine GetMinMaxInfo ( ref st_minmaxinfo d, long s, long l ) library 'kernel32.dll' alias for RtlMoveMemory 
subroutine SetMinMaxInfo ( long d, st_minmaxinfo s, long l ) library 'kernel32.dll' alias for RtlMoveMemory 

end prototypes

type variables
nv_auto_resizer inv_resizer
boolean ib_resize_handler = false
boolean ib_resizable = false
long il_minwidth
long il_minheight
long il_maxwidth
long il_maxheight


end variables

forward prototypes
public subroutine of_makeresizable ()
end prototypes

event ue_getminmaxinfo;st_minmaxinfo lstr_minmaxinfo
Environment le_env

if ib_resizable then
	GetEnvironment(le_env)
	
	// copy the data to local structure
	GetMinMaxInfo(lstr_minmaxinfo, minmaxinfo, 40)
	
	// set the minimum size for our window
	lstr_minmaxinfo.ptMinTrackSize.lx = il_minwidth
	lstr_minmaxinfo.ptMinTrackSize.ly = il_minheight
	
	// set the maximum size for our window
	if il_maxwidth > 0 then
		lstr_minmaxinfo.ptMaxTrackSize.lx = il_maxwidth
	else
		lstr_minmaxinfo.ptMaxTrackSize.lx = le_env.ScreenWidth
	end if
	if il_maxheight > 0 then
		lstr_minmaxinfo.ptMaxTrackSize.ly = il_maxheight
	else
		lstr_minmaxinfo.ptMaxTrackSize.ly = le_env.ScreenHeight
	end if
	
	// copy the structure back into memory
	SetMinMaxInfo(minmaxinfo, lstr_minmaxinfo, 40)
end if

//// important: must return 0
Return 0

end event

public subroutine of_makeresizable ();
//resizable response window; picked from Kodigo

ulong lul_style, lul_hMenu, lul_hWnd
CONSTANT long GWL_STYLE           = (-16)
CONSTANT unsignedlong WS_THICKFRAME       = 262144 //00040000L
CONSTANT unsignedlong WS_SYSMENU          = 524288 //00080000L
CONSTANT uint SC_MINIMIZE     = 61472
CONSTANT uint SC_MAXIMIZE     = 61488
CONSTANT uint SC_CLOSE        = 61536
CONSTANT uint SC_RESTORE      = 61728
CONSTANT uint MF_BYCOMMAND = 0

//this.controlmenu = false

lul_hwnd = handle(this)
lul_style = getwindowlong(lul_hwnd, GWL_STYLE)

if lul_style <> 0 then
	if setwindowlong(lul_hwnd, GWL_STYLE, lul_style + WS_THICKFRAME + WS_SYSMENU) <> 0 then
		lul_hMenu = getsystemmenu(lul_hwnd, false)
		if lul_hmenu > 0 then
			// removing the  menu items from the system menu
			DeleteMenu( lul_hMenu, SC_MINIMIZE , MF_BYCOMMAND)
			DeleteMenu( lul_hMenu, SC_MAXIMIZE , MF_BYCOMMAND)
			DeleteMenu( lul_hMenu, SC_RESTORE  , MF_BYCOMMAND)
			// Force immediate menu update
			DrawMenuBar( lul_hWnd )				 
		end if
	end if 
end if

end subroutine

on w_anc_response.create
end on

on w_anc_response.destroy
end on

event open;
if ib_resize_handler then
	inv_resizer = create nv_auto_resizer
	inv_resizer.initialize(this,this)
end if

if ib_resizable then
	this.controlmenu = false
	this.post of_makeresizable()
end if

setredraw(false)
setredraw(true)

end event

event close;
if ib_resize_handler then
	if isvalid(inv_resizer) then destroy inv_resizer
end if

end event

event resize;
if ib_resize_handler then
	this.setredraw(false)
	inv_resizer.event resize(sizetype, width, height)
	this.setredraw(true)
end if

end event

