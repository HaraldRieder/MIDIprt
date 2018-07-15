This folder contains the sources of my library and of ACS modules.
The library contains various functions that are used by 2 main
applications (MIDI File Printer and MIDI Enhancer) on 4 operating
systems (Atari TOS, 32 Bit Windows, 68K Palm OS >= V3.5, Linux).

acsmodule
---------
Contains sources of ACS modules. ACS modules are plugins that can
be used in any ACS application (ACS >= V2.3), like in the MIDI File
Printer and the MIDI Enhancer. ACS is a GUI builder or even an
integrated development environment for Atari TOS (ACS = application
construction system). It generates C header files *.H and *.AH from 
*.ACS GUI builder files. A description of the modules can be found in 
the WWW.

include
-------
Header files of my library.


include/my_vdi.h
----------------
A vdi.h C header file. It contains a VDI API modified by me.
See the source documentation of the MIDI file printer for
more information why this became necessary.

src
---
Source files of my library. Also the project and make files are there.
On Atari TOS you need Pure C Version 1.1 and the latest (English)
ACS Version 3 to produce the library. On Windows I used Visual C++ 2008.
For Palm OS I didn't build a library. I just used soft links to the 
necessary source and header files to get them linked into the MIDI
Enhancer (Cygwin for Windows lets you do this).

obj
---
Object files generated automatically by the UNIX makefile.

dep
---
Dependency files generated automatically by the UNIX makefile.

Last modified 2008-10-04
Harald Rieder
