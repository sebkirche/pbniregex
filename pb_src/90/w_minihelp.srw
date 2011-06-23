$PBExportHeader$w_minihelp.srw
forward
global type w_minihelp from window
end type
type mle_txt from multilineedit within w_minihelp
end type
end forward

global type w_minihelp from window
integer width = 1568
integer height = 812
boolean titlebar = true
string title = "Quick help"
boolean controlmenu = true
boolean minbox = true
windowtype windowtype = popup!
long backcolor = 67108864
string icon = "Question!"
boolean center = true
mle_txt mle_txt
end type
global w_minihelp w_minihelp

on w_minihelp.create
this.mle_txt=create mle_txt
this.Control[]={this.mle_txt}
end on

on w_minihelp.destroy
destroy(this.mle_txt)
end on

type mle_txt from multilineedit within w_minihelp
integer x = 23
integer y = 20
integer width = 1518
integer height = 692
integer taborder = 10
integer textsize = -8
integer weight = 400
fontcharset fontcharset = ansi!
fontpitch fontpitch = variable!
fontfamily fontfamily = swiss!
string facename = "Tahoma"
long textcolor = 33554432
long backcolor = 67108864
string text = "Without ~"auto update~" option :~r~n1) set a pattern and select some options (global, multiline, ...)~r~n3) Initialize() button~r~n4) you can enter some data to match + test() or search()~r~n   you can also enter a replace string + replace()~r~n5) you just need to Initialize() again only if you change the pattern or an option~r~n~r~nWith the ~"auto update~" option :~r~nThe test(), search() or replace() methods are called automatically on each change of the pattern, data, replace or options.~r~n~r~nYou can copy the result string by double-clicking on it."
boolean border = false
boolean displayonly = true
borderstyle borderstyle = stylelowered!
end type

