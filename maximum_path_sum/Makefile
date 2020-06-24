CC=g++

#try to format all code in:
#clang-format --style=Google

src = $(wildcard *.cpp)
obj = $(patsubst %.cpp,%,$(src))

all: $(obj)


.PHONY: all clean

$(obj): %: %.cpp
	$(CC) -g -pthread -o $@ $<

clean:
	rm -f $(obj)



