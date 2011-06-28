$PBExportHeader$unvo_regexp.sru
$PBExportComments$vbscript.regexp object mapped into a PB user object by Nicolas Georges
forward
global type unvo_regexp from nonvisualobject
end type
end forward

global type unvo_regexp from nonvisualobject
end type
global unvo_regexp unvo_regexp

type variables
/*
 This is the "vbscript.regexp" class implemented in PB !
 
 
 CF : http://msdn2.microsoft.com/en-us/library/ms974570.aspx
 
 
 Allow to test, match and replace patterns...
 
RegExp Object
===============================
string					Pattern
bool					IgnoreCase
bool					Global
--------------------------------------------------------------
bool					Test( string )
string					Replace( string, string )
matchcollection	Execute( string )

MatchCollection Object
===============================
long					Count
Match()				Item
--------------------------------------------------------------

Match Object
===============================
long					FirstIndex 
long					Length
string					Value
--------------------------------------------------------------


Patterns working :
----------------[ Position matching ]----------------------------------------------
^						Only match the beginning of a string.
$						Only match the ending of a string.
\b						Matches any word boundary
\B						Matches any non-word boundary
----------------[ Litterals ]----------------------------------------------
Alphanumeric		Matches alphabetical and numerical characters literally.
\n  					Matches a new line
\f 						Matches a form feed
\r 						Matches carriage return
\t 						Matches horizontal tab
\v 						Matches vertical tab
\? 						Matches ?
\* 						Matches *
\+ 						Matches +
\. 						Matches .
\| 						Matches |
\{ 						Matches {
\} 						Matches }
\\ 						Matches \
\[ 						Matches [
\] 						Matches ]
\( 						Matches (
\) 						Matches )
\xxx 					Matches the ASCII character expressed by the octal number xxx. ( "\50" matches "(" or chr (40). )
\xdd 					Matches the ASCII character expressed by the hex number dd. ( "\x28" matches "(" or chr (40). )
\uxxxx 				Matches the ASCII character expressed by the UNICODE xxxx. ( "\u00A3" matches "£". )
----------------[ Character Classes ]----------------------------------------------
[xyz]  				Match any one character enclosed in the character set. ( "[a-e]" matches "b" in "basketball". )
[^xyz] 				Match any one character not enclosed in the character set. ( "[^a-e]" matches "s" in "basketball". )
.					 	Match any character except \n.
\w 						Match any word character. Equivalent to [a-zA-Z_0-9].
\W 					Match any non-word character. Equivalent to [^a-zA-Z_0-9].
\d 						Match any digit. Equivalent to [0-9].
\D					 	Match any non-digit. Equivalent to [^0-9].
\s					 	Match any space character. Equivalent to [ \t\r\n\v\f].
\S 						Match any non-space character. Equivalent to [^ \t\r\n\v\f].
----------------[ Repetition ]----------------------------------------------
{x}  					Match exactly x occurrences of a regular expression. ( "\d{5}" matches 5 digits. )
{x,} 					Match x or more occurrences of a regular expression. ( "\s{2,}" matches at least 2 space characters. )
{x,y} 					Matches x to y number of occurrences of a regular expression. ( "\d{2,3}" matches at least 2 but no more than 3 digits. )
? 						Match zero or one occurrences. Equivalent to {0,1}. ( "a\s?b" matches "ab" or "a b". )
* 						Match zero or more occurrences. Equivalent to {0,}.
+ 						Match one or more occurrences. Equivalent to {1,}.
----------------[ Alternation & Grouping ]----------------------------------------------
()  						Grouping a clause to create a clause. May be nested. "(ab)?(c)" matches "abc" or "c".
| 						Alternation combines clauses into one regular expression and then matches any of the individual clauses. ( "(ab)|(cd)|(ef)" matches "ab" or "cd" or "ef". )
----------------[ BackReferences ]----------------------------------------------
()\n  					Matches a clause as numbered by the left parenthesis
						"(\w+)\s+\1" matches any word that occurs twice in a row, such as "hubba hubba."
--------------------------------------------------------------
 

 GeNi - summer 2007
	- initial version
 SeKi - 2007-12-04 
 	- addition of the the group() and groupcount() methods to get the content of matched groups
 
*/

private:
	oleobject iole_regexp
	oleobject iole_matchcollection
	
public:
	boolean globalScope = true
	boolean ignoreCase = true
	string searchPattern
	
	
