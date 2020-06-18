
CXXFLAGS=-g -O2 -Wall
LDFLAGS=

all: parseasm run bnf

packed_int.o: packed_int.h packed_int.cc

vm.o: vm.h vm.cc

parseasm.o: parseasm.cc

run.o: run.cc

xml.o : xml.cc

turtle.o: turtle.cc

parseasm: parseasm.o vm.o packed_int.o
	$(CXX) -o $@ $+ $(LDFLAGS)

run: run.o vm.o packed_int.o
	$(CXX) -o $@ $+ $(LDFLAGS)
	
bnf: bnf.o vm.o packed_int.o
	$(CXX) -o $@ $+ $(LDFLAGS)

xml: xml.o vm.o packed_int.o
	$(CXX) -o $@ $+ $(LDFLAGS)
	
turtle: turtle.o vm.o packed_int.o
	$(CXX) -o $@ $+ $(LDFLAGS)
