## fractals directory
A C++/sfml/tgui/CUDA GUI framework to display fractals. Features:
* Detects number of cores and uses all of them.
* Detects CUDA device and uses it 
* Fractal status and selection GUI
* Mouse and Keyboard and GUI Controls
* GUI settable fractal parameters:power and zconst
* Screenshot hotkey and hide all widgets hotkey
* CUDA on/off toggle 
* Mandelbrot (zoom and pan via mouse) Threaded.
* Julia (zoom and pan via mouse) Threaded.
* Buddhabrot(Nebulabrot). Threaded and CUDA optimized(no settable power support for cuda). Will run threads on all the cores to generate the image. To generate the image needs a lot of CPU. The threads have been optimized to generate the image very fast.
On my AMD 16 core machine, the full 16 threads on all cores version is about twice as fast as CUDA and no threads.
CUDA programming is very finicky and there is probably lots of room for improvement.
Nevertheless, its a massive improvement over one AMD thread doing the work.
<p align="center">
<img src="nebulabrot.png" width="400" height="266">
<img src="mandelbrot.png" width="400" height="266">
<img src="julia.png" width="400" height="266">
</p>