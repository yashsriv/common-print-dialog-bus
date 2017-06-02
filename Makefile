all: build docs

docs: CommonPrint.html

CommonPrint.html: docs/CommonPrint.xml docs/org.openprinting.PrintFrontend.ref.xml docs/org.openprinting.PrintBackend.ref.xml
	xmlto xhtml-nochunks -m docs/config.xsl docs/CommonPrint.xml

docs/org.openprinting.PrintFrontend.ref.xml: org.openprinting.PrintFrontend.doc.xml docs/spec-to-docbook.xsl
	xsltproc docs/spec-to-docbook.xsl $< | tail -n +2 > $@

docs/org.openprinting.PrintBackend.ref.xml: org.openprinting.PrintBackend.doc.xml docs/spec-to-docbook.xsl
	xsltproc docs/spec-to-docbook.xsl $< | tail -n +2 > $@

build: client-prog server-prog

server-prog: server/server.c
	gcc -o server-prog server/server.c `pkg-config --libs --cflags gio-2.0 gio-unix-2.0`

client-prog: client/client.c
	gcc -o client-prog client/client.c `pkg-config --libs --cflags gio-2.0 gio-unix-2.0`

.PHONY: clean build docs

clean:
	rm *.o
	rm docs/*.ref.xml
	rm *.html
	rm client-prog
	rm server-prog
