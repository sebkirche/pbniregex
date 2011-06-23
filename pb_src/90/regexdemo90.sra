$PBExportHeader$regexdemo90.sra
$PBExportComments$Generated Application Object
forward
global type regexdemo90 from application
end type
global transaction sqlca
global dynamicdescriptionarea sqlda
global dynamicstagingarea sqlsa
global error error
global message message
end forward

global variables

end variables

global type regexdemo90 from application
string appname = "regexdemo90"
end type
global regexdemo90 regexdemo90

type prototypes
SUBROUTINE OutputDebugString (String lpszOutputString)  LIBRARY "kernel32.dll" ALIAS FOR "OutputDebugStringA;ansi";
FUNCTION integer GetSystemMetrics (int nIndex) LIBRARY "user32.dll"

end prototypes

on regexdemo90.create
appname="regexdemo90"
message=create message
sqlca=create transaction
sqlda=create dynamicdescriptionarea
sqlsa=create dynamicstagingarea
error=create error
end on

on regexdemo90.destroy
destroy(sqlca)
destroy(sqlda)
destroy(sqlsa)
destroy(error)
destroy(message)
end on

event open;
open(w_test)
end event

