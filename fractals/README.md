# fractals directory
A C++/sfml/tgui/CUDA GUI framework to display/explore fractals. Threaded and CUDA optimized. Features:
* Detects number of cores and uses all of them to speed rendering.
* Detects CUDA device and uses it.  CUDA on/off toggle
* Fractal status and selection GUI
* Mouse and Keyboard and GUI Controls
* Mouse: wheel to zoom, right click to recenter pan, left click/hold/drag to select a rectangle and zoom to it.
* GUI settable fractal parameters: power and zconst(julia style) and max iterations
* GUI selectable color palettes and color cycle size options
* GUI palette reflection button to prevent discontinuities
* Other coloring options including interior coloring, shadow maps, image tiling
* Screenshot hotkey and hide all widgets hotkey and pause cpu usage hotkey
* Crop an area of the fractal (displays border) and it will zoom to crop.
* Save and Load fractal key support from file and from memory
* Mandelbrot (zoom and pan via mouse) Threaded.
* Julia (zoom and pan via mouse) Threaded.
* Spiral Septagon (zoom and pan via mouse) Threaded.
* Nova method fractal (zoom and pan via mouse) Threaded.
* Newton method fractal (zoom and pan via mouse) Threaded.
* Anti-buddhabrot with oversampling
* Buddhabrot(Nebulabrot). Threaded and CUDA optimized(no settable power support for cuda). Will run threads on all the cores to generate the image. To generate the image needs a lot of CPU. The threads have been optimized to generate the image very fast.
On my AMD 16 core machine, the full 16 threads on all cores version is about twice as fast as CUDA and no threads.
CUDA programming is very finicky and there is probably lots of room for improvement.
Nevertheless, its a massive improvement over one hw thread doing the work.


This has been ported to Windows and Ubuntu.
<p align="center">
<img src="screenshots/Mandelbrot_30004-09-2022_12_06_24.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30004-09-2022_13_18_20.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30004-09-2022_13_20_56.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30004-09-2022_14_16_05.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30004-09-2022_18_03_54.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30004-09-2022_20_11_29.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30005-09-2022_11_57_08.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30005-09-2022_12_00_41.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30005-09-2022_17_23_49.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30006-09-2022_18_33_18.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30010-09-2022_07_57_20.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30010-09-2022_08_02_25.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30011-09-2022_11_13_13.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30011-09-2022_14_11_13.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30013-09-2022_16_42_43.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30013-09-2022_17_57_50.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30017-09-2022_08_14_45.png" width="400" height="266">
<img src="screenshots/Mandelbrot_30017-09-2022_08_33_27.png" width="400" height="266">
<img src="fractal1.png" width="400" height="266">
<img src="fractal2.png" width="400" height="266">
<img src="fractal3.png" width="400" height="266">
<img src="fractal4.png" width="400" height="266">
<img src="fractal5.png" width="400" height="266">
<img src="fractal6.png" width="400" height="266">
<img src="fractal7.png" width="400" height="266">
<img src="fractal8.png" width="400" height="266">
<img src="interface_fractal1.png" width="800" height="266">
<img src="interface_fractal2.png" width="800" height="266">
<img src="interface_fractal3.png" width="800" height="266">
</p>