(set-logic SLIA)
(synth-fun f ((_arg_0 String)) String
  ( (Start String (ntString))
  (argString String (
	_arg_0
  ))
  (conString String (
	" "
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

(constraint (= (f "UC Berkeley|Berkeley, CA") "UC Berkeley, Berkeley, CA"))
(constraint (= (f "University of Pennsylvania|Phialdelphia, PA, USA") "University of Pennsylvania, Phialdelphia, PA, USA"))
(constraint (= (f "UCLA|Los Angeles, CA") "UCLA, Los Angeles, CA"))
(constraint (= (f "Cornell University|Ithaca, New York, USA") "Cornell University, Ithaca, New York, USA"))
(constraint (= (f "Penn|Philadelphia, PA, USA") "Penn, Philadelphia, PA, USA"))
(constraint (= (f "University of Michigan|Ann Arbor, MI, USA") "University of Michigan, Ann Arbor, MI, USA"))
(constraint (= (f "UC Berkeley|Berkeley, CA") "UC Berkeley, Berkeley, CA"))
(constraint (= (f "MIT|Cambridge, MA") "MIT, Cambridge, MA"))
(constraint (= (f "University of Pennsylvania|Phialdelphia, PA, USA") "University of Pennsylvania, Phialdelphia, PA, USA"))
(constraint (= (f "UCLA|Los Angeles, CA") "UCLA, Los Angeles, CA"))
(constraint (= (f "University of Maryland College Park|College Park, MD") "University of Maryland College Park, College Park, MD"))
(constraint (= (f "University of Michigan|Ann Arbor, MI, USA") "University of Michigan, Ann Arbor, MI, USA"))
(constraint (= (f "UC Berkeley|Berkeley, CA") "UC Berkeley, Berkeley, CA"))
(constraint (= (f "MIT|Cambridge, MA") "MIT, Cambridge, MA"))
(constraint (= (f "Rice University|Houston, TX") "Rice University, Houston, TX"))
(constraint (= (f "Yale University|New Haven, CT, USA") "Yale University, New Haven, CT, USA"))
(constraint (= (f "Columbia University|New York, NY, USA") "Columbia University, New York, NY, USA"))
(constraint (= (f "NYU|New York, New York, USA") "NYU, New York, New York, USA"))
(constraint (= (f "Drexel University|Philadelphia, PA") "Drexel University, Philadelphia, PA"))
(constraint (= (f "UC Berkeley|Berkeley, CA") "UC Berkeley, Berkeley, CA"))
(constraint (= (f "UIUC|Urbana, IL") "UIUC, Urbana, IL"))
(constraint (= (f "Temple University|Philadelphia, PA") "Temple University, Philadelphia, PA"))
(constraint (= (f "Harvard University|Cambridge, MA, USA") "Harvard University, Cambridge, MA, USA"))
(constraint (= (f "University of Connecticut|Storrs, CT, USA") "University of Connecticut, Storrs, CT, USA"))
(constraint (= (f "Drexel University|Philadelphia, PA") "Drexel University, Philadelphia, PA"))
(constraint (= (f "NYU|New York, New York, USA") "NYU, New York, New York, USA"))
(constraint (= (f "UIUC|Urbana, IL") "UIUC, Urbana, IL"))
(constraint (= (f "New Haven University|New Haven, CT, USA") "New Haven University, New Haven, CT, USA"))
(constraint (= (f "University of California, Santa Barbara|Santa Barbara, CA, USA") "University of California, Santa Barbara, Santa Barbara, CA, USA"))
(constraint (= (f "University of Connecticut|Storrs, CT, USA") "University of Connecticut, Storrs, CT, USA"))
(check-synth)
