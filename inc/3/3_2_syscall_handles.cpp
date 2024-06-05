
bool is_resolved_node_allowed(const Sandbox_settings& settings, const string& path){

    // check already existing filters

    {

        // this is bad but it's good enough
        string path_dash = path;
        if(!path_dash.ends_with("/")){
            path_dash += "/";
        }

        for(string allowed_path_dash : settings.filesystem_allowed_nodes){
            // cout << "DEBUG: path_dash:" << path_dash << " allowed_path_dash:" << allowed_path_dash << endl;

            // this is bad but it's good enough
            if(!allowed_path_dash.ends_with("/")){
                allowed_path_dash += "/";
            }

            // `path` and `allowed_path` are the same node

            if(path_dash == allowed_path_dash){
                return true;
            }

            // `path` is a file within `allowed_path`

            if(path_dash.starts_with(allowed_path_dash)){
                return true;
            }

        }

    }

    // interactively decide

    {

        // check if interactive mode is even enabled

        if(!settings.filesystem_ask){
            return false;
        }

        // allow/deny if found in the "permanent" lists

        static vector<string> permanent_whitelist_path;
        static vector<string> permanent_blacklist_path;

        static vector<string> permanent_whitelist_path_prefix;
        static vector<string> permanent_blacklist_path_prefix;

        if(vec_contains(permanent_whitelist_path, path)){
            return true;
        }else if(vec_contains(permanent_blacklist_path, path)){
            return false;
        }

        for(string& prefix : permanent_whitelist_path_prefix){
            if(path.starts_with(prefix)){
                return true;
            }
        }

        for(string& prefix : permanent_blacklist_path_prefix){
            if(path.starts_with(prefix)){
                return false;
            }
        }

        // ask user if he wants to permit/deny the syscall request

        cout << "Syscall request: filesystem: open `" << path << "`\n";

        for(;;){

            cout << "(a):allow / (d):deny / (pa):permanently-allow / (pd):permanently-deny / (pap):permanently-allow-prefix / (pdp):permanently-deny-prefix > ";

            string action;
            getline(cin, action);

            if(action == "a"){
                return true;

            }else if(action == "d"){
                return false;

            }else if(action == "pa"){
                permanent_whitelist_path.push_back(path);
                return true;

            }else if(action == "pd"){
                permanent_blacklist_path.push_back(path);
                return false;

            }else if(action == "pap"){
                cout << "Enter the prefix that you want to permanently allow: ";

                string prefix;
                getline(cin, prefix);

                if(!path.starts_with(prefix)){
                    cout << "Path `" << path << "` doesn't start with prefix `" << prefix << "`\n";
                    continue;
                }

                permanent_whitelist_path_prefix.push_back(prefix);
                return true;

            }else if(action == "pdp"){
                cout << "Enter the prefix that you want to permanently deny: ";

                string prefix;
                getline(cin, prefix);

                if(!path.starts_with(prefix)){
                    cout << "Path `" << path << "` doesn't start with prefix `" << prefix << "`\n";
                    continue;
                }

                permanent_blacklist_path_prefix.push_back(prefix);
                return false;

            }else{
                cout << "Invalid action `" << action << "`\n";
                continue;
            }
        }
    }

}

pair<bool, string> is_unresolved_node_allowed(const Sandbox_settings& settings, const pid_t relative_to_process_pid, const int relative_to_dir_fd, const string& unresolved_path){

    string resolved_path = resolve_path(relative_to_process_pid, relative_to_dir_fd, unresolved_path);

    return make_pair(is_resolved_node_allowed(settings, resolved_path), resolved_path);
}

pair<bool, string> handle_syscall_arg0path(const Sandbox_settings& settings, pid_t pid, struct user_regs_struct& regs){

    char* path_cstr = (char*)CPU_REG_R_SYSCALL_ARG0(regs);

    string path = process_read_cstr_as_string(pid, path_cstr);

    return is_unresolved_node_allowed(settings, pid, AT_FDCWD, path);
}

