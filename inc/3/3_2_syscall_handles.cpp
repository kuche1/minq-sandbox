
bool is_resolved_node_allowed(const Sandbox_settings& settings, const string& path){

    // check already existing filters

    {

        // this is bad but it's good enough
        string path_dash = path;
        if(!path_dash.ends_with("/")){
            path_dash += "/";
        }

        for(string allowed_path_dash : settings.filesystem_allowed_nodes){

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

    // do we permit or deny the syscall if the path cannot be resolved
    //
    // denying the syscall might cause an app to fail (example: python3)
    //
    // perhaps this has to do something with the way we are currently
    // blocking the syscalls (by invalidating the ID (as of writing this))
    constexpr bool cant_resolve_path = true;

    string resolved_path;

    if(relative_to_dir_fd == AT_FDCWD){ // relative to CWD

        auto [failed, resolved] = resolve_path(relative_to_process_pid, unresolved_path);
        if(failed){
            return make_pair(cant_resolve_path, resolved);
        }
    
        resolved_path = resolved;
    
    }else{

        auto [failure_fd_path, fd_path] = process_get_fd_path(relative_to_process_pid, relative_to_dir_fd);
        if(failure_fd_path){
            return make_pair(cant_resolve_path, fd_path);
        }

        auto [failure_resolved, resolved] = resolve_path(relative_to_process_pid, unresolved_path, fd_path);
        if(failure_resolved){
            return make_pair(cant_resolve_path, resolved);
        }

        resolved_path = resolved;

    }

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
    string info = "path0<" + info0 + "> path1<" + info1 + ">";

    return {allow, info};
}
