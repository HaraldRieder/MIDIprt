*.sln, *.vcproj are Microsoft Visual C++ 2008 with wxWidgets 2.8
production files.

*.dsw, *.dsp old Microsoft Visual Studio 6.0 with wxWindows 2.4.2 
production files. Probably will not work with current sources
(Deprecated BeginDrawing() and EndDrawing() might need to be 
re-activated).

Requires own library from the my_libs dir.

The WXWIN environment variable must be set to the installation
dir. of wxWidgets (older versions are called wxWindows).