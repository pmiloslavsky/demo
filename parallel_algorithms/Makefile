CC=g++

#try to format all code in:
#clang-format --style=Google

src = $(wildcard *.cpp)
obj = $(patsubst %.cpp,%,$(src))

all: $(obj)


.PHONY: all clean

$(obj): %: %.cpp
	$(CC) -std=c++17 -g -DNDEBUG -fwrapv -O3 -Wall -pthread -o $@ $< -ltbb

clean:
	rm -f $(obj)



