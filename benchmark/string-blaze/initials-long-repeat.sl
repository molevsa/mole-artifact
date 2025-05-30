(set-logic SLIA)
(synth-fun f ((_arg_0 String)) String
  ( (Start String (ntString))
  (argString String (
	_arg_0
  ))
  (conString String (
	" "
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

(constraint (= (f "Nancy FreeHafer") "N.F."))
(constraint (= (f "Andrew Cencici") "A.C."))
(constraint (= (f "Jan Kotas") "J.K."))
(constraint (= (f "Mariya Sergienko") "M.S."))
(constraint (= (f "Launa Withers") "L.W."))
(constraint (= (f "Launa Withers") "L.W."))
(constraint (= (f "Launa Withers") "L.W."))
(constraint (= (f "Lakenya Edison") "L.E."))
(constraint (= (f "Lakenya Edison") "L.E."))
(constraint (= (f "Lakenya Edison") "L.E."))
(constraint (= (f "Brendan Hage") "B.H."))
(constraint (= (f "Brendan Hage") "B.H."))
(constraint (= (f "Brendan Hage") "B.H."))
(constraint (= (f "Bradford Lango") "B.L."))
(constraint (= (f "Bradford Lango") "B.L."))
(constraint (= (f "Bradford Lango") "B.L."))
(constraint (= (f "Rudolf Akiyama") "R.A."))
(constraint (= (f "Rudolf Akiyama") "R.A."))
(constraint (= (f "Rudolf Akiyama") "R.A."))
(constraint (= (f "Lara Constable") "L.C."))
(constraint (= (f "Lara Constable") "L.C."))
(constraint (= (f "Lara Constable") "L.C."))
(constraint (= (f "Madelaine Ghoston") "M.G."))
(constraint (= (f "Madelaine Ghoston") "M.G."))
(constraint (= (f "Madelaine Ghoston") "M.G."))
(constraint (= (f "Salley Hornak") "S.H."))
(constraint (= (f "Salley Hornak") "S.H."))
(constraint (= (f "Salley Hornak") "S.H."))
(constraint (= (f "Micha Junkin") "M.J."))
(constraint (= (f "Micha Junkin") "M.J."))
(constraint (= (f "Micha Junkin") "M.J."))
(constraint (= (f "Teddy Bobo") "T.B."))
(constraint (= (f "Teddy Bobo") "T.B."))
(constraint (= (f "Teddy Bobo") "T.B."))
(constraint (= (f "Coralee Scalia") "C.S."))
(constraint (= (f "Coralee Scalia") "C.S."))
(constraint (= (f "Coralee Scalia") "C.S."))
(constraint (= (f "Jeff Quashie") "J.Q."))
(constraint (= (f "Jeff Quashie") "J.Q."))
(constraint (= (f "Jeff Quashie") "J.Q."))
(constraint (= (f "Vena Babiarz") "V.B."))
(constraint (= (f "Vena Babiarz") "V.B."))
(constraint (= (f "Vena Babiarz") "V.B."))
(constraint (= (f "Karrie Lain") "K.L."))
(constraint (= (f "Karrie Lain") "K.L."))
(constraint (= (f "Karrie Lain") "K.L."))
(constraint (= (f "Tobias Dermody") "T.D."))
(constraint (= (f "Tobias Dermody") "T.D."))
(constraint (= (f "Tobias Dermody") "T.D."))
(constraint (= (f "Celsa Hopkins") "C.H."))
(constraint (= (f "Celsa Hopkins") "C.H."))
(constraint (= (f "Celsa Hopkins") "C.H."))
(constraint (= (f "Kimberley Halpern") "K.H."))
(constraint (= (f "Kimberley Halpern") "K.H."))
(constraint (= (f "Kimberley Halpern") "K.H."))
(constraint (= (f "Phillip Rowden") "P.R."))
(constraint (= (f "Phillip Rowden") "P.R."))
(constraint (= (f "Phillip Rowden") "P.R."))
(constraint (= (f "Elias Neil") "E.N."))
(constraint (= (f "Elias Neil") "E.N."))
(constraint (= (f "Elias Neil") "E.N."))
(constraint (= (f "Lashanda Cortes") "L.C."))
(constraint (= (f "Lashanda Cortes") "L.C."))
(constraint (= (f "Lashanda Cortes") "L.C."))
(constraint (= (f "Mackenzie Spell") "M.S."))
(constraint (= (f "Mackenzie Spell") "M.S."))
(constraint (= (f "Mackenzie Spell") "M.S."))
(constraint (= (f "Kathlyn Eccleston") "K.E."))
(constraint (= (f "Kathlyn Eccleston") "K.E."))
(constraint (= (f "Kathlyn Eccleston") "K.E."))
(constraint (= (f "Georgina Brescia") "G.B."))
(constraint (= (f "Georgina Brescia") "G.B."))
(constraint (= (f "Georgina Brescia") "G.B."))
(constraint (= (f "Beata Miah") "B.M."))
(constraint (= (f "Beata Miah") "B.M."))
(constraint (= (f "Beata Miah") "B.M."))
(constraint (= (f "Desiree Seamons") "D.S."))
(constraint (= (f "Desiree Seamons") "D.S."))
(constraint (= (f "Desiree Seamons") "D.S."))
(constraint (= (f "Jeanice Soderstrom") "J.S."))
(constraint (= (f "Jeanice Soderstrom") "J.S."))
(constraint (= (f "Jeanice Soderstrom") "J.S."))
(constraint (= (f "Mariel Jurgens") "M.J."))
(constraint (= (f "Mariel Jurgens") "M.J."))
(constraint (= (f "Mariel Jurgens") "M.J."))
(constraint (= (f "Alida Bogle") "A.B."))
(constraint (= (f "Alida Bogle") "A.B."))
(constraint (= (f "Alida Bogle") "A.B."))
(constraint (= (f "Jacqualine Olague") "J.O."))
(constraint (= (f "Jacqualine Olague") "J.O."))
(constraint (= (f "Jacqualine Olague") "J.O."))
(constraint (= (f "Joaquin Clasen") "J.C."))
(constraint (= (f "Joaquin Clasen") "J.C."))
(constraint (= (f "Joaquin Clasen") "J.C."))
(constraint (= (f "Samuel Richert") "S.R."))
(constraint (= (f "Samuel Richert") "S.R."))
(constraint (= (f "Samuel Richert") "S.R."))
(constraint (= (f "Malissa Marcus") "M.M."))
(constraint (= (f "Malissa Marcus") "M.M."))
(constraint (= (f "Malissa Marcus") "M.M."))
(constraint (= (f "Alaina Partida") "A.P."))
(constraint (= (f "Alaina Partida") "A.P."))
(constraint (= (f "Alaina Partida") "A.P."))
(constraint (= (f "Trinidad Mulloy") "T.M."))
(constraint (= (f "Trinidad Mulloy") "T.M."))
(constraint (= (f "Trinidad Mulloy") "T.M."))
(constraint (= (f "Carlene Garrard") "C.G."))
(constraint (= (f "Carlene Garrard") "C.G."))
(constraint (= (f "Carlene Garrard") "C.G."))
(constraint (= (f "Melodi Chism") "M.C."))
(constraint (= (f "Melodi Chism") "M.C."))
(constraint (= (f "Melodi Chism") "M.C."))
(constraint (= (f "Bess Chilcott") "B.C."))
(constraint (= (f "Bess Chilcott") "B.C."))
(constraint (= (f "Bess Chilcott") "B.C."))
(constraint (= (f "Chong Aylward") "C.A."))
(constraint (= (f "Chong Aylward") "C.A."))
(constraint (= (f "Chong Aylward") "C.A."))
(constraint (= (f "Jani Ramthun") "J.R."))
(constraint (= (f "Jani Ramthun") "J.R."))
(constraint (= (f "Jani Ramthun") "J.R."))
(constraint (= (f "Jacquiline Heintz") "J.H."))
(constraint (= (f "Jacquiline Heintz") "J.H."))
(constraint (= (f "Jacquiline Heintz") "J.H."))
(constraint (= (f "Hayley Marquess") "H.M."))
(constraint (= (f "Hayley Marquess") "H.M."))
(constraint (= (f "Hayley Marquess") "H.M."))
(constraint (= (f "Andria Spagnoli") "A.S."))
(constraint (= (f "Andria Spagnoli") "A.S."))
(constraint (= (f "Andria Spagnoli") "A.S."))
(constraint (= (f "Irwin Covelli") "I.C."))
(constraint (= (f "Irwin Covelli") "I.C."))
(constraint (= (f "Irwin Covelli") "I.C."))
(constraint (= (f "Gertude Montiel") "G.M."))
(constraint (= (f "Gertude Montiel") "G.M."))
(constraint (= (f "Gertude Montiel") "G.M."))
(constraint (= (f "Stefany Reily") "S.R."))
(constraint (= (f "Stefany Reily") "S.R."))
(constraint (= (f "Stefany Reily") "S.R."))
(constraint (= (f "Rae Mcgaughey") "R.M."))
(constraint (= (f "Rae Mcgaughey") "R.M."))
(constraint (= (f "Rae Mcgaughey") "R.M."))
(constraint (= (f "Cruz Latimore") "C.L."))
(constraint (= (f "Cruz Latimore") "C.L."))
(constraint (= (f "Cruz Latimore") "C.L."))
(constraint (= (f "Maryann Casler") "M.C."))
(constraint (= (f "Maryann Casler") "M.C."))
(constraint (= (f "Maryann Casler") "M.C."))
(constraint (= (f "Annalisa Gregori") "A.G."))
(constraint (= (f "Annalisa Gregori") "A.G."))
(constraint (= (f "Annalisa Gregori") "A.G."))
(constraint (= (f "Jenee Pannell") "J.P."))
(constraint (= (f "Jenee Pannell") "J.P."))
(constraint (= (f "Jenee Pannell") "J.P."))
(constraint (= (f "Launa Withers") "L.W."))
(constraint (= (f "Lakenya Edison") "L.E."))
(constraint (= (f "Brendan Hage") "B.H."))
(constraint (= (f "Bradford Lango") "B.L."))
(constraint (= (f "Rudolf Akiyama") "R.A."))
(constraint (= (f "Lara Constable") "L.C."))
(constraint (= (f "Madelaine Ghoston") "M.G."))
(constraint (= (f "Salley Hornak") "S.H."))
(constraint (= (f "Micha Junkin") "M.J."))
(constraint (= (f "Teddy Bobo") "T.B."))
(constraint (= (f "Coralee Scalia") "C.S."))
(constraint (= (f "Jeff Quashie") "J.Q."))
(constraint (= (f "Vena Babiarz") "V.B."))
(constraint (= (f "Karrie Lain") "K.L."))
(constraint (= (f "Tobias Dermody") "T.D."))
(constraint (= (f "Celsa Hopkins") "C.H."))
(constraint (= (f "Kimberley Halpern") "K.H."))
(constraint (= (f "Phillip Rowden") "P.R."))
(constraint (= (f "Elias Neil") "E.N."))
(constraint (= (f "Lashanda Cortes") "L.C."))
(constraint (= (f "Mackenzie Spell") "M.S."))
(constraint (= (f "Kathlyn Eccleston") "K.E."))
(constraint (= (f "Georgina Brescia") "G.B."))
(constraint (= (f "Beata Miah") "B.M."))
(constraint (= (f "Desiree Seamons") "D.S."))
(constraint (= (f "Jeanice Soderstrom") "J.S."))
(constraint (= (f "Mariel Jurgens") "M.J."))
(constraint (= (f "Alida Bogle") "A.B."))
(constraint (= (f "Jacqualine Olague") "J.O."))
(constraint (= (f "Joaquin Clasen") "J.C."))
(constraint (= (f "Samuel Richert") "S.R."))
(constraint (= (f "Malissa Marcus") "M.M."))
(constraint (= (f "Alaina Partida") "A.P."))
(constraint (= (f "Trinidad Mulloy") "T.M."))
(constraint (= (f "Carlene Garrard") "C.G."))
(constraint (= (f "Melodi Chism") "M.C."))
(constraint (= (f "Bess Chilcott") "B.C."))
(constraint (= (f "Chong Aylward") "C.A."))
(constraint (= (f "Jani Ramthun") "J.R."))
(constraint (= (f "Jacquiline Heintz") "J.H."))
(constraint (= (f "Hayley Marquess") "H.M."))
(constraint (= (f "Andria Spagnoli") "A.S."))
(constraint (= (f "Irwin Covelli") "I.C."))
(constraint (= (f "Gertude Montiel") "G.M."))
(constraint (= (f "Stefany Reily") "S.R."))
(constraint (= (f "Rae Mcgaughey") "R.M."))
(constraint (= (f "Cruz Latimore") "C.L."))
(constraint (= (f "Maryann Casler") "M.C."))
(constraint (= (f "Annalisa Gregori") "A.G."))
(constraint (= (f "Jenee Pannell") "J.P."))
(check-synth)
