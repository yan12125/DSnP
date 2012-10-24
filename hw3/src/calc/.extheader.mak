calc.d: ../../include/calcModNum.h 
../../include/calcModNum.h: calcModNum.h
	@rm -f ../../include/calcModNum.h
	@ln -fs ../src/calc/calcModNum.h ../../include/calcModNum.h
