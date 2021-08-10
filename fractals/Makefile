CC=g++

#try to format all code in:
#clang-format --style=Google

src = $(filter-out graph_class_implementation.cpp fractals_with_gui_cuda.cpp graph_class_pimpl.cpp, $(wildcard *.cpp))
obj = $(patsubst %.cpp,%,$(src))

all: cuda_fractal


.PHONY: all clean

$(obj): %: %.cpp
	$(CC) -g -pthread -std=c++17 -fstack-protector -Wformat -Werror=format-security  -DNDEBUG -g -fwrapv -O3 -Wall -o $@ $< -ltgui -lsfml-graphics -lsfml-window -lsfml-system

cudaversion=11.4
#set cudaversion=11.4
#set path = (/usr/local/cuda-$(cudaversion)/bin $path)
#set LD_LIBRARY_PATH = (/usr/local/cuda-$(cudaversion)/lib64)
#echo 'export PATH=/usr/local/cuda/bin${PATH:+:${PATH}}' >> ~/.bashrc

cuda_fractal:
	nvcc buddha_cuda_kernel.cu -c -o buddha_cuda_kernel.o
	$(CC) -g -pthread -std=c++17 -fstack-protector -Wformat -Werror=format-security  -DNDEBUG -g -fwrapv -O3 -Wall -o fractals_with_gui_cuda buddha_cuda_kernel.o fractals_with_gui_cuda.cpp -L/usr/local/cuda-$(cudaversion)/lib64 -lcuda -lcudart -ltgui -lsfml-graphics -lsfml-window -lsfml-system


clean:
	rm -f $(obj)



