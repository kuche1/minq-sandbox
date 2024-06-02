
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

        if(pid == -1){
            // TODO this should not be happening
            cerr << "DEBUG: wtf, pid==-1; processes_running:" << processes_running << "\n";
            exit(1);
            // continue;
        }

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

            if(!WIFSTOPPED(status)){
                // WIFSTOPPED(status): returns true if the child process was stopped by delivery of a signal; this is only possible if the call was done using WUNTRACED or when the child is being traced
                // so, this was NOT caused by us, and using PTRACE_CONT will do nothing and fail
                continue;
            }

            if(WSTOPSIG(status) == SIGTRAP){
                PTRACE(PTRACE_CONT, pid, NULL, NULL);
                continue;
            }

            // time_t now = time(nullptr);
            // cout << "DEBUG: " << put_time(localtime(&now), "%T") << " -> stop signal is " << WSTOPSIG(status) << '\n';

            // forward the signal to the child
            PTRACE(PTRACE_CONT, pid, NULL, WSTOPSIG(status));

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
                        exit(1);
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