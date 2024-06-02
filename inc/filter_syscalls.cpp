
// TODO? make it so that we get the return code of the main thread and return that, altho
//  it might be the case that the main thread is just a "spawner"
// we should probably make this into a define - weather to return bad code if ANY of the processes has
//  failed, or to only return the code of "spawner", or even to kill all processes if the "spawner" has died
//  (altho this seems like the worst option since we would probably break some apps)

int return_code = 69;
// if this doesn't get changed, that must meanthat we made a mistake

{

    int processes_running = 1;
    int processes_failed = 0;
    int syscalls_blocked = 0;

    for(;;){
        
        int status;
        pid_t pid = waitpid(-1, &status, 0); // the first argument being -1 means: wait for any child process
        // `waitpid` returns when a child's state changes, and that means: the child terminated; the child was stopped by a signal; or the child was resumed by a signal

        if(
            ( status>>8 == (SIGTRAP | (PTRACE_EVENT_CLONE<<8)) ) ||
            ( status>>8 == (SIGTRAP | (PTRACE_EVENT_FORK<<8))  ) ||
            ( status>>8 == (SIGTRAP | (PTRACE_EVENT_VFORK<<8)) )
        ){
            // new process was created

            // cout << "DEBIG: new process was spawned\n";
            processes_running += 1;
            PTRACE(PTRACE_CONT, pid, NULL, NULL);
            continue;

        }else if(
            status>>8 == (SIGTRAP | (PTRACE_EVENT_EXIT<<8))
        ){
            // process died

            // cout << "DEBUG: process died\n";
            processes_running -= 1;

            unsigned long event_message;
            PTRACE(PTRACE_GETEVENTMSG, pid, NULL, &event_message);

            if(event_message){
                // note that it might be the case that the return code signifies something else
                // rather than success/failure
                processes_failed += 1;
            }

            if(pid == original_spawned_process_pid){
                // there's something wrong with the code that gets the return code
                // so we'll only use 0 or 1
                if(event_message){
                    return_code = 1;
                }else{
                    return_code = 0;
                }
            }

            ptrace(PTRACE_CONT, pid, NULL, NULL);

            if(processes_running <= 0){
                break;
            }

            continue;

        }else if(
            status>>8 == (SIGTRAP | (PTRACE_EVENT_SECCOMP<<8))
        ){
            // generic syscall that we need to filter

        }else{
            // // no idea how we got here

            // // it seems that if we get to this point we cannot effect the syscall
            // // eg, we cannot invalidate the syscall id
            // // (this very well seems to be the case with the execve syscall)

            // cout << "DEBUG: wtf, this must never happen; it's possible that the next syscall will be unblockable\n";

            // // TODO? check if the syscall is an `execvp`, and if so just give up
            // // however, it might be the case that we have fucked something up in the spawn code
            // // and somehow given permission to run execvp?????

            // if(!WIFSTOPPED(status)){
            //     cerr << "wtf ** 2 A\n";
            //     exit(1);
            // }

            // if(WSTOPSIG(status) != SIGTRAP){
            //     cerr << "wtf ** 2 B\n";
            //     exit(1);
            // }




            // // fuck this shit

            // if(WIFSTOPPED(status)){
            //     if(WSTOPSIG(status) != SIGTRAP){
            //         cerr << "DEBUG: wtf\n";
            //     }
            //     ptrace(PTRACE_CONT, pid, NULL, NULL);
            // }else{
            //     cerr << "DEBUG: fucking impossible\n";
            // }

            // // ptrace(PTRACE_CONT, pid, NULL, NULL);

            // continue;




            if(!WIFSTOPPED(status)){
                // WIFSTOPPED(status): returns true if the child process was stopped by delivery of a signal; this is only possible if the call was done using WUNTRACED or when the child is being traced
                // so, this was NOT caused by us, and using PTRACE_CONT will do nothing and fail
                continue;
            }

            // cout << "DEBUG: stop signal is " << WSTOPSIG(status) << '\n';

            // KILL(pid, SIGCONT);
            PTRACE(PTRACE_CONT, pid, NULL, NULL);

            continue;

        }

        // get value of CPU regs
        struct user_regs_struct regs;
        PTRACE(PTRACE_GETREGS, pid, NULL, &regs);

        long syscall_id = CPU_REG_RW_SYSCALL_ID(regs);
        bool syscall_allow = false;

        switch(syscall_id){

            case SYS_socket:
            case SYS_socketpair:
            {
                // https://man7.org/linux/man-pages/man2/socket.2.html

                int domain = CPU_REG_R_SYSCALL_ARG0(regs);
                
                switch(domain){

                    case AF_LOCAL: // same as AF_UNIX
                    case AF_BRIDGE:
                    case AF_NETLINK:
                        syscall_allow = true;
                        break;

                    case AF_INET:
                    case AF_INET6:
                    case AF_DECnet:
                    case AF_ROSE:
                        syscall_allow = false;
                        break;

                    default:
                        cerr << "Unknown domain `" << domain << "` detected during socket creation\n";
                        break;

                }

            } break;


            case SYS_open:
            {

                int dir_fd = AT_FDCWD;
                char *filename = (char*)CPU_REG_R_SYSCALL_ARG0(regs);
                int flags = CPU_REG_R_SYSCALL_ARG1(regs);
                mode_t mode = CPU_REG_R_SYSCALL_ARG2(regs);

                syscall_allow = handle_syscall_openat(pid, dir_fd, filename, flags, mode);

            } break;


            case SYS_openat:
            {

                int dir_fd = CPU_REG_R_SYSCALL_ARG0(regs);
                char *filename = (char*)CPU_REG_R_SYSCALL_ARG1(regs);
                int flags = CPU_REG_R_SYSCALL_ARG2(regs);
                mode_t mode = CPU_REG_R_SYSCALL_ARG3(regs);

                syscall_allow = handle_syscall_openat(pid, dir_fd, filename, flags, mode);

            } break;

        }

        // block or allow the syscall

        if(!syscall_allow){

            syscalls_blocked += 1;

            cout << "Blocked syscall " << syscall_id << '\n';

            CPU_REG_RW_SYSCALL_ID(regs) = -1; // invalidate the syscall by changing the ID

            // TOD0 there is probably a way to only set the syscall id reg, and not all of them
            ptrace(PTRACE_SETREGS, pid, NULL, &regs);
        }

        PTRACE(PTRACE_CONT, pid, NULL, NULL);

    }

    // print info

    cout << "Failed processes: " << processes_failed << '\n';
    cout << "Blocked syscalls: " << syscalls_blocked << '\n';

}
