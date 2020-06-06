.PHONY: all help clean chat lib

CXX=g++
CPP=g++
AR=ar


MUDUO_SRC = $(wildcard muduo/base/*.cpp) \
		$(wildcard muduo/net/*.cpp)
MUDUO_OBJ :=$(MUDUO_SRC:.cpp=.o)
MUDUO_DEP :=$(MUDUO_OBJ:.o=.d)

TEST_SRC = main.cpp
TEST_OBJ :=$(TEST_SRC:.cpp=.o)
TEST_DEP :=$(TEST_OBJ:.o=.d)

CHAT_SERVER_SRC = examples/chat/server.cpp
CHAT_SERVER_OBJ :=$(CHAT_SERVER_SRC:.cpp=.o)
CHAT_SERVER_DEP :=$(CHAT_SERVER_OBJ:.o=.d)

CHAT_CLIENT_SRC = examples/chat/client.cpp
CHAT_CLIENT_OBJ :=$(CHAT_CLIENT_SRC:.cpp=.o)
CHAT_CLIENT_DEP :=$(CHAT_CLIENT_OBJ:.o=.d)


default: help

libmuduo.a:$(MUDUO_OBJ)
	$(AR) -r $@ $^
lib: libmuduo.a


ALL_OBJ := $(MUDUO_OBJ) $(TEST_OBJ) $(CHAT_CLIENT_OBJ) $(CHAT_SERVER_OBJ)
ALL_DEP := $(ALL_OBJ:.o=.d)


ALL_EXE := test chat_client chat_server

CXXFLAGS += -g -O0 -std=c++11 -I./  -Wformat=0

LDFLAGS = -L./  -lmuduo -lpthread



help:
	@cat help.txt


test: $(TEST_OBJ) libmuduo.a
	g++ -o $@ $^ $(LDFLAGS)

chat: libmuduo.a chat_server chat_client

chat_server:$(CHAT_SERVER_OBJ)
	g++ -o $@ $^ $(LDFLAGS)

chat_client:$(CHAT_SERVER_OBJ)
	g++ -o $@ $^ $(LDFLAGS)

-include $(ALL_DEP)


%.d:%.cpp
	@$(CPP) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@


.PHONY: clean cleandep distclean

distclean:clean cleandep

clean:
	rm -f $(ALL_OBJ) libmuduo.a
	rm -f $(ALL_EXE)
cleandep:
	rm -rf $(ALL_DEP)

