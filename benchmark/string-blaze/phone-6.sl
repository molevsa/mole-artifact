(set-logic SLIA)
(synth-fun f ((_arg_0 String)) String
  ( (Start String (ntString))
  (argString String (
	_arg_0
  ))
  (conString String (
	" "
	"+"
	"-"
	"."
  ))
  (fullString String (
	conString
	(str.substr argString ntInt ntInt)
  ))
  (regex String (
	"ProperCase"
	"CAPS"
	"lowercase"
	"Digits"
	"Alphabets"
	"Alphanumeric"
	"WhiteSpace"
	"StartT"
	"EndT"
	"ProperCaseWSpaces"
	"CAPSWSpaces"
	"lowercaseSpaces"
	"AlphabetsWSpaces"
	"Sep"
  ))
  (ntString String (
	fullString
	(str.++ fullString ntString)
  ))
  (direction Int (
	0 1
  ))
  (conPos Int (
	0 1 2 3 4 5 6 7 8 9 10
  ))
  (conMatch Int (
	1 2 3 -1 -2 -3
  ))
  (ntInt Int (
	conPos
	(str.indexof argString regex conMatch direction)
  ))
 )
)

(constraint (= (f "+106 769-858-438") "769"))
(constraint (= (f "+83 973-757-831") "973"))
(constraint (= (f "+62 647-787-775") "647"))
(constraint (= (f "+172 027-507-632") "027"))
(constraint (= (f "+72 001-050-856") "001"))
(constraint (= (f "+95 310-537-401") "310"))
(constraint (= (f "+6 775-969-238") "775"))
(check-synth)
