
// TODO
//
// handle filesystem:delete syscalls
// in fact, we don't seem to handle all open cases
// https://linasm.sourceforge.net/docs/syscalls/filesystem.php
//
// restructure using folders ? put a number and a name on the folder and only numbers on the files within?
//
// I should be taking into account children's PWD
// readlink -f /proc/<pid>/pwd
// https://stackoverflow.com/questions/23183093/how-to-resolve-a-file-path

// TOD0
//
// make it so that instead of invalidating the syscall id we skip the execution and overwrite the return code
//
// add the option to save all filesystem choices in a folder

// TOD1
//
// if the sandbox has any questions to ask: save the terminal buffer, ask the question, restore the buffer
// https://unix.stackexchange.com/questions/243237/how-to-save-restore-terminal-output
// sounds cool, but is probably a bad idea
//
// make user interactions colorful

// BAD IDEAS
//
// filter based on read-only/read-write
// the sandbox becomes too painful to use (from the user's prespective)

#include "inc/1_0_includes_namespaces.cpp"
#include "inc/2_0_functions_macros.cpp"
#include "inc/3_0_main.cpp"