pair<bool, string> handle_syscall_arg0path_arg1path(const Sandbox_settings& settings, pid_t pid, struct user_regs_struct& regs){

    char* path0_cstr = (char*)CPU_REG_R_SYSCALL_ARG0(regs);
    char* path1_cstr = (char*)CPU_REG_R_SYSCALL_ARG1(regs);

    string path0 = process_read_cstr_as_string(pid, path0_cstr);
    string path1 = process_read_cstr_as_string(pid, path1_cstr);

    auto [allow0, info0] = is_unresolved_node_allowed(settings, pid, AT_FDCWD, path0);
    auto [allow1, info1] = is_unresolved_node_allowed(settings, pid, AT_FDCWD, path1);

    bool allow = allow0 && allow1;

    string allow0_str = allow0 ? "good" : "bad";
    string allow1_str = allow1 ? "good" : "bad";

    string info = "path0:" + allow0_str + "<" + info0 + "> path1:" + allow1_str + "<" + info1 + ">";

    return {allow, info};
}

pair<bool, string> handle_syscall_arg0dirfd_arg1path(const Sandbox_settings& settings, pid_t pid, struct user_regs_struct& regs){

    int dir_fd = CPU_REG_R_SYSCALL_ARG0(regs);
    char* path_cstr = (char*)CPU_REG_R_SYSCALL_ARG1(regs);

    string path = process_read_cstr_as_string(pid, path_cstr);

    return is_unresolved_node_allowed(settings, pid, dir_fd, path);

}

pair<bool, string> handle_syscall_arg0dirfdA_arg1pathA_arg2dirfdB_arg3pathB(const Sandbox_settings& settings, pid_t pid, struct user_regs_struct& regs){

    int dir_fd_old = CPU_REG_R_SYSCALL_ARG0(regs);
    char* path_cstr_old = (char*)CPU_REG_R_SYSCALL_ARG1(regs);

    int dir_fd_new = CPU_REG_R_SYSCALL_ARG2(regs);
    char* path_cstr_new = (char*)CPU_REG_R_SYSCALL_ARG3(regs);

    string path_old = process_read_cstr_as_string(pid, path_cstr_old);
    string path_new = process_read_cstr_as_string(pid, path_cstr_new);

    auto [allow0, info0] = is_unresolved_node_allowed(settings, pid, dir_fd_old, path_old);
    auto [allow1, info1] = is_unresolved_node_allowed(settings, pid, dir_fd_new, path_new);

    bool allow = allow0 && allow1;

    string allow0_str = allow0 ? "good" : "bad";
    string allow1_str = allow1 ? "good" : "bad";

    string info = "path0:" + allow0_str + "<" + info0 + "> path1:" + allow1_str + "<" + info1 + ">";

    return {allow, info};
}

pair<bool, string> handle_syscall_arg0path_arg1dirfdA_arg2pathA(const Sandbox_settings& settings, pid_t pid, struct user_regs_struct& regs){

    char* path0_cstr = (char*)CPU_REG_R_SYSCALL_ARG0(regs);

    int dirfd1 = CPU_REG_R_SYSCALL_ARG1(regs);
    char* path1_cstr = (char*)CPU_REG_R_SYSCALL_ARG2(regs);

    string path0 = process_read_cstr_as_string(pid, path0_cstr);
    string path1 = process_read_cstr_as_string(pid, path1_cstr);

    auto [allow0, info0] = is_unresolved_node_allowed(settings, pid, AT_FDCWD, path0);
    auto [allow1, info1] = is_unresolved_node_allowed(settings, pid, dirfd1, path1);

    bool allow = allow0 && allow1;

    string allow0_str = allow0 ? "good" : "bad";
    string allow1_str = allow1 ? "good" : "bad";

    string info = "path0:" + allow0_str + "<" + info0 + "> path1:" + allow1_str + "<" + info1 + ">";

    return {allow, info};
}
