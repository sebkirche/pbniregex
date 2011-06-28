$PBExportHeader$w_unit_tests.srw
forward
global type w_unit_tests from w_anc_response
end type
type st_results from statictext within w_unit_tests
end type
type cb_encoderesult from commandbutton within w_unit_tests
end type
type pb_delete from picturebutton within w_unit_tests
end type
type pb_insert from picturebutton within w_unit_tests
end type
type cb_load from commandbutton within w_unit_tests
end type
type cb_save from commandbutton within w_unit_tests
end type
type cb_edit from commandbutton within w_unit_tests
end type
type cb_tests_all from commandbutton within w_unit_tests
end type
type cb_close from commandbutton within w_unit_tests
end type
type dw_tests from datawindow within w_unit_tests
end type
end forward

global type w_unit_tests from w_anc_response
integer x = 1500
integer y = 2000
integer width = 3337
integer height = 1476
string title = "Unit tests"
boolean minbox = true
boolean resizable = true
windowtype windowtype = popup!
string icon = "Debug5!"
boolean clientedge = true
boolean palettewindow = true
boolean center = false
boolean ib_resize_handler = true
boolean ib_resizable = false
long il_minwidth = 0
long il_minheight = 0
long il_maxwidth = 0
long il_maxheight = 0
st_results st_results
cb_encoderesult cb_encoderesult
pb_delete pb_delete
pb_insert pb_insert
cb_load cb_load
cb_save cb_save
cb_edit cb_edit
cb_tests_all cb_tests_all
cb_close cb_close
dw_tests dw_tests
end type
global w_unit_tests w_unit_tests

type variables
string is_unittests_file = "regexdemo_unittests.xml"
end variables

forward prototypes
public function boolean of_test (long row)
public subroutine of_run_tests ()
public function boolean of_save ()
public function boolean of_load ()
public function string of_searchresultencode (uo_regex inv_regex, string as_seach_string)
public subroutine of_protect (boolean ab_protect)
end prototypes

public function boolean of_test (long row);//test the row and return the result of test
boolean lb_res = false
uo_regex lnv_regex
lnv_regex = create uo_regex

lnv_regex.initialize( dw_tests.object.pattern[row] , iif( dw_tests.object.sw_g[row]='1', true, false ) , iif( dw_tests.object.sw_i[row]='1', true, false ) )
lnv_regex.setdotmatchnewline( iif( dw_tests.object.sw_s[row]='1', true, false ) )
lnv_regex.setextendedsyntax( iif( dw_tests.object.sw_x[row]='1', true, false ) )
lnv_regex.setmultiline( iif( dw_tests.object.sw_m[row]='1', true, false ))
lnv_regex.setungreedy( iif( dw_tests.object.sw_n[row]='1', true, false ))

choose case upper( dw_tests.object.action[row] )
	case "T"
		if upper(string(lnv_regex.test( dw_tests.object.test_data[row] ))) = upper( dw_tests.object.test_result[row] ) then
			lb_res = true
		else
			lb_res  = false
		end if
	case "S"
		lb_res = ( of_SearchResultEncode( lnv_regex, dw_tests.object.test_data[row] ) = dw_tests.object.test_result[row] )
	case "R"
		lb_res = ( lnv_regex.replace( dw_tests.object.test_data[row], dw_tests.object.test_replace[row]) = dw_tests.object.test_result[row] )
end choose

destroy lnv_regex

return lb_res
end function

public subroutine of_run_tests ();long row
boolean lb_res
long ll_errors = 0
for row = 1 to dw_tests.rowcount( )
	lb_res = of_test( row )
	dw_tests.object.state[ row ] = iif( lb_res, '1', '0' )
	if not lb_res then ll_errors++
next

st_results.text = string( ll_errors ) + " error(s) found on " + string( dw_tests.rowcount( ) ) + " tests."
end subroutine

public function boolean of_save ();//save tests
dw_tests.accepttext( )
if dw_tests.saveas( is_unittests_file , XML!, true) <> 1 then
	Messagebox( title, "Data not saved, an error occured while trying to save rows... Sorry !", StopSign!, Ok! )
	return false
end if

return true
end function

public function boolean of_load ();//reload tests...
dw_tests.reset( )
long ll_res
ll_res = dw_tests.importfile( XML!, is_unittests_file)
if ll_res < 0 then
	MessageBox( title, "Unable to load unit tests file !", StopSign!, Ok! )
	return false
end if

dw_tests.sort( )
dw_tests.resetupdate( )

return true
/*
Long. 
	Returns the number of rows that were imported if it succeeds and one of the following negative integers if an error occurs:

	-1  No rows or startrow value supplied is greater than the number of rows in the file
	-2  Empty file
	-3  Invalid argument
	-4  Invalid input
	-5  Could not open the file
	-6  Could not close the file
	-7  Error reading the text
	-8  Unsupported file name suffix (must be *.txt, *.csv, *.dbf or *.xml)
	-10  Unsupported dBase file format (not version 2 or 3)
	-11  XML Parsing Error; XML parser libraries not found or XML not well formed

	-12  XML Template does not exist or does not match the DataWindow
	-13   Unsupported DataWindow style for import
	-14   Error resolving DataWindow nesting
*/
end function

