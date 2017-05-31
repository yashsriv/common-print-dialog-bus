all: client-rule server-rule

server-rule: server/server.cpp common-rule
	g++ -o server-prog server/server.cpp common-lib `pkg-config --libs --cflags gio-2.0`

client-rule: client/client.cpp common-rule
	g++ -o client-prog client/client.cpp common-lib `pkg-config --libs --cflags gio-2.0`

common-rule: common/common.cpp
	g++ -o common-lib -c common/common.cpp
