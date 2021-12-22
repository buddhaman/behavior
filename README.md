
# Voxel Godbox.

Some stuff thrown together to generate a quick voxel world and play with it. Kind of paint 3d.

## Building

### Linux

Make sure SDL2 is installed. Use your package manager. Run the build script.

    ./build all

This is all. Now run with

    ./behavior

## Windows

Requires more work, but is not hard. Find the equivalent commands for every line in the build script. 
These are technically just 4 lines. Most of the commands are trivial, I just don't feel like it now.

## Faster building

I put all external libraries into one object file. You don't have to recompile that every time.
Just run ./build without any arguments if you only want to recompile code in /src.

## Ideas 

 - Living paint 3d. Draw houses. Generate world. Make creatures. 
 - Add drones that build buildings. 
 - Switch to type of block instead of color.
 - Implement water.
 - Implement alpha and sorting. 
   Explode. Erase. Trees. 
 - Generate cities.
 - Add cars.

