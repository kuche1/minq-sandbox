
// TODO
//
// make it so that instead of invalidating the syscall id we skip the execution and overwrite the return code
//
// make it so that syscall settings are presistent over sandbox restart

// TOD0
//
// make the file `functions_macros.cpp` less fucky

#include "inc/includes_namespaces.cpp"

#include "inc/functions_macros.cpp"

int main(int argc, char *argv[]){

    #include "inc/parse_cmdline.cpp"

    #include "inc/spawn_executable.cpp"

    #include "inc/filter_syscalls.cpp"

    return return_code;
}
