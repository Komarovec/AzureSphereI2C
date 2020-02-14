## Overview
This sample is designed to get you started developing a multi-core application for the MT3620 development board.


## Sample Structure
This sample contains two subfolders, HighLeveCore and RealTimeCore, each containing an app that deploys to that specific core on the board.

These apps are designed so that you can either open this folder in Visual Studio to work on both apps at the same time,
or open one of the subfolders to just work on that particular app.


## Build and Debug
Once Visual Studio has finished generating the CMake cache, use the Build menu to build/rebuild/clean both apps at the same time.

If you want to build just one of the apps, right-click on the CMakeLists.txt of the app you want to build in the Solution window,
and select the Build command there.

To debug, first select the core you want to debug from the Select Startup Item pulldown menu, either GDB Debugger (HLCore),
GDB Debugger (RTCore), or GDB Debugger (All Cores). After selecting, press F5 to deploy to and debug the selected cores.

## Inter-core Communication
To see an example of inter-core communication between High-Level and Real-Time apps, check out the IntercoreComms sample on our GitHub,
https://github.com/Azure/azure-sphere-samples/tree/master/Samples/IntercoreComms
