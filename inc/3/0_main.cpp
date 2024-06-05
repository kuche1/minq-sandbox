
#include "1_parse_cmdline.cpp"
#include "2_0_spawn_executable.cpp"
#include "3_0_filter_syscalls.cpp"

int main(int argc, char *argv[]){

    Sandbox_settings settings = parse_cmdline(argc, argv);

    pid_t first_child_pid = spawn_executable(settings);

    int return_code = filter_syscalls(settings, first_child_pid);

    return return_code;
}
