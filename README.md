# NotePlayer

A C program that plays a specified note on a macOS device's built-in hardware, using AudioToolbox. The user may give the program a frequency to play, and set the octave and volume to play it at.

## Getting Started

### Requirements

* A Mac system with macOS 10.0+

### Compiling

```
clang -lc -framework AudioToolbox NotePlayer.c -o NotePlayer
```

### Executing

```
./NotePlayer <frequency> <octave difference> <up(0)/down(1)> <optional: volume(1-100)>
```

```
Ex for one octave above middle C: ./NotePlayer 261.6 1 0
```

## Authors

Kevin Schulcz