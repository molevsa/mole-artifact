; https=//exceljet.net/formula/remove-text-by-matching
(set-logic SLIA)
(synth-fun f ((_arg_0 String)) String 
 ((Start String (ntString)) 
 (ntString String (
	_arg_0
	"" " " "-"
	(str.++ ntString ntString) 
	(str.replace ntString ntString ntString) 
	(str.at ntString ntInt)
	(int.to.str ntInt)
	(ite ntBool ntString ntString)
	(str.substr ntString ntInt ntInt)
)) 
 (ntInt Int (
	
	1 0 -1
	(+ ntInt ntInt)
	(- ntInt ntInt)
	(str.len ntString)
	(str.to.int ntString)
	(ite ntBool ntInt ntInt)
	(str.indexof ntString ntString ntInt)
)) 
 (ntBool Bool (
	
	true false
	(= ntInt ntInt)
	(str.prefixof ntString ntString)
	(str.suffixof ntString ntString)
	(str.contains ntString ntString)
)) ))
(constraint (= (f "801-345-1987") "8013451987"))
(constraint (= (f "612-554-2000") "6125542000"))
(check-synth)
(define-fun f_1 ((_arg_0 String)) String (str.replace (str.replace _arg_0 "-" "") "-" ""))
