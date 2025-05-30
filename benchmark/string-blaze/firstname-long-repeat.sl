(set-logic SLIA)
(synth-fun f ((_arg_0 String)) String
  ( (Start String (ntString))
  (argString String (
	_arg_0
  ))
  (conString String (
	" "
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

(constraint (= (f "Nancy FreeHafer") "Nancy"))
(constraint (= (f "Andrew Cencici") "Andrew"))
(constraint (= (f "Jan Kotas") "Jan"))
(constraint (= (f "Mariya Sergienko") "Mariya"))
(constraint (= (f "Launa Withers") "Launa"))
(constraint (= (f "Launa Withers") "Launa"))
(constraint (= (f "Launa Withers") "Launa"))
(constraint (= (f "Lakenya Edison") "Lakenya"))
(constraint (= (f "Lakenya Edison") "Lakenya"))
(constraint (= (f "Lakenya Edison") "Lakenya"))
(constraint (= (f "Brendan Hage") "Brendan"))
(constraint (= (f "Brendan Hage") "Brendan"))
(constraint (= (f "Brendan Hage") "Brendan"))
(constraint (= (f "Bradford Lango") "Bradford"))
(constraint (= (f "Bradford Lango") "Bradford"))
(constraint (= (f "Bradford Lango") "Bradford"))
(constraint (= (f "Rudolf Akiyama") "Rudolf"))
(constraint (= (f "Rudolf Akiyama") "Rudolf"))
(constraint (= (f "Rudolf Akiyama") "Rudolf"))
(constraint (= (f "Lara Constable") "Lara"))
(constraint (= (f "Lara Constable") "Lara"))
(constraint (= (f "Lara Constable") "Lara"))
(constraint (= (f "Madelaine Ghoston") "Madelaine"))
(constraint (= (f "Madelaine Ghoston") "Madelaine"))
(constraint (= (f "Madelaine Ghoston") "Madelaine"))
(constraint (= (f "Salley Hornak") "Salley"))
(constraint (= (f "Salley Hornak") "Salley"))
(constraint (= (f "Salley Hornak") "Salley"))
(constraint (= (f "Micha Junkin") "Micha"))
(constraint (= (f "Micha Junkin") "Micha"))
(constraint (= (f "Micha Junkin") "Micha"))
(constraint (= (f "Teddy Bobo") "Teddy"))
(constraint (= (f "Teddy Bobo") "Teddy"))
(constraint (= (f "Teddy Bobo") "Teddy"))
(constraint (= (f "Coralee Scalia") "Coralee"))
(constraint (= (f "Coralee Scalia") "Coralee"))
(constraint (= (f "Coralee Scalia") "Coralee"))
(constraint (= (f "Jeff Quashie") "Jeff"))
(constraint (= (f "Jeff Quashie") "Jeff"))
(constraint (= (f "Jeff Quashie") "Jeff"))
(constraint (= (f "Vena Babiarz") "Vena"))
(constraint (= (f "Vena Babiarz") "Vena"))
(constraint (= (f "Vena Babiarz") "Vena"))
(constraint (= (f "Karrie Lain") "Karrie"))
(constraint (= (f "Karrie Lain") "Karrie"))
(constraint (= (f "Karrie Lain") "Karrie"))
(constraint (= (f "Tobias Dermody") "Tobias"))
(constraint (= (f "Tobias Dermody") "Tobias"))
(constraint (= (f "Tobias Dermody") "Tobias"))
(constraint (= (f "Celsa Hopkins") "Celsa"))
(constraint (= (f "Celsa Hopkins") "Celsa"))
(constraint (= (f "Celsa Hopkins") "Celsa"))
(constraint (= (f "Kimberley Halpern") "Kimberley"))
(constraint (= (f "Kimberley Halpern") "Kimberley"))
(constraint (= (f "Kimberley Halpern") "Kimberley"))
(constraint (= (f "Phillip Rowden") "Phillip"))
(constraint (= (f "Phillip Rowden") "Phillip"))
(constraint (= (f "Phillip Rowden") "Phillip"))
(constraint (= (f "Elias Neil") "Elias"))
(constraint (= (f "Elias Neil") "Elias"))
(constraint (= (f "Elias Neil") "Elias"))
(constraint (= (f "Lashanda Cortes") "Lashanda"))
(constraint (= (f "Lashanda Cortes") "Lashanda"))
(constraint (= (f "Lashanda Cortes") "Lashanda"))
(constraint (= (f "Mackenzie Spell") "Mackenzie"))
(constraint (= (f "Mackenzie Spell") "Mackenzie"))
(constraint (= (f "Mackenzie Spell") "Mackenzie"))
(constraint (= (f "Kathlyn Eccleston") "Kathlyn"))
(constraint (= (f "Kathlyn Eccleston") "Kathlyn"))
(constraint (= (f "Kathlyn Eccleston") "Kathlyn"))
(constraint (= (f "Georgina Brescia") "Georgina"))
(constraint (= (f "Georgina Brescia") "Georgina"))
(constraint (= (f "Georgina Brescia") "Georgina"))
(constraint (= (f "Beata Miah") "Beata"))
(constraint (= (f "Beata Miah") "Beata"))
(constraint (= (f "Beata Miah") "Beata"))
(constraint (= (f "Desiree Seamons") "Desiree"))
(constraint (= (f "Desiree Seamons") "Desiree"))
(constraint (= (f "Desiree Seamons") "Desiree"))
(constraint (= (f "Jeanice Soderstrom") "Jeanice"))
(constraint (= (f "Jeanice Soderstrom") "Jeanice"))
(constraint (= (f "Jeanice Soderstrom") "Jeanice"))
(constraint (= (f "Mariel Jurgens") "Mariel"))
(constraint (= (f "Mariel Jurgens") "Mariel"))
(constraint (= (f "Mariel Jurgens") "Mariel"))
(constraint (= (f "Alida Bogle") "Alida"))
(constraint (= (f "Alida Bogle") "Alida"))
(constraint (= (f "Alida Bogle") "Alida"))
(constraint (= (f "Jacqualine Olague") "Jacqualine"))
(constraint (= (f "Jacqualine Olague") "Jacqualine"))
(constraint (= (f "Jacqualine Olague") "Jacqualine"))
(constraint (= (f "Joaquin Clasen") "Joaquin"))
(constraint (= (f "Joaquin Clasen") "Joaquin"))
(constraint (= (f "Joaquin Clasen") "Joaquin"))
(constraint (= (f "Samuel Richert") "Samuel"))
(constraint (= (f "Samuel Richert") "Samuel"))
(constraint (= (f "Samuel Richert") "Samuel"))
(constraint (= (f "Malissa Marcus") "Malissa"))
(constraint (= (f "Malissa Marcus") "Malissa"))
(constraint (= (f "Malissa Marcus") "Malissa"))
(constraint (= (f "Alaina Partida") "Alaina"))
(constraint (= (f "Alaina Partida") "Alaina"))
(constraint (= (f "Alaina Partida") "Alaina"))
(constraint (= (f "Trinidad Mulloy") "Trinidad"))
(constraint (= (f "Trinidad Mulloy") "Trinidad"))
(constraint (= (f "Trinidad Mulloy") "Trinidad"))
(constraint (= (f "Carlene Garrard") "Carlene"))
(constraint (= (f "Carlene Garrard") "Carlene"))
(constraint (= (f "Carlene Garrard") "Carlene"))
(constraint (= (f "Melodi Chism") "Melodi"))
(constraint (= (f "Melodi Chism") "Melodi"))
(constraint (= (f "Melodi Chism") "Melodi"))
(constraint (= (f "Bess Chilcott") "Bess"))
(constraint (= (f "Bess Chilcott") "Bess"))
(constraint (= (f "Bess Chilcott") "Bess"))
(constraint (= (f "Chong Aylward") "Chong"))
(constraint (= (f "Chong Aylward") "Chong"))
(constraint (= (f "Chong Aylward") "Chong"))
(constraint (= (f "Jani Ramthun") "Jani"))
(constraint (= (f "Jani Ramthun") "Jani"))
(constraint (= (f "Jani Ramthun") "Jani"))
(constraint (= (f "Jacquiline Heintz") "Jacquiline"))
(constraint (= (f "Jacquiline Heintz") "Jacquiline"))
(constraint (= (f "Jacquiline Heintz") "Jacquiline"))
(constraint (= (f "Hayley Marquess") "Hayley"))
(constraint (= (f "Hayley Marquess") "Hayley"))
(constraint (= (f "Hayley Marquess") "Hayley"))
(constraint (= (f "Andria Spagnoli") "Andria"))
(constraint (= (f "Andria Spagnoli") "Andria"))
(constraint (= (f "Andria Spagnoli") "Andria"))
(constraint (= (f "Irwin Covelli") "Irwin"))
(constraint (= (f "Irwin Covelli") "Irwin"))
(constraint (= (f "Irwin Covelli") "Irwin"))
(constraint (= (f "Gertude Montiel") "Gertude"))
(constraint (= (f "Gertude Montiel") "Gertude"))
(constraint (= (f "Gertude Montiel") "Gertude"))
(constraint (= (f "Stefany Reily") "Stefany"))
(constraint (= (f "Stefany Reily") "Stefany"))
(constraint (= (f "Stefany Reily") "Stefany"))
(constraint (= (f "Rae Mcgaughey") "Rae"))
(constraint (= (f "Rae Mcgaughey") "Rae"))
(constraint (= (f "Rae Mcgaughey") "Rae"))
(constraint (= (f "Cruz Latimore") "Cruz"))
(constraint (= (f "Cruz Latimore") "Cruz"))
(constraint (= (f "Cruz Latimore") "Cruz"))
(constraint (= (f "Maryann Casler") "Maryann"))
(constraint (= (f "Maryann Casler") "Maryann"))
(constraint (= (f "Maryann Casler") "Maryann"))
(constraint (= (f "Annalisa Gregori") "Annalisa"))
(constraint (= (f "Annalisa Gregori") "Annalisa"))
(constraint (= (f "Annalisa Gregori") "Annalisa"))
(constraint (= (f "Jenee Pannell") "Jenee"))
(constraint (= (f "Jenee Pannell") "Jenee"))
(constraint (= (f "Jenee Pannell") "Jenee"))
(constraint (= (f "Launa Withers") "Launa"))
(constraint (= (f "Lakenya Edison") "Lakenya"))
(constraint (= (f "Brendan Hage") "Brendan"))
(constraint (= (f "Bradford Lango") "Bradford"))
(constraint (= (f "Rudolf Akiyama") "Rudolf"))
(constraint (= (f "Lara Constable") "Lara"))
(constraint (= (f "Madelaine Ghoston") "Madelaine"))
(constraint (= (f "Salley Hornak") "Salley"))
(constraint (= (f "Micha Junkin") "Micha"))
(constraint (= (f "Teddy Bobo") "Teddy"))
(constraint (= (f "Coralee Scalia") "Coralee"))
(constraint (= (f "Jeff Quashie") "Jeff"))
(constraint (= (f "Vena Babiarz") "Vena"))
(constraint (= (f "Karrie Lain") "Karrie"))
(constraint (= (f "Tobias Dermody") "Tobias"))
(constraint (= (f "Celsa Hopkins") "Celsa"))
(constraint (= (f "Kimberley Halpern") "Kimberley"))
(constraint (= (f "Phillip Rowden") "Phillip"))
(constraint (= (f "Elias Neil") "Elias"))
(constraint (= (f "Lashanda Cortes") "Lashanda"))
(constraint (= (f "Mackenzie Spell") "Mackenzie"))
(constraint (= (f "Kathlyn Eccleston") "Kathlyn"))
(constraint (= (f "Georgina Brescia") "Georgina"))
(constraint (= (f "Beata Miah") "Beata"))
(constraint (= (f "Desiree Seamons") "Desiree"))
(constraint (= (f "Jeanice Soderstrom") "Jeanice"))
(constraint (= (f "Mariel Jurgens") "Mariel"))
(constraint (= (f "Alida Bogle") "Alida"))
(constraint (= (f "Jacqualine Olague") "Jacqualine"))
(constraint (= (f "Joaquin Clasen") "Joaquin"))
(constraint (= (f "Samuel Richert") "Samuel"))
(constraint (= (f "Malissa Marcus") "Malissa"))
(constraint (= (f "Alaina Partida") "Alaina"))
(constraint (= (f "Trinidad Mulloy") "Trinidad"))
(constraint (= (f "Carlene Garrard") "Carlene"))
(constraint (= (f "Melodi Chism") "Melodi"))
(constraint (= (f "Bess Chilcott") "Bess"))
(constraint (= (f "Chong Aylward") "Chong"))
(constraint (= (f "Jani Ramthun") "Jani"))
(constraint (= (f "Jacquiline Heintz") "Jacquiline"))
(constraint (= (f "Hayley Marquess") "Hayley"))
(constraint (= (f "Andria Spagnoli") "Andria"))
(constraint (= (f "Irwin Covelli") "Irwin"))
(constraint (= (f "Gertude Montiel") "Gertude"))
(constraint (= (f "Stefany Reily") "Stefany"))
(constraint (= (f "Rae Mcgaughey") "Rae"))
(constraint (= (f "Cruz Latimore") "Cruz"))
(constraint (= (f "Maryann Casler") "Maryann"))
(constraint (= (f "Annalisa Gregori") "Annalisa"))
(constraint (= (f "Jenee Pannell") "Jenee"))
(check-synth)
