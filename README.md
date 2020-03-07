## Overview
This piece of code shows basic communication via low-level I2C libraries. As an example control of PCA9685 motor controller is handeled.

## Sample Structure
This sample contains two subfolders, HighLevelCore and RealTimeCore, each containing an app that deploys to that specific core on the board. Currently only HighLevelCore is in use.

These apps are designed so that you can either open this folder in Visual Studio to work on both apps at the same time,
or open one of the subfolders to just work on that particular app.

## Build and Debug
To build and debug application you need to install Visual Studio 2019 and follow Microsoft's docs dor AzureSphere SDK install.
After installing everything, open root folder of this project and build HLCore.
