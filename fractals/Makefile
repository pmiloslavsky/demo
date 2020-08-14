CC=g++

#try to format all code in:
#clang-format --style=Google

src = $(filter-out graph_class_implementation.cpp graph_class_pimpl.cpp, $(wildcard *.cpp))
obj = $(patsubst %.cpp,%,$(src))

all: $(obj) #graph_class_pimpl


.PHONY: all clean

$(obj): %: %.cpp
	$(CC) -g -pthread -std=c++17 -fstack-protector -Wformat -Werror=format-security  -DNDEBUG -g -fwrapv -O3 -Wall -o $@ $< -ltgui -lsfml-graphics -lsfml-window -lsfml-system

clean:
	rm -f $(obj)



