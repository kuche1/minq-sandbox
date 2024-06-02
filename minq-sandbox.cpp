
#include "inc/includes_namespaces.cpp"

#include "inc/functions_macros.cpp"

int main(int argc, char *argv[]){

    #include "inc/parse_cmdline.cpp"

    #include "inc/spawn_executable.cpp"

    #include "inc/filter_syscalls.cpp"

    return return_code;
}