end variables

forward prototypes
public function string replace (string as_searchstring, string as_replacestring)
public function boolean test (string as_teststring)
public function integer search (string as_searchstring)
public subroutine initialize (string as_pattern, boolean ab_globalscope, boolean ab_casesensitive)
public function long matchcount ()
public function string match (long al_index)
public function long matchposition (long al_index)
public function long matchlength (long al_index)
public subroutine initialize ()
public function long groupcount (long al_matchindex)
public function string group (long al_matchindex, long al_groupindex)
end prototypes

public function string replace (string as_searchstring, string as_replacestring);/*
 Replace match in as_searchstring with as_replacestring
 */
string ls_result

initialize( )
ls_result = string( iole_regexp.Replace( as_searchstring , as_replacestring ) )

return ls_result
end function

public function boolean test (string as_teststring);/*
 Return True if the test pattern match, False otherwhise
 */
 boolean ib_result
 
 initialize( )
 ib_result = iole_regexp.Test( as_teststring )
 
return ib_result
end function

public function integer search (string as_searchstring);/*
 This function should be called EXECUTE but, as it is a reserved word in PB, I use Search.
 
Returns	- the number of matchs 
		 	- or -1 if an error come
*/
if isvalid( iole_matchcollection ) then
	destroy iole_matchcollection
end if

initialize( )
iole_matchcollection = iole_regexp.Execute( as_searchstring )
if isvalid( iole_matchcollection ) then
	return ( iole_matchcollection.Count )
end if

return -1
end function

public subroutine initialize (string as_pattern, boolean ab_globalscope, boolean ab_casesensitive);this.globalscope = ab_globalscope
this.ignorecase = not ab_casesensitive
this.searchpattern = as_pattern

initialize( )
end subroutine

public function long matchcount ();long ll_res
if isvalid( iole_matchcollection ) then
	ll_res = long( iole_matchcollection.count )
else
	ll_res = -1
end if

return ll_res
end function

public function string match (long al_index);string ls_result

if isvalid( iole_matchcollection ) and matchcount()>=al_index then
	ls_result = string( iole_matchcollection.Item( al_index - 1 ).value )
else
	setnull( ls_result )
end if

return ls_result
end function

public function long matchposition (long al_index);long ll_result

if isvalid( iole_matchcollection ) and matchcount()>=al_index then
	ll_result = 1 + long( iole_matchcollection.Item( al_index - 1 ).FirstIndex )
else
	ll_result = -1
end if

return ll_result
end function

public function long matchlength (long al_index);long ll_result

if isvalid( iole_matchcollection ) and matchcount()>=al_index then
	ll_result = long( iole_matchcollection.Item( al_index - 1 ).Length )
else
	ll_result = -1
end if

return ll_result
end function

public subroutine initialize ();iole_regexp.Global = globalscope
iole_regexp.IgnoreCase = ignorecase
iole_regexp.Pattern = searchpattern
end subroutine

public function long groupcount (long al_matchindex);long ll_res
if isvalid( iole_matchcollection ) and (matchcount() >= al_matchindex) then
	ll_res = long( iole_matchcollection.item(al_matchindex - 1).submatches.count)
else
	ll_res = -1
end if

return ll_res
end function

public function string group (long al_matchindex, long al_groupindex);string ls_result

if isvalid( iole_matchcollection ) and (matchcount() >= al_matchindex) then
	if (iole_matchcollection.item(al_matchindex - 1).submatches.count >= al_groupindex) then
		ls_result = string( iole_matchcollection.Item( al_matchindex - 1 ).submatches.item(al_groupindex - 1))
	else
		setnull(ls_result)
	end if
else
	setnull( ls_result )
end if

return ls_result
end function

on unvo_regexp.create
call super::create
TriggerEvent( this, "constructor" )
end on

on unvo_regexp.destroy
TriggerEvent( this, "destructor" )
call super::destroy
end on

event constructor;int i
iole_regexp = create oleobject
i = iole_regexp.ConnectToNewObject("vbscript.regexp")
if i <> 0 then
//	Throw create Exception
	MessageBox("unvo_RegExp", "Unable to load this module !")
end if

end event

event destructor;if isvalid( iole_matchcollection ) then
	destroy iole_matchcollection
end if

if isvalid( iole_regexp ) then
	iole_regexp.DisconnectObject ( )
	Destroy iole_regexp
end if
end event

