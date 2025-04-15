(set-logic SLIA)
(synth-fun f ((_arg_0 String)) String
  ( (Start String (ntString))
  (argString String (
	_arg_0
  ))
  (conString String (
	" "
	"."
	","
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

(constraint (= (f "Launa|Withers") "Withers, L."))
(constraint (= (f "Lakenya|Edison") "Edison, L."))
(constraint (= (f "Brendan|Hage") "Hage, B."))
(constraint (= (f "Bradford|Lango") "Lango, B."))
(constraint (= (f "Rudolf|Akiyama") "Akiyama, R."))
(check-synth)
