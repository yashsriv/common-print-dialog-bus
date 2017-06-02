all: build docs

docs: CommonPrint.html

CommonPrint.html: docs/CommonPrint.xml docs/org.openprinting.PrintFrontend.ref.xml docs/org.openprinting.PrintBackend.ref.xml
	xmlto xhtml-nochunks -m docs/config.xsl docs/CommonPrint.xml

docs/org.openprinting.PrintFrontend.ref.xml: org.openprinting.PrintFrontend.xml docs/spec-to-docbook.xsl
	xsltproc docs/spec-to-docbook.xsl $< | tail -n +2 > $@

docs/org.openprinting.PrintBackend.ref.xml: org.openprinting.PrintBackend.xml docs/spec-to-docbook.xsl
	xsltproc docs/spec-to-docbook.xsl $< | tail -n +2 > $@

build: common-lib.o client-prog server-prog

server-prog: server/server.cpp common/common.cpp
	g++ -o server-prog server/server.cpp common-lib.o `pkg-config --libs --cflags gio-2.0 gio-unix-2.0`

client-prog: client/client.cpp common/common.cpp
	g++ -o client-prog client/client.cpp common-lib.o `pkg-config --libs --cflags gio-2.0 gio-unix-2.0`

common-lib.o: common/common.cpp
	g++ -o common-lib.o -c common/common.cpp

.PHONY: clean build docs

clean:
	rm *.o
	rm docs/*.ref.xml
	rm *.html
	rm client-prog
	rm server-prog
