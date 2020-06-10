CC=g++

# the code wraps the python integration code in PYTHON_INTEGRATION
# disable that in the code and you can remove python flags in Makefile
PYTHONCFLAGS=-fPIE `python3-config --cflags --embed`
PYTHONLDFLAGS=`python3-config --ldflags --embed`
#gprof needs -pg

#try to format all code in:
#clang-format --style=Google

src = $(filter-out graph_class_implementation.cpp graph_class_pimpl.cpp, $(wildcard *.cpp))
obj = $(patsubst %.cpp,%,$(src))

all: $(obj) graph_class_pimpl


.PHONY: all clean

$(obj): %: %.cpp
	$(CC) -g -pthread -o $@ $<
#	$(CC) -g -pthread $(PYTHONCFLAGS) -o $@ $< $(PYTHONLDFLAGS)

#no main in library
graph_class_pimpl: graph_class_pimpl.cpp graph_class_implementation.cpp graph_class_interface.h
	$(CC) -fstack-protector -Wformat -Werror=format-security -DNDEBUG -g -fwrapv -O3 -Wall -pthread graph_class_implementation.cpp $< -o graph_class_pimpl
#	$(CC) -g -pthread $(PYTHONCFLAGS) graph_class_implementation.cpp $< -o graph_class_pimpl $(PYTHONLDFLAGS)

clean:
	rm -f $(obj) graph_class_pimpl 



