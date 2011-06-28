$PBExportHeader$regexdemo105.sra
$PBExportComments$Generated Application Object
forward
global type regexdemo105 from application
end type
global transaction sqlca
global dynamicdescriptionarea sqlda
global dynamicstagingarea sqlsa
global error error
global message message
end forward

global variables
//long GL_DEBUG_MESSAGE = 4


end variables
global type regexdemo105 from application
string appname = "regexdemo105"
end type
global regexdemo105 regexdemo105

on regexdemo105.create
appname="regexdemo105"
message=create message
sqlca=create transaction
sqlda=create dynamicdescriptionarea
sqlsa=create dynamicstagingarea
error=create error
end on

on regexdemo105.destroy
destroy(sqlca)
destroy(sqlda)
destroy(sqlsa)
destroy(error)
destroy(message)
end on

event open;open(w_test)

end event

