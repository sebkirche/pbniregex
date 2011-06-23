$PBExportHeader$nv_auto_resizer.sru
$PBExportComments$Resize handler that can move / resize objects in a container during resizing - Author GeNi
forward
global type nv_auto_resizer from nonvisualobject
end type
end forward

global type nv_auto_resizer from nonvisualobject
event type long resize ( unsignedlong sizetype,  integer newwidth,  integer newheight )
end type
global nv_auto_resizer nv_auto_resizer

type variables
powerobject ipo_object
nv_auto_resizer ir_registered_controls[]
nv_auto_resizer inv_parent_resizer

boolean ib_anchor_left
boolean ib_anchor_right
boolean ib_anchor_top
boolean ib_anchor_bottom

long il_anchored_left
long il_anchored_right
long il_anchored_top
long il_anchored_bottom
end variables

forward prototypes
public function integer of_scanobject (powerobject apo_obj)
public function long of_get_x (powerobject apo_obj)
public function string of_datatypeof (powerobject apo_object)
public function string of_get_tag (powerobject apo_obj)
public function boolean of_register_object (powerobject apo_obj)
public function boolean of_isvisual (powerobject apo_object)
public function long of_get_y (powerobject apo_obj)
public function long of_get_width (powerobject apo_obj)
public function long of_get_height (powerobject apo_obj)
public subroutine of_set_tag (powerobject apo_obj, string as_newtag)
public subroutine of_set_y (powerobject apo_obj, long al_y)
public function integer initialize (powerobject apo_obj, powerobject apo_parent)
public subroutine of_set_x (powerobject apo_obj, long al_x)
public subroutine of_set_width (powerobject apo_obj, long al_w)
public subroutine of_set_height (powerobject apo_obj, long al_h)
public function string of_datatypeofcontainer (powerobject apo_object)
public function powerobject of_findobject (powerobject ado_obj)
public subroutine of_width_changed (long al_newwidth)
public subroutine of_x_changed (long al_newx, long al_newwidth, boolean ab_toleft)
public subroutine of_y_changed (long al_newy, long al_newheight, boolean ab_totop)
end prototypes

event type long resize(unsignedlong sizetype, integer newwidth, integer newheight);if newwidth = 0 and newheight = 0 then return 0
//debug_message( classname(), "Resize : " + string( newwidth ) + " x " + string( newheight ) + " - " + string( sizetype ) )
//Now we can do the main Engine here !
long ll_x, ll_y, ll_width, ll_height
ll_x = of_get_x( ipo_object )
ll_y = of_get_y( ipo_object )
ll_width = of_get_width( ipo_object )
ll_height = of_get_height( ipo_object )

if ib_anchor_bottom and not ib_anchor_top then 
	of_set_y( ipo_object, newheight - il_anchored_bottom - ll_height )
end if

if ib_anchor_right and not ib_anchor_left then
	of_set_x( ipo_object, newwidth - il_anchored_right - ll_width )	
end if

if ib_anchor_left and ib_anchor_right then
	of_set_width( ipo_object, newwidth - il_anchored_right - il_anchored_left )
end if

if ib_anchor_top and ib_anchor_bottom  then
	of_set_height( ipo_object, newheight - il_anchored_top - il_anchored_bottom )
end if

//Here we ask to all of our child to be resized as we are !
long i
for i = 1 to upperbound( ir_registered_controls[] )
	ir_registered_controls[i].event resize( sizetype, of_get_width( ipo_object ), of_get_height( ipo_object ) )
next
return 0
end event

public function integer of_scanobject (powerobject apo_obj);//this scan the current powerobject for it's children...

if isnull( apo_obj ) or not isvalid( apo_obj ) then return -1

long i

//debug_message( classname(), "DataTypeOf = " + of_datatypeof( apo_obj ) )
choose case of_datatypeofcontainer( apo_obj )
	case "window"
		window l_window
		l_window = apo_obj
		for i = 1 to upperbound( l_window.control[] ) 
			of_register_object( l_window.control[i] )
		next
	case "tab"
		tab l_tab
		l_tab = apo_obj
		for i = 1 to upperbound( l_tab.control[] ) 
			of_register_object( l_tab.control[i] )
		next
	case "userobject"
		userobject l_userobject
		l_userobject = apo_obj
		for i = 1 to upperbound( l_userobject.control[] ) 
			of_register_object( l_userobject.control[i] )
		next		
	case "dragobject", "roundrectangle", "rectangle", "oval", "line"
		return 0
	case else
		debug_message( classname(), "of_scanobject : unhandled datatype = " +  of_datatypeofcontainer( apo_obj ) )
end choose
		
//choisi les types convenables et regardes leur TAG,
//si le tag est compatible, on l'envois dans un register...
//of_register_object( apo_obj )

return 0
end function

public function long of_get_x (powerobject apo_obj);//return the x value of the currenct object
long ll_x
choose case of_datatypeof( apo_obj )
	case "window"
		window l_window
		l_window = apo_obj
		ll_x = l_window.x
	case "dragobject"
		dragobject l_object
		l_object = apo_obj
		ll_x = l_object.x
	case "roundrectangle"
		roundrectangle l_rr
		l_rr = apo_obj
		ll_x = l_rr.x		
	case else
		debug_message( classname(), "of_get_x :  type not handled = " + of_datatypeof( apo_obj ) )
end choose

return ll_x
end function

public function string of_datatypeof (powerobject apo_object);classdefinition lcd_classdef
lcd_classdef = apo_object.classdefinition
string ls_datatype
choose case lcd_classdef.datatypeof
	case "commandbutton", "vprogressbar", "vtrackbar", "statichyperlink", "singlelineedit", "statictext", &
		"picturebutton", "checkbox", "datawindow", "radiobutton", "inkedit", "inkpicture", "animation", &
		"groupbox", "datepicker", "olecustomcontrol", "treeview", "listview", "graph", "monthcalendar", &
		"listbox", "dropdownlistbox", "picturelistbox", "dropdownpicturelistbox", "picture", "picturehyperlink",&
		"hscrollbar", "vscrollbar", "hprogressbar", "htrackbar", "editmask", "richtextedit", "multilineedit"
		ls_datatype = "dragobject"		
	case "tab", "userobject"
		ls_datatype = "dragobject"
	case else
		ls_datatype = lcd_classdef.datatypeof
end choose
return ls_datatype
end function

public function string of_get_tag (powerobject apo_obj);//return the tag value of the currenct object
string ls_tag
choose case of_datatypeof( apo_obj )
	case "window"
		window l_window
		l_window = apo_obj
		ls_tag = l_window.tag			
	case "dragobject"
		dragobject l_object
		l_object = apo_obj
		ls_tag = l_object.tag
	case "roundrectangle"
		roundrectangle l_rr
		l_rr = apo_obj
		ls_tag = l_rr.tag		
	case else
		debug_message( classname(), "of_get_tag :  type not handled = " + of_datatypeof( apo_obj ) )
end choose

return ls_tag
end function

public function boolean of_register_object (powerobject apo_obj);if true /*of_isvisual( apo_obj ) and match( upper(of_get_tag( apo_obj )), "^[LRTB];?.*$" )*/ then
	nv_auto_resizer lnv_resizer
	lnv_resizer = create nv_auto_resizer
	lnv_resizer.initialize( apo_obj, this.ipo_object )
	lnv_resizer.inv_parent_resizer = this
	ir_registered_controls[ upperbound(ir_registered_controls[]) +1 ] = lnv_resizer
	return true
end if

return false
end function

public function boolean of_isvisual (powerobject apo_object);classdefinition lcd_classdef
lcd_classdef = apo_object.classdefinition
return lcd_classdef.isvisualtype
end function

public function long of_get_y (powerobject apo_obj);//return the y value of the currenct object
long ll_y
choose case of_datatypeof( apo_obj )
	case "window"
		window l_window
		l_window = apo_obj
		ll_y = l_window.y
	case "dragobject"
		dragobject l_object
		l_object = apo_obj
		ll_y = l_object.y
	case "roundrectangle"
		roundrectangle l_rr
		l_rr = apo_obj
		ll_y = l_rr.y		
	case else
		debug_message( classname(), "of_get_y :  type not handled = " + of_datatypeof( apo_obj ) )
end choose

return ll_y
end function

public function long of_get_width (powerobject apo_obj);//return the width value of the currenct object
long ll_w
choose case of_datatypeof( apo_obj )
	case "window"
		window l_window
		l_window = apo_obj
		ll_w = l_window.width
	case "dragobject"
		dragobject l_object
		l_object = apo_obj
		ll_w = l_object.width
	case "roundrectangle"
		roundrectangle l_rr
		l_rr = apo_obj
		ll_w = l_rr.width
	case else
		debug_message( classname(), "of_get_width :  type not handled = " + of_datatypeof( apo_obj ) )
end choose

return ll_w
end function

