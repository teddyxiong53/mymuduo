ccsrc = $(wildcard muduo/base/*.cpp) \
		$(wildcard muduo/net/*.cpp)

ccsrc += main.cpp

obj = $(ccsrc:.cpp=.o)
dep = $(obj:.o=.d)

CXX=g++
CPP=g++

LDFLAGS = -lpthread

TARGET = test

CXXFLAGS += -std=c++11 -I./

$(TARGET): $(obj)
	$(CXX) -o $@ $^ $(LDFLAGS)

-include $(dep)


%.d:%.cpp
	@$(CPP) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@


.PHONY: clean cleandep distclean

distclean:clean cleandep

clean:
	rm -f $(obj) $(TARGET)

cleandep:
	rm -rf $(dep)

