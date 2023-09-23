## fractals directory
To build for windows you will need to install:
* SFML
* TGUI
* CUDA
* Visual Studio (has CUDA 11.7 C++ project)
<br>
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
* mkdir x64
* mkdir x64/Release/
* cp -r ../themes x64/Release/
* cp ../escape_image.jpg x64/Release/
* cp ../../../SFML-2.6.0/bin/*.dll x64/Release/
* cp ../../../TGUI-1.0/bin/*.dll x64/Release/
* cp ../../../SFML-2.6.0/bin/*.dll x64/Debug/
* cp ../../../TGUI-1.0/bin/*.dll x64/Debug/
* [Run Visual Studio]
