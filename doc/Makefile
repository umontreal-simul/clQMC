.PHONY: all html latex install clean


all: html latex
	
html:
	doxygen clQMC.doxy

latex: html
	$(MAKE) -C out/latex

install: html latex
	mkdir -p "$(PREFIX)/doc"
	install -d out/html "$(PREFIX)/doc"
	install --mode=0644 out/latex/refman.pdf "$(PREFIX)/doc"

clean:
	rm -rf out/html out/latex
