<h2> About the Emulator </h2>
It runs a version of FOCAL69 which an actual PDP8 may run. The files containing both FOCAL69 and a version of it which runs with no interrupts (for testing purposes) are courtesy of Dr. Margaret Black.

Lastly, this was a research project, not an attempt to make a real emulator. Therefore, it's quirky at the best of times and may not work as expected.

<h2> Compile on Commandline </h2>

To run, the compiled pdp8 program is already supplied. 
For completeness' sake: the command to compile manually is simple.

``` gcc -o pdp8 pdp8.c ```

<h2> Running a program </h2>

```./pdp8 focal.dump.nointerrupts.raw 0 200```
or
```./pdp focal.dump.bn.ascii 0 200```




