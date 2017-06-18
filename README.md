# ImagePuzzle
GTK3+/C ImagePuzzle

At first you need to install following packages:

To install essential development tools:
sudo dnf install make automake clang kernel-devel gdb

For Fedora and Ubuntu:
sudo dnf install gtk3-devel gtk3-devel-docs glib2-doc cairo-devel
Ubuntu users will additionally require to install the package libgtk-3-dev

--------------------------------------------------------------------------------

To compile the game use the make command.
Then just execute the program with "./imagepuzzle".

As soon as you execute the program, the game will start automatically and the timer starts to run. 
For each difficulty you got different time limits:
1. Easy 60 seconds
2. Normal 300 seconds
3. Hard 600 seconds

Have fun!
