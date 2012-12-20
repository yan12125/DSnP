REFPKGS  = cmd
SRCPKGS  = util cir sat
LIBPKGS  = $(REFPKGS) $(SRCPKGS)
MAIN     = main

LIBS     = $(addprefix -l, $(LIBPKGS))
SRCLIBS  = $(addsuffix .a, $(addprefix lib, $(SRCPKGS)))

EXEC     = fraig

all: libs main

libs:
	@for pkg in $(SRCPKGS); \
	do \
		echo "Checking $$pkg..."; \
		cd src/$$pkg; make --no-print-directory PKGNAME=$$pkg; \
		cd ../..; \
	done

main:
	@echo "Checking $(MAIN)..."
	@cd src/$(MAIN); \
		make --no-print-directory INCLIB="$(LIBS)" EXEC=$(EXEC);
	@ln -fs bin/$(EXEC) .
#	@strip bin/$(EXEC)

clean:
	@for pkg in $(SRCPKGS); \
	do \
		echo "Cleaning $$pkg..."; \
		cd src/$$pkg; make --no-print-directory PKGNAME=$$pkg clean; \
                cd ../..; \
	done
	@echo "Cleaning $(MAIN)..."
	@cd src/$(MAIN); make --no-print-directory clean
	@echo "Removing $(SRCLIBS)..."
	@cd lib; rm -f $(SRCLIBS)
	@echo "Removing $(EXEC)..."
	@rm -f bin/$(EXEC)

cleanall: clean
	@echo "Removing bin/*..."
	@rm -f bin/*

ctags:	  
	@rm -f src/tags
	@for pkg in $(SRCPKGS) main; \
	do \
		echo "Tagging $$pkg..."; \
		cd src; ctags -a $$pkg/*.cpp $$pkg/*.h; cd ..; \
	done
	@echo "Tagging $(MAIN)..."
	@cd src; ctags -a $(MAIN)/*.cpp $(MAIN)/*.h

32:
	@for pkg in $(REFPKGS); \
	do \
	        cd lib; ln -sf lib$$pkg-32.a lib$$pkg.a; cd ../..; \
	done
	@cd ref; ln -sf $(EXEC)-32 $(EXEC);

64:
	@for pkg in $(REFPKGS); \
	do \
	        cd lib; ln -sf lib$$pkg-64.a lib$$pkg.a; cd ../..; \
	done
	@cd ref; ln -sf $(EXEC)-64 $(EXEC);
