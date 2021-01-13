## fractals directory
To build for windows you will need to install:
SFML
TGUI
CUDA
Visual Studio (has CUDA C++ project)

Set your include directories and linrary directories correctly.
Define _WINDOWS in the preprocessor if needed.

You will also need to copy the themes directory and the SFML and TGUI dlls into where the windows build is.
Also escape_image.jpg.

mkdir x64
mkdir x64/Release/
cp -r themes x64/Release/
cp escape_image.jpg x64/Release/
cp ../../SFML-2.5.1/bin/*.dll x64/Release/
cp ../../TGUI-0.8/bin/*.dll x64/Release/
[Visual Studio]
