# Memory-Allocation

This was a school project for one of my C programming courses.

This application provides a package for dynamic memory application, as an alternative to the
existing 'malloc' function given by the C language. It contains different allocation options, such
as best fit, first fit, and coalescing. There are also test drivers provided, and the application
can be compared against the performance of the existing 'malloc' function. 

To run the application, run ./lab4 and follow with the flags for which test driver and settings
you would like to use. Further information about each test driver itself can be found in the lab4.c
comments for each driver.

-u x: run with unit driver 'x', for test driver 0 through 4.  
-c: run with coalescing (off by default).  
-f best: run with best fit allocation (runs with first fit by default).  
-s x: random number generator uses seed 'x'.  
-v: verbose (more output information).  
-e: run equilibrium test driver. This driver uses the list.c functions from the previous MP3 project.  
-d: run equilibrium driver with existing 'malloc' function instead of this implementation.  

Below is a brief description of each file in the application. Further information can be found
in the header comment of each file. 

lab4.c: Contains the main loop, and drivers to test the mem.c package for memory allocation.  
mem.c: Provides functions for management of memory, to replace standard 'malloc' and 'free' functions.  
list.c: Provides functions to interact with a two way linked list (for equilibrium driver).  
MP4testscript.c: Test script to automatically run several of the drivers using different settings.  
datatypes.h: Defines struct type to be stored in list for equilibrium driver.  
mem.h: Function prototypes definitions for mem.c.  
list.h: Function prototypes definitions for list.c.  
makefile: Compiles the application.  


