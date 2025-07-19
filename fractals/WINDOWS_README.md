## fractals directory
To build for windows you will need to install: [take the latest versions and fix vcxproj]
* SFML 3.0.0
* TGUI 1.10
* CUDA
* Visual Studio (has CUDA 12.9 C++ project)
<br>
You might need to look at migration guides: https://www.sfml-dev.org/tutorials/3.0/getting-started/migrate/
Set your include directories and library directories correctly.
Define _WINDOWS in the preprocessor.
<br>
You will also need to copy the:
* themes directory
* SFML and TGUI dlls
* escape_image.jpg
into where the windows build is.
<br>
Sample commands on my computer:
* cd C:\Users\xxx\Documents\GitHub\demo\fractals\fractals_cuda
* mkdir x64
* mkdir x64/Release/
* cp -r ../themes x64/Release/
* #cp ../escape_image.jpg x64/Release/
* cp ../../../SFML-3.0.0/bin/*.dll x64/Release/
* cp ../../../TGUI-1.10/bin/*.dll x64/Release/

* cp ../../../SFML-3.0.0/bin/*.dll x64/Debug/
* cp ../../../TGUI-1.10/bin/*.dll x64/Debug/
* [Run Visual Studio]
