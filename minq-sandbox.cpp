
// TODO
//
// make it so that instead of invalidating the syscall id we skip the execution and overwrite the return code
//
// make it so that syscall settings are presistent over sandbox restart
//
// handle filesystem:delete syscalls
// in fact, we don't seem to handle all open cases
// https://linasm.sourceforge.net/docs/syscalls/filesystem.php
//
// handle `--help`

// TOD0
//
// NONE

#include "inc/1_includes_namespaces.cpp"

#include "inc/2_functions_macros.cpp"

#include "inc/3_main.cpp"