public function long of_get_height (powerobject apo_obj);//return the height value of the currenct object
long ll_h
choose case of_datatypeof( apo_obj )
	case "window"
		window l_window
		l_window = apo_obj
		ll_h = l_window.height
	case "dragobject"
		dragobject l_object
		l_object = apo_obj
		ll_h = l_object.height
	case "roundrectangle"
		roundrectangle l_rr
		l_rr = apo_obj
		ll_h = l_rr.height
		////TODO : add this kind of code for the others methods getters and setters ( x, y, width, tag )
//	case "rectangle"
//		rectangle l_r
//		l_r = apo_obj
//		ll_h = l_r.height
//	case "oval"
//		oval l_oval
//		l_oval = apo_obj
//		ll_h = l_oval.height		
//	case "line"
//		line l_line
//		l_line = apo_obj
//		ll_h = l_line.endy - l_line.beginy
	case else
		debug_message( classname(), "of_get_hieght :  type not handled = " + of_datatypeof( apo_obj ) )
end choose

return ll_h
end function

public subroutine of_set_tag (powerobject apo_obj, string as_newtag);//return the x value of the currenct object
choose case of_datatypeof( apo_obj )
	case "window"
		window l_window
		l_window.tag = as_newtag
	case "dragobject"
		dragobject l_object
		l_object = apo_obj
		l_object.tag = as_newtag
	case "roundrectangle"
		roundrectangle l_rr
		l_rr = apo_obj
		l_rr.tag = as_newtag		
	case else
		debug_message( classname(), "of_set_tag :  type not handled = " + of_datatypeof( apo_obj ) )
end choose

end subroutine

public subroutine of_set_y (powerobject apo_obj, long al_y);choose case of_datatypeof( apo_obj )
	case "window"		
		window l_window
		l_window = apo_obj
		l_window.y = al_y
	case "dragobject"
		dragobject l_object
		l_object = apo_obj
		l_object.y = al_y
	case "roundrectangle"
		roundrectangle l_rr
		l_rr = apo_obj
		l_rr.y = al_y		
	case else
		debug_message( classname(), "of_set_y :  type not handled = " + of_datatypeof( apo_obj ) )
end choose

end subroutine

public function integer initialize (powerobject apo_obj, powerobject apo_parent);//this will parse the full list of control which is comptabile with this service.
ipo_object = apo_obj
long i
string ls_tag
ls_tag = of_get_tag( apo_obj )
i = pos( ls_tag, ";" )
if i > 0 then
	of_set_tag( apo_obj, mid(ls_tag, i+1) )
	ls_tag = 	upper( mid(ls_tag, 1, i -1) )
end if
//debug_message( classname(), "initialize: " + apo_obj.classname() +".tag="+ ls_tag  )
if pos( ls_tag, "L" ) > 0 then 
	ib_anchor_left = true
	il_anchored_left = of_get_x( apo_obj )
//	debug_message( classname(), "initialize: " + apo_obj.classname() +".x="+string(il_anchored_left)  )
end if

if pos( ls_tag, "T" ) > 0 then 
	ib_anchor_top = true
	il_anchored_top = of_get_y( apo_obj )
//	debug_message( classname(), "initialize: " + apo_obj.classname() +".y="+string(il_anchored_top)  )
end if

if pos( ls_tag, "R" ) > 0 then 
	ib_anchor_right = true
	il_anchored_right = of_get_width( apo_parent) - ( of_get_x( apo_obj ) + of_get_width( apo_obj ) )
//	debug_message( classname(), "initialize: " + apo_obj.classname() +".x2="+string(il_anchored_right)  )
end if

if pos( ls_tag, "B" ) > 0 then 
	ib_anchor_bottom = true
	il_anchored_bottom = of_get_height(apo_parent) - ( of_get_height( apo_obj ) + of_get_y( apo_obj ) )
//	debug_message( classname(), "initialize: " + apo_obj.classname() +".y2="+string(il_anchored_bottom )  )
end if

if hasMethod( "setresizer", apo_obj ) then
	apo_obj.dynamic setResizer( this )		//allow the control to respond if it want to move
end if

//regardes si cet object contient d'autres objets
of_scanobject( ipo_object )

return upperbound( ir_registered_controls[] )
end function

public subroutine of_set_x (powerobject apo_obj, long al_x);choose case of_datatypeof( apo_obj )
	case "window"		
		window l_window
		l_window = apo_obj
		l_window.x = al_x
	case "dragobject"
		dragobject l_object
		l_object = apo_obj
		l_object.x = al_x
	case "roundrectangle"
		roundrectangle l_rr
		l_rr = apo_obj
		l_rr.x = al_x		
	case else
		debug_message( classname(), "of_set_x :  type not handled = " + of_datatypeof( apo_obj ) )
end choose

end subroutine

public subroutine of_set_width (powerobject apo_obj, long al_w);choose case of_datatypeof( apo_obj )
	case "window"		
		window l_window
		l_window = apo_obj
		l_window.width = al_w
	case "dragobject"
		dragobject l_object
		l_object = apo_obj
		l_object.width = al_w
	case "roundrectangle"
		roundrectangle l_rr
		l_rr = apo_obj
		l_rr.width = al_w		
	case else
		debug_message( classname(), "of_set_width :  type not handled = " + of_datatypeof( apo_obj ) )
end choose

end subroutine

public subroutine of_set_height (powerobject apo_obj, long al_h);choose case of_datatypeof( apo_obj )
	case "window"		
		window l_window
		l_window = apo_obj
		l_window.height = al_h
	case "dragobject"
		dragobject l_object
		l_object = apo_obj
		l_object.height = al_h
	case "roundrectangle"
		roundrectangle l_rr
		l_rr = apo_obj
		l_rr.height = al_h
	case else
		debug_message( classname(), "of_set_height :  type not handled = " + of_datatypeof( apo_obj ) )
end choose

end subroutine

public function string of_datatypeofcontainer (powerobject apo_object);classdefinition lcd_classdef
lcd_classdef = apo_object.classdefinition
string ls_datatype
choose case lcd_classdef.datatypeof
	case "commandbutton", "vprogressbar", "vtrackbar", "statichyperlink", "singlelineedit", "statictext", &
		"picturebutton", "checkbox", "datawindow", "radiobutton", "inkedit", "inkpicture", "animation", &
		"groupbox", "datepicker", "olecustomcontrol", "treeview", "listview", "graph", "monthcalendar", &
		"listbox", "dropdownlistbox", "picturelistbox", "dropdownpicturelistbox", "picture", "picturehyperlink",&
		"hscrollbar", "vscrollbar", "hprogressbar", "htrackbar", "editmask", "richtextedit", "multilineedit"
		ls_datatype = "dragobject"
	case else
		ls_datatype = lcd_classdef.datatypeof
end choose
return ls_datatype
end function

public function powerobject of_findobject (powerobject ado_obj);//this function look for it's child and return the matching object if any...
powerobject lnv_res
long i
for i = 1 to upperbound( inv_parent_resizer.ir_registered_controls[] )
	if inv_parent_resizer.ir_registered_controls[i].ipo_object = ado_obj then
		return inv_parent_resizer.ir_registered_controls[i]
	end if
next

return lnv_res
end function

public subroutine of_width_changed (long al_newwidth);
end subroutine

public subroutine of_x_changed (long al_newx, long al_newwidth, boolean ab_toleft);if not (ib_anchor_left or ib_anchor_right) then return

if ib_anchor_left and ib_anchor_right then
	if ab_toleft then
		il_anchored_right = il_anchored_left + al_newwidth
	else
		il_anchored_left = al_newx
	end if
else
	if ib_anchor_left then
		if ab_toleft then
			il_anchored_right = il_anchored_left + al_newwidth
		else
			il_anchored_left = al_newx
		end if
	else//		if ib_anchor_left then
		if ab_toleft then
			il_anchored_right = il_anchored_left + al_newwidth
		else
			il_anchored_left = al_newx
		end if
	end if
end if
end subroutine

public subroutine of_y_changed (long al_newy, long al_newheight, boolean ab_totop);if not (ib_anchor_top or ib_anchor_bottom) then return

if ib_anchor_top and ib_anchor_bottom then
	if ab_totop then
		il_anchored_bottom = il_anchored_top + al_newheight
	else
		il_anchored_top = al_newy
	end if
else
	if ib_anchor_top then
		if ab_totop then
			il_anchored_bottom = il_anchored_top + al_newheight
		else
			il_anchored_top = al_newy
		end if
	else//		if ib_anchor_bottom then
		if ab_totop then
			il_anchored_bottom = il_anchored_top + al_newheight
		else
			il_anchored_top = al_newy
		end if
	end if
end if
end subroutine

on nv_auto_resizer.create
call super::create
TriggerEvent( this, "constructor" )
end on

on nv_auto_resizer.destroy
TriggerEvent( this, "destructor" )
call super::destroy
end on

event destructor;long i

for i = 1 to upperbound( ir_registered_controls[] )
	if isvalid( ir_registered_controls[i] ) then destroy ir_registered_controls[i]
next
end event

