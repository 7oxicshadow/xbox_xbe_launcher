# xbox_xbe_launcher

![Screenshot](/doc/images/xbe_launcher.png)

When placed on the root of an xiso it will list all detected XBE files and provide an option to select which one you wish to launch. This is useful for chihiro games that do not have a default.xbe or for allowing multiple modded xbe files to exist on the iso (60fps, widescreen patched etc...)

## Prerequisites
NXDK will need some additional libraries to be installed. Use the following guide:  
    https://github.com/XboxDev/nxdk/wiki/Install-the-Prerequisites

## Build
1. Clone the repo
2. cd into the xbox_xbe_launcher folder
3. Run build.sh

## Usage
1. Extract the xiso using extract-xiso (32bit version recommended as 64bit can create corrupt files)
2. Rename the default.xbe located in the root of the extracted iso to "_default.xbe". (The auto start timer uses this.)  
   Note: If your game does not have a default.xbe (chihiro) then skip step 2
3. Copy the built default.xbe from the bin folder of this project to the extracted iso folder
4. Rebuild the xiso using extract-xiso with the '-c' option
5. Play


## Notes
Use UP and Down on the DPAD and Button A to select.

