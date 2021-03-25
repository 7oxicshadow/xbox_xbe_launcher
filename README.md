# xbox_xbe_launcher
When placed on the root of an xiso it will list all detected XBE files and provide an option to select which one you wish to launch. This is useful for chihiro games that do not have a default.xbe or for allowing multiple modded xbe files to exist on the idso (60fps, widescreen patched etc...)

## Prerequisites
NXDK will need some additional libraries to be installed. Use the following guide:  
    https://github.com/XboxDev/nxdk/wiki/Install-the-Prerequisites

## Build
1. Clone the repo
2. cd into the xbox_xbe_launcher folder
3. Update the submodules:  
        git submodule update --init --recursive
4. run make

## Usage
1. Extract the xiso using extract-xiso (32bit version recommended as 64bit can create corrupt files)
2. Rename the default.xbe located in the root of the extracted iso to anything you like (I use _default.xbe)
3. Copy the built default.xbe from the bin folder of this project to the extracted iso folder
4. Rebuild the xiso using extract-xiso with the '-c' option
5. Play


## Notes
Use UP and Down on the DPAD and Button A to select.

