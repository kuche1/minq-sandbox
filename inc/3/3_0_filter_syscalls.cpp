
#include "3_1_syscall_names.cpp"
#include "3_2_syscall_handles.cpp"

int filter_syscalls(Sandbox_settings settings, pid_t first_child_pid){

    tuple<bool, int> return_code = make_tuple(true, 1);

    int processes_running = 1;
    int processes_failed = 0;
    int syscalls_blocked = 0;

    for(;;){
        
        int status;
        pid_t pid = waitpid(-1, &status, 0); // the first argument being -1 means: wait for any child process
        // `waitpid` returns when a child's state changes, and that means: the child terminated; the child was stopped by a signal; or the child was resumed by a signal

        if(pid == -1){
            // TOD0 what is causing this?
            cerr << "DEBUG: wtf, pid==-1; processes_running:" << processes_running << "\n";
            break;
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

            int code = event_message >> 8;

            if(code){
                // note that it might be the case that the return code signifies something else
                // rather than success/failure
                processes_failed += 1;
            }

            if(pid == first_child_pid){
                return_code = make_tuple(false, code);
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
        const char* syscall_name = get_syscall_name(syscall_id);
        bool syscall_allow = false;
        string syscall_info = "no info available";

        switch(syscall_id){

            // file operations

            case SYS_creat:
            case SYS_open:
            case SYS_mknod:
            case SYS_truncate:
            {
                tie(syscall_allow, syscall_info) = handle_syscall_arg0path(settings, pid, regs);
            } break;

            case SYS_openat:
            case SYS_name_to_handle_at:
            case SYS_mknodat:
            {
                tie(syscall_allow, syscall_info) = handle_syscall_arg0dirfd_arg1path(settings, pid, regs);
            } break;

            case SYS_rename:
            {
                tie(syscall_allow, syscall_info) = handle_syscall_arg0path_arg1path(settings, pid, regs);
            } break;

            case SYS_renameat:
            case SYS_renameat2:
            {
                tie(syscall_allow, syscall_info) = handle_syscall_arg0dirfdA_arg1pathA_arg2dirfdB_arg3pathB(settings, pid, regs);
            } break;

            // directory operations

            case SYS_mkdir:
            case SYS_rmdir:
            case SYS_chdir:
            case SYS_chroot:
            {
                tie(syscall_allow, syscall_info) = handle_syscall_arg0path(settings, pid, regs);
            } break;

            case SYS_mkdirat:
            {
                tie(syscall_allow, syscall_info) = handle_syscall_arg0dirfd_arg1path(settings, pid, regs);
            } break;

            // link operations

            case SYS_link:
            case SYS_symlink:
            {
                tie(syscall_allow, syscall_info) = handle_syscall_arg0path_arg1path(settings, pid, regs);
            } break;

            case SYS_linkat:
            {
                tie(syscall_allow, syscall_info) = handle_syscall_arg0dirfdA_arg1pathA_arg2dirfdB_arg3pathB(settings, pid, regs);
            } break;

            case SYS_symlinkat:
            {
                tie(syscall_allow, syscall_info) = handle_syscall_arg0path_arg1dirfdA_arg2pathA(settings, pid, regs);
            } break;

            case SYS_unlink:
            case SYS_readlink:
            {
                tie(syscall_allow, syscall_info) = handle_syscall_arg0path(settings, pid, regs);
            } break;

            case SYS_unlinkat:
            case SYS_readlinkat:
            {
                tie(syscall_allow, syscall_info) = handle_syscall_arg0dirfd_arg1path(settings, pid, regs);
            } break;

            // networking

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
                        syscall_info = "local";
                        break;

                    case AF_INET:
                    case AF_INET6:
                    case AF_DECnet:
                    case AF_ROSE:
                        syscall_allow = false;
                        syscall_info = "outside world";
                        break;

                    default:
                        cerr << "Unknown domain `" << domain << "` detected during socket creation\n";
                        exit(1);
                        break;

                }

            } break;

            // ...

            default:
            {
                cerr << "Unknown syscall: " << syscall_id << ": " << syscall_name << endl;
                exit(1);
            } break;

        }

        // block or allow the syscall

        if(!syscall_allow){

            syscalls_blocked += 1;

            cout << COL_BLOCKED_SYSCALL << "Blocked syscall " << syscall_id << ": " << syscall_name << ": " << syscall_info << COL_RESET << endl;

            CPU_REG_RW_SYSCALL_ID(regs) = -1; // invalidate the syscall by changing the ID
            CPU_REG_RW_SYSCALL_RET(regs) = -1; // also put bad return code, suprisingly this fixes some programs (example: python3)

            // TOD0 there is probably a way to only set the syscall id reg, and not all of them
            ptrace(PTRACE_SETREGS, pid, NULL, &regs);
        }

        PTRACE(PTRACE_CONT, pid, NULL, NULL);

    }

    // print info

    cout << "Failed processes: " << processes_failed << '\n';
    cout << "Blocked syscalls: " << syscalls_blocked << '\n';

    // return

    auto [failure, code] = return_code;
    if(failure){
        cerr << "Could not determine return code of original child";
        return 1;
    }

    return code;

}