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
(constraint (= (f "Lara|Constable") "Constable, L."))
(constraint (= (f "Madelaine|Ghoston") "Ghoston, M."))
(constraint (= (f "Salley|Hornak") "Hornak, S."))
(constraint (= (f "Micha|Junkin") "Junkin, M."))
(constraint (= (f "Teddy|Bobo") "Bobo, T."))
(constraint (= (f "Coralee|Scalia") "Scalia, C."))
(constraint (= (f "Jeff|Quashie") "Quashie, J."))
(constraint (= (f "Vena|Babiarz") "Babiarz, V."))
(constraint (= (f "Karrie|Lain") "Lain, K."))
(constraint (= (f "Tobias|Dermody") "Dermody, T."))
(constraint (= (f "Celsa|Hopkins") "Hopkins, C."))
(constraint (= (f "Kimberley|Halpern") "Halpern, K."))
(constraint (= (f "Phillip|Rowden") "Rowden, P."))
(constraint (= (f "Elias|Neil") "Neil, E."))
(constraint (= (f "Lashanda|Cortes") "Cortes, L."))
(constraint (= (f "Mackenzie|Spell") "Spell, M."))
(constraint (= (f "Kathlyn|Eccleston") "Eccleston, K."))
(constraint (= (f "Georgina|Brescia") "Brescia, G."))
(constraint (= (f "Beata|Miah") "Miah, B."))
(constraint (= (f "Desiree|Seamons") "Seamons, D."))
(constraint (= (f "Jeanice|Soderstrom") "Soderstrom, J."))
(constraint (= (f "Mariel|Jurgens") "Jurgens, M."))
(constraint (= (f "Alida|Bogle") "Bogle, A."))
(constraint (= (f "Jacqualine|Olague") "Olague, J."))
(constraint (= (f "Joaquin|Clasen") "Clasen, J."))
(constraint (= (f "Samuel|Richert") "Richert, S."))
(constraint (= (f "Malissa|Marcus") "Marcus, M."))
(constraint (= (f "Alaina|Partida") "Partida, A."))
(constraint (= (f "Trinidad|Mulloy") "Mulloy, T."))
(constraint (= (f "Carlene|Garrard") "Garrard, C."))
(constraint (= (f "Melodi|Chism") "Chism, M."))
(constraint (= (f "Bess|Chilcott") "Chilcott, B."))
(constraint (= (f "Chong|Aylward") "Aylward, C."))
(constraint (= (f "Jani|Ramthun") "Ramthun, J."))
(constraint (= (f "Jacquiline|Heintz") "Heintz, J."))
(constraint (= (f "Hayley|Marquess") "Marquess, H."))
(constraint (= (f "Andria|Spagnoli") "Spagnoli, A."))
(constraint (= (f "Irwin|Covelli") "Covelli, I."))
(constraint (= (f "Gertude|Montiel") "Montiel, G."))
(constraint (= (f "Stefany|Reily") "Reily, S."))
(constraint (= (f "Rae|Mcgaughey") "Mcgaughey, R."))
(constraint (= (f "Cruz|Latimore") "Latimore, C."))
(constraint (= (f "Maryann|Casler") "Casler, M."))
(constraint (= (f "Annalisa|Gregori") "Gregori, A."))
(constraint (= (f "Jenee|Pannell") "Pannell, J."))
(check-synth)
