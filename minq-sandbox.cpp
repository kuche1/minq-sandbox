
// TODO
//
// the filesystem user interface sucks
//     (possible fix) add option to remember across sandbox reboots OR/AND add option to allow certain dirs as cmdline

// TOD0
//
// make it so that instead of invalidating the syscall id we skip the execution and overwrite the return code
//
// handle filesystem:delete syscalls
// in fact, we don't seem to handle all open cases
// https://linasm.sourceforge.net/docs/syscalls/filesystem.php

// TOD1
//
// if the sandbox has any questions to ask: save the terminal buffer, ask the question, restore the buffer
// https://unix.stackexchange.com/questions/243237/how-to-save-restore-terminal-output
// sounds cool, but is probably a bad idea

#include "inc/1_includes_namespaces.cpp"

#include "inc/2_functions_macros.cpp"

#include "inc/3_main.cpp"
