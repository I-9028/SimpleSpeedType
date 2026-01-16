# SimpleSpeedType
A simple Terminal-based typing test written in C using ncurses. Test your typing skills using both timed and untimed modes.

## Features
- **Timed Mode**: Race against the clock with a customizable timer of upto 600s. Can you complete the test before the timer runs out?
- **Untimed Mode**: Relax and finish the test at your leisre.
- **Real-time Colored Feedback**: Gray shows untyped text, white shows text ypu entered correctly, red shows incorrect letters (do you have the leeway to go back and correct?), green denotes the current character you're on.

## Suppoted Platfroms
**Linux**, **Windows**.

## Prerequisites
**Windows**: N/A
**Linux**: You'll need the ``ncurses`` library installed:

### Ubuntu/Debian

``sudo apt install libncurses-dev``

### Fedora/RHEL

``sudo dnf install ncurses-devel``

### Arch-Linux

``sudo pacman -S ncurses``

## Build Instructions
1) Clone or download the repository

2) Compile the program:
``cc -O3 speedtype.c -lncurses -o speedtype``

3) Run the executable:
``./speedtype``

## Usage
### Starting the Program:
When you run the program, you will see:
```
Welcome to this Typing Test.
Press Enter to Begin
Press Esc to exit.
```

### Selecting a Mode
If you press anything apart from ``Enter``, you exit the program. On pressing ``Enter``, you are greeted by a prompt asking you to select a mode:
```
Timed Mode or No? Y/N
Default is non-timed
```

### Timed Mode
Pressing ``Y`` or ``y`` lands you into timed mode. 

Enter the test duration in seconds (1-600, default: 60)
```
Enter the number of seconds you wanna test for
Enter a number between 0 and 600
Default is 60s
```

The timer displays at the top while you type:
```
                    Time: 28.451s
   
The quick brown fox jumps over the lazy dog
```

The test ends when you finish, time runs out, or you press ``Esc``.

### Untimed Mode

Pressing any other key on the mode selection screen alnds you here.

Type at your own pace.

Complete the entire text or press ``Esc`` to exit early.

### Results

Once you finish or exit the test, the results are displays as follows:
```
Total characters: 139
Correct characters: 126
Incorrect characters: 13
Typing accuracy: 90.65%
Time Taken: 59.203 s
```

## Configuration
The text you wish to perform a test with should be pasted in ``data.txt``

## To-Do
1) Add UserWarning if data.txt not provided with Windows executable.
2) Fix Issues regarding missing statistics display on Windows testing.
3) Add a Makefile for easier packaging.
4) Add a strict mode. Exits when you make a single error.
5) Add multiple text samples for variety.
