Note Player
By: Kevin Schulcz

Description:
    Plays a specified note, by frequency, on a Mac system's built-in 
    hardware, using AudioToolbox. Can play the note in different octaves 
    and at different volumes.

To Compile:
    Note - Requires a Mac system with macOS 10.0+.

    clang -lc -framework AudioToolbox NotePlayer.c -o NotePlayer

To Run:
    ./NotePlayer <frequency> <octave difference> <up(0)/down(1)> <optional: volume(1-100)>

    Eaxmple for one octave above middle C:
    ./NotePlayer 261.6 1 0