public function string of_searchresultencode (uo_regex inv_regex, string as_seach_string);string ls_result = ""

long ll_matchcount, ll_groupcount, i, j
ll_matchcount = inv_regex.search( as_seach_string )
ls_result += string( ll_matchcount, "[General];null" ) + " m("
for i = 1 to ll_matchcount
	ls_result += "@" + string( inv_regex.matchposition( i ) , "[General];null" ) + &
				":" + string( inv_regex.matchlength( i ) , "[General];null" ) + " "
	ll_groupcount = inv_regex.groupcount( i )
	ls_result += string( ll_groupcount, "[General];null" ) + " g("
	for j = 1 to ll_groupcount
		ls_result += "@" + string( inv_regex.groupposition( i, j ) , "[General];null" ) + &
					":" + string( inv_regex.grouplength( i, j ) , "[General];null" ) + " "
	next
	ls_result += ") "
next
ls_result += ")"

return ls_result
end function

public subroutine of_protect (boolean ab_protect);long i
string ls_value

if ab_protect then
	ls_value = "1"
else
	ls_value = "0"
end if

for i = 1 to long( dw_tests.describe( "datawindow.column.count" ) )
	dw_tests.modify("#"+string( i ) + ".protect='" + ls_value + "'")
next
end subroutine

on w_unit_tests.create
int iCurrent
call super::create
this.st_results=create st_results
this.cb_encoderesult=create cb_encoderesult
this.pb_delete=create pb_delete
this.pb_insert=create pb_insert
this.cb_load=create cb_load
this.cb_save=create cb_save
this.cb_edit=create cb_edit
this.cb_tests_all=create cb_tests_all
this.cb_close=create cb_close
this.dw_tests=create dw_tests
iCurrent=UpperBound(this.Control)
this.Control[iCurrent+1]=this.st_results
this.Control[iCurrent+2]=this.cb_encoderesult
this.Control[iCurrent+3]=this.pb_delete
this.Control[iCurrent+4]=this.pb_insert
this.Control[iCurrent+5]=this.cb_load
this.Control[iCurrent+6]=this.cb_save
this.Control[iCurrent+7]=this.cb_edit
this.Control[iCurrent+8]=this.cb_tests_all
this.Control[iCurrent+9]=this.cb_close
this.Control[iCurrent+10]=this.dw_tests
end on

on w_unit_tests.destroy
call super::destroy
destroy(this.st_results)
destroy(this.cb_encoderesult)
destroy(this.pb_delete)
destroy(this.pb_insert)
destroy(this.cb_load)
destroy(this.cb_save)
destroy(this.cb_edit)
destroy(this.cb_tests_all)
destroy(this.cb_close)
destroy(this.dw_tests)
end on

event open;call super::open;of_protect( true )
of_load( )
end event

event closequery;call super::closequery;if cb_edit.visible = false then
	if dw_tests.deletedcount( ) + dw_tests.modifiedcount( ) > 0 then
		choose case messagebox( title, "Your unit tests are modified, do you want to save them ?", Question!, YesNoCancel! )
			case 1	//save...
				if not of_save( ) then
					return 1		//save failed !!!
				end if
			case 2	//don't save				
			case 3	//do nothing
				return 1
		end choose
	end if
end if

return 0
end event

type st_results from statictext within w_unit_tests
string tag = "BLR;"
integer x = 411
integer y = 1196
integer width = 1047
integer height = 96
integer textsize = -10
integer weight = 400
fontcharset fontcharset = ansi!
fontpitch fontpitch = variable!
fontfamily fontfamily = swiss!
string facename = "Tahoma"
long textcolor = 33554432
long backcolor = 67108864
boolean border = true
borderstyle borderstyle = stylelowered!
boolean focusrectangle = false
end type

type cb_encoderesult from commandbutton within w_unit_tests
string tag = "BR;"
boolean visible = false
integer x = 1472
integer y = 1188
integer width = 462
integer height = 112
integer taborder = 70
integer textsize = -10
integer weight = 400
fontcharset fontcharset = ansi!
fontpitch fontpitch = variable!
fontfamily fontfamily = swiss!
string facename = "Tahoma"
string text = "Generate result"
end type

event clicked;long row
row = dw_tests.getrow()
if row < 1 then return
dw_tests.accepttext( )
uo_regex lnv_regex
lnv_regex = create uo_regex
lnv_regex.initialize( dw_tests.object.pattern[row] , iif( dw_tests.object.sw_g[row]='1', true, false ) , iif( dw_tests.object.sw_i[row]='1', true, false ) )
lnv_regex.setdotmatchnewline( iif( dw_tests.object.sw_s[row]='1', true, false ) )
lnv_regex.setextendedsyntax( iif( dw_tests.object.sw_x[row]='1', true, false ) )
lnv_regex.setmultiline( iif( dw_tests.object.sw_m[row]='1', true, false ))
lnv_regex.setungreedy( iif( dw_tests.object.sw_n[row]='1', true, false ))
string ls_result
choose case upper( dw_tests.object.action[row] )
	case 'T' ;	ls_result = string( lnv_regex.test( dw_tests.object.test_data[row] )	 )
	case 'S' ;	ls_result = of_searchresultencode( lnv_regex, dw_tests.object.test_data[row] )
	case 'R' ;	ls_result = lnv_regex.replace( dw_tests.object.test_data[row], dw_tests.object.test_replace[row] )
end choose
dw_tests.object.test_result[row] = ls_result

destroy lnv_regex
end event

type pb_delete from picturebutton within w_unit_tests
string tag = "BR;"
boolean visible = false
integer x = 2039
integer y = 1200
integer width = 101
integer height = 88
integer taborder = 80
integer textsize = -10
integer weight = 400
fontcharset fontcharset = ansi!
fontpitch fontpitch = variable!
fontfamily fontfamily = swiss!
string facename = "Tahoma"
string picturename = "DeleteRow!"
alignment htextalign = left!
boolean map3dcolors = true
string powertiptext = "Delete row"
end type

event clicked;if dw_tests.getRow() > 0 then
	dw_tests.deleterow( dw_tests.getRow() )
end if
end event

type pb_insert from picturebutton within w_unit_tests
string tag = "BR;"
boolean visible = false
integer x = 1938
integer y = 1200
integer width = 101
integer height = 88
integer taborder = 70
integer textsize = -10
integer weight = 400
fontcharset fontcharset = ansi!
fontpitch fontpitch = variable!
fontfamily fontfamily = swiss!
string facename = "Tahoma"
boolean originalsize = true
string picturename = "Insert5!"
alignment htextalign = left!
boolean map3dcolors = true
end type

event clicked;long row
row = dw_tests.insertrow( 0 )
dw_tests.setitem( row, "index", row )
end event

type cb_load from commandbutton within w_unit_tests
string tag = "BR;"
boolean visible = false
integer x = 2149
integer y = 1188
integer width = 219
integer height = 112
integer taborder = 60
integer textsize = -10
integer weight = 400
fontcharset fontcharset = ansi!
fontpitch fontpitch = variable!
fontfamily fontfamily = swiss!
string facename = "Tahoma"
string text = "Load"
end type

event clicked;of_load( )

end event

type cb_save from commandbutton within w_unit_tests
string tag = "BR;"
boolean visible = false
integer x = 2373
integer y = 1188
integer width = 219
integer height = 112
integer taborder = 50
integer textsize = -10
integer weight = 400
fontcharset fontcharset = ansi!
fontpitch fontpitch = variable!
fontfamily fontfamily = swiss!
string facename = "Tahoma"
string text = "Save"
end type

event clicked;if of_save( ) then
	//to reset the deleted/modified count...
	of_load( )
end if
end event

type cb_edit from commandbutton within w_unit_tests
string tag = "BR;"
integer x = 2597
integer y = 1188
integer width = 201
integer height = 112
integer taborder = 30
integer textsize = -10
integer weight = 400
fontcharset fontcharset = ansi!
fontpitch fontpitch = variable!
fontfamily fontfamily = swiss!
string facename = "Tahoma"
string text = "Edit"
end type

event clicked;of_protect( false )
cb_save.visible = true
cb_load.visible = true
pb_delete.visible  = true
pb_insert.visible = true
cb_encoderesult.visible = true
this.visible = false
end event

type cb_tests_all from commandbutton within w_unit_tests
string tag = "B;"
integer y = 1188
integer width = 402
integer height = 112
integer taborder = 30
integer textsize = -10
integer weight = 400
fontcharset fontcharset = ansi!
fontpitch fontpitch = variable!
fontfamily fontfamily = swiss!
string facename = "Tahoma"
string text = "Tests all"
boolean default = true
end type

event clicked;dw_tests.accepttext( )
of_run_tests( )
end event

type cb_close from commandbutton within w_unit_tests
string tag = "BR;"
integer x = 2816
integer y = 1188
integer width = 402
integer height = 112
integer taborder = 20
integer textsize = -10
integer weight = 400
fontcharset fontcharset = ansi!
fontpitch fontpitch = variable!
fontfamily fontfamily = swiss!
string facename = "Tahoma"
string text = "Close"
boolean cancel = true
end type

event clicked;close( parent )
end event

type dw_tests from datawindow within w_unit_tests
string tag = "TBLR;"
integer width = 3218
integer height = 1176
integer taborder = 10
string title = "none"
string dataobject = "dw_unittests"
boolean hscrollbar = true
boolean vscrollbar = true
boolean hsplitscroll = true
boolean livescroll = true
borderstyle borderstyle = stylelowered!
end type

