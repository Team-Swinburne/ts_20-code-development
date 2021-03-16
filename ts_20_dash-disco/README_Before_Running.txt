WELCOME TO THE TS_20 LittlevGL DASH PROGRAM
-------------------------------------------
NOTE THAT THIS PROJECT IS STILL USING THE LITTLEVGL DEMO CODE
-------------------------------------------
DESCRIPTION
This dash program works with three different environments:
1 - Simulation
2 - 32F429 reference board (To Be Removed)
3 - 32F469 board

Each environment uses the same base code in which the program is built,
however the referred drivers used to compile are different and specified
in the "platformio.ini" file (build_flags and src_filter).

What this means is that a change made for the simulation can be directly
replicated on our physical  board.

THINGS TO NOTE
To run the simulation, after building the simulation environment in VSCode, execute
the .bat file "RunSimulation" in the TS_20_DashCode folder. This copies the needed SDL2.dll
file from the resources folder to run the simulation then begins it.