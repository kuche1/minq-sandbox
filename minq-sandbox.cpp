
// TODO
//
// add flag that makes it so that only ~ is filtered and nothing else
//
// maybe we should just use paths instead of strings
//
// new function `process_get_cwd` and use it in: string resolve_path(pid_t process_pid, int relative_to_fd, const string& path)
// ACTUALLY I need to check if AT_CWD is a real file descriptor or just a hack, cuz if it's a real fd that means that some of my rules are fucked sice they rely that the only way of creaaating new fds if thru open
//
// this doesnt work:
// ./compile.sh && ./minq-sandbox --fs-common-allow --fs-metadata-allow-all --fs-allow:/tmp/a touch /tmp/a
// curiously, adding sudo does work:
// ./compile.sh && sudo ./minq-sandbox --fs-common-allow --fs-metadata-allow-all --fs-allow:/tmp/a touch /tmp/a

// TOD0
//
// make user interactions colorful

// TOD1
//
// if the sandbox has any questions to ask: save the terminal buffer, ask the question, restore the buffer
// https://unix.stackexchange.com/questions/243237/how-to-save-restore-terminal-output
// sounds cool, but is probably a bad idea
//
// add the option to save all filesystem choices in a folder
//
// make it so that instead of invalidating the syscall id we skip the execution and overwrite the return code
// this is somewhat solved since I can't find any apps that break with the current way of ivalidating+changing the syscall ret code

// BAD IDEAS
//
// filter based on read-only/read-write
// the sandbox becomes too painful to use (from the user's prespective)

#include "inc/1/0_includes_namespaces.cpp"
#include "inc/2/0_functions_macros.cpp"
#include "inc/3/0_main.cpp"
