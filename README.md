VGA VIDEO Output on a STM32F103C8 (vidout)
==========================================

VGA Video output on CORTEX-M3.  

Vidout provides 50 x 18 text output on a STM32F103 CPU (e.g. BluePill) 
using only 24% of the CPU, 1.2K of RAM and 7K of Flash. It's intended 
as a debug aid and should be trivial to port to other CPUs. 

You can see it in action at https://youtu.be/5UFpp3ao460

Pinout;
   PA1 = VSYNC (Pin 14 on VGA connector)
   PA8 = HSYNC (Pin 13 on VGA connector)
   PA7 = Video (Pin 1, 2 or 3 on VGA connector for R, G or B respectively).

Note that the spec calls for VSYNC/HSYNC to be 5V and Video to be 1V max so 
you might want to put a series resistor on PA7. The input impedence of VGA 
is 75R so something in the region of 220R should be fine.

To use this subsystem just make sure you've not got anything on the high priority
interrupt levels (by default this is configured for the highest two, but the SPI
interrupt can have lower priority if you've got other things you need to do).

Just call vidInit to start video output and to get an object back that you can manipulate
to create output.  Both text and graphic output are supported. See the example main
for how to use it, and the API exposed by displayFile too.

In general this should be fire and forget. Once video is up and running it doesn't
need any further maintainence or input from you.

Enjoy

DAVE (dave@marples.net)