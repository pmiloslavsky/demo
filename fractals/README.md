## fractals directory
A C++/sfml/tgui/CUDA GUI framework to display/explore fractals. Features:
* Detects number of cores and uses all of them to speed rendering.
* Detects CUDA device and uses it.  CUDA on/off toggle
* Fractal status and selection GUI
* Mouse and Keyboard and GUI Controls
* Mouse: wheel to zoom, right click to recenter pan, left click/hold/drag to select a rectangle and zoom to it.
* GUI settable fractal parameters: power and zconst(julia style)
* GUI selectable color palettes and color cycle size options
* Screenshot hotkey and hide all widgets hotkey
* Crop an area of the fractal (displays border) and it will zoom to crop.
* Mandelbrot (zoom and pan via mouse) Threaded.
* Julia (zoom and pan via mouse) Threaded.
* Spiral Septagon (zoom and pan via mouse) Threaded.
* Buddhabrot(Nebulabrot). Threaded and CUDA optimized(no settable power support for cuda). Will run threads on all the cores to generate the image. To generate the image needs a lot of CPU. The threads have been optimized to generate the image very fast.
On my AMD 16 core machine, the full 16 threads on all cores version is about twice as fast as CUDA and no threads.
CUDA programming is very finicky and there is probably lots of room for improvement.
Nevertheless, its a massive improvement over one AMD thread doing the work.
<p align="center">
<img src="interface_fractal.png" width="800" height="266">
<img src="fractal1.png" width="400" height="266">
<img src="fractal2.png" width="400" height="266">
<img src="fractal3.png" width="400" height="266">
<img src="fractal4.png" width="400" height="266">
<img src="fractal5.png" width="400" height="266">
<img src="fractal6.png" width="400" height="266">
<img src="fractal7.png" width="400" height="266">
<img src="fractal8.png" width="400" height="266">
</p>