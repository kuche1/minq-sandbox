
// TODO
//
// make it so that instead of invalidating the syscall id we skip the execution and overwrite the return code
//
// make it so that syscall settings are presistent over sandbox restart

// TOD0
//
// make it so that `parse_cmdline`, ... are actual functions, so that we know the variables used

#include "inc/1_includes_namespaces.cpp"

#include "inc/2_functions_macros.cpp"

int main(int argc, char *argv[]){

    #include "inc/3_parse_cmdline.cpp"

    #include "inc/4_spawn_executable.cpp"

    #include "inc/5_filter_syscalls.cpp"

    #include "inc/6_return.cpp"
}
