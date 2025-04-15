(set-logic SLIA)
(synth-fun f ((_arg_0 String)) String
  ( (Start String (ntString))
  (argString String (
	_arg_0
  ))
  (conString String (
	" "
	","
	"USA"
	"PA"
	"CT"
	"CA"
	"MD"
	"NY"
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

(constraint (= (f "UC Berkeley|Berkeley, CA") "Berkeley, CA, USA"))
(constraint (= (f "University of Pennsylvania|Phialdelphia, PA, USA") "Phialdelphia, PA, USA"))
(constraint (= (f "UCLA|Los Angeles, CA") "UCLA, Los Angeles, CA, USA"))
(constraint (= (f "Cornell University|Ithaca, New York, USA") "Ithaca, NY, USA"))
(constraint (= (f "Penn|Philadelphia, PA, USA") "Philadelphia, PA, USA"))
(constraint (= (f "University of Michigan|Ann Arbor, MI, USA") "Ann Arbor, MI, USA"))
(constraint (= (f "UC Berkeley|Berkeley, CA") "Berkeley, CA, USA"))
(constraint (= (f "MIT|Cambridge, MA") "Cambridge, MA, USA"))
(constraint (= (f "University of Pennsylvania|Phialdelphia, PA, USA") "Phialdelphia, PA, USA"))
(constraint (= (f "UCLA|Los Angeles, CA") "Los Angeles, CA, USA"))
(constraint (= (f "University of Maryland College Park|College Park, MD") "College Park, MD, USA"))
(constraint (= (f "University of Michigan|Ann Arbor, MI, USA") "Ann Arbor, MI, USA"))
(constraint (= (f "UC Berkeley|Berkeley, CA") "Berkeley, CA, USA"))
(constraint (= (f "MIT|Cambridge, MA") "Cambridge, MA, USA"))
(constraint (= (f "Rice University|Houston, TX") "Houston, TX, USA"))
(constraint (= (f "Yale University|New Haven, CT, USA") "New Haven, CT, USA"))
(constraint (= (f "Columbia University|New York, NY, USA") "New York, NY, USA"))
(constraint (= (f "NYU|New York, New York, USA") "New York, NY, USA"))
(constraint (= (f "Drexel University|Philadelphia, PA") "Philadelphia, PA, USA"))
(constraint (= (f "UC Berkeley|Berkeley, CA") "Berkeley, CA, USA"))
(constraint (= (f "UIUC|Urbana, IL") "Urbana, IL, USA"))
(constraint (= (f "Temple University|Philadelphia, PA") "Philadelphia, PA, USA"))
(constraint (= (f "Harvard University|Cambridge, MA, USA") "Cambridge, MA, USA"))
(constraint (= (f "University of Connecticut|Storrs, CT, USA") "Storrs, CT, USA"))
(constraint (= (f "Drexel University|Philadelphia, PA") "DPhiladelphia, PA, USA"))
(constraint (= (f "NYU|New York, New York, USA") "New York, NY, USA"))
(constraint (= (f "UIUC|Urbana, IL") "Urbana, IL, USA"))
(constraint (= (f "New Haven University|New Haven, CT, USA") "New Haven, CT, USA"))
(constraint (= (f "University of California, Santa Barbara|Santa Barbara, CA, USA") "Santa Barbara, CA, USA"))
(constraint (= (f "University of Connecticut|Storrs, CT, USA") "Storrs, CT, USA"))
(check-synth)
