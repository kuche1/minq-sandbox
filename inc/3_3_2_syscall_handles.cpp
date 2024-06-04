
//////////////
////////////////
/////////////////// handling of syscalls
////////////////
/////////////

pair<bool, string> handle_syscall_openat(Sandbox_settings& settings, pid_t pid, int dir_fd, char *pidmem_filename, int flags, mode_t mode){

    // https://man7.org/linux/man-pages/man2/openat.2.html

    constexpr bool ret_value_if_cant_resolve_path = true;
    // probably file doesn't exist
    // denying the syscall would be the safer choice, however I've seen apps break because of this
    // as the app crashes thinkin that the OS doesnt support the `open` syscall (as of writing this the
    // method for invalidating syscalls that we use is to ivalidate the syscall id) (example: python3)

    // "permanent" settings

    static vector<string> permanent_whitelist_path;
    static vector<string> permanent_blacklist_path;

    static vector<string> permanent_whitelist_path_prefix;
    static vector<string> permanent_blacklist_path_prefix;

    // read and sanitise parameter `path` from process memory
    // this can, in fact, be a file or a folder

    string path = process_read_cstr_as_string(pid, pidmem_filename);

    if(dir_fd == AT_FDCWD){ // relative to CWD

        auto [failed, resolved_path] = resolve_path_at_cwd(path);
        if(failed){
            return make_pair(ret_value_if_cant_resolve_path, path);
        }
        path = resolved_path;
    
    }else{

        if(path.starts_with("/")){

            auto [failed, resolved_path] = resolve_path_at_cwd(path);
            if(failed){
                return make_pair(ret_value_if_cant_resolve_path, path);
            }
            path = resolved_path;

        }else{

            auto [failed, fd_path] = process_get_fd_path(pid, dir_fd);
            if(failed){
                return make_pair(ret_value_if_cant_resolve_path, path);
            }

            // we know that `path` doesn't start with `/`, so the only thing to check is `fd_path`
            if(!fd_path.ends_with("/")){
                fd_path += "/";
            }
    
            // now combine the folder path and the filename
            path = fd_path + path;

        }

    }

    // parse parameter `flags`

    // `flags` must include one of these: O_RDONLY (read only), O_WRONLY (write only), O_RDWR (read and write)
    // other flags can be bitwise OR-ed
    bool read_only = (flags | O_RDONLY);
    bool write_only = (flags | O_WRONLY);
    bool read_write = (flags | O_RDWR);

    // allow generic stuff

    if(path == "/dev/tty"){ // using the terminal (as in printing, not executing commands)
        return make_pair(true, path);
    }

    if(path == "/dev/null"){ // the "nothing" file
        return make_pair(true, path);
    }

    if(read_only){

        if(path.starts_with("/usr/lib/")){ // libraries
            return make_pair(true, path);
        }

        if(path == "/etc/ld.so.cache"){ // linker
            return make_pair(true, path);
        }

    }

    // allow/deny if found in the "permanent" lists

    if(vec_contains(permanent_whitelist_path, path)){
        return make_pair(true, path);
    }else if(vec_contains(permanent_blacklist_path, path)){
        return make_pair(false, path);
    }

    for(string& prefix : permanent_whitelist_path_prefix){
        if(path.starts_with(prefix)){
            return make_pair(true, path);
        }
    }

    for(string& prefix : permanent_blacklist_path_prefix){
        if(path.starts_with(prefix)){
            return make_pair(false, path);
        }
    }

    // allow whitelisted nodes

    for(string node : settings.filesystem_allowed_nodes){

        // if `path` is a file or a folder, and is the same as `node`

        if(path == node){
            return make_pair(true, path);
        }

        // if `path` is a file, and is in a folder `node`

        if(!node.ends_with("/")){
            node += "/";
        }

        if(path.starts_with(node)){
            return make_pair(true, path);
        }
    }

    // check if we should ask the user or refuse access

    if(!settings.filesystem_ask){
        return make_pair(false, path);
    }

    // ask user if he wants to permit/deny the syscall request

    cout << '\n';
    cout << "Syscall request: filesystem: open\n";
    cout << "   path:" << path << '\n';
    cout << "   flags:" << flags << " [read-only:" << read_only << " write-only:" << write_only << " read-write:" << read_write << "]\n";
    cout << "   mode:" << mode << '\n';
    cout << "   pid:" << pid << '\n';

    for(;;){

        cout << "(a):allow / (d):deny / (pa):permanently-allow / (pd):permanently-deny / (pap):permanently-allow-prefix / (pdp):permanently-deny-prefix > ";

        string action;
        getline(cin, action);

        if(action == "a"){
            return make_pair(true, path);

        }else if(action == "d"){
            return make_pair(false, path);

        }else if(action == "pa"){
            permanent_whitelist_path.push_back(path);
            return make_pair(true, path);

        }else if(action == "pd"){
            permanent_blacklist_path.push_back(path);
            return make_pair(false, path);

        }else if(action == "pap"){
            cout << "Enter the prefix that you want to permanently allow: ";

            string prefix;
            getline(cin, prefix);

            if(!path.starts_with(prefix)){
                cout << "Path `" << path << "` doesn't start with prefix `" << prefix << "`\n";
                continue;
            }

            permanent_whitelist_path_prefix.push_back(prefix);
            return make_pair(true, path);

        }else if(action == "pdp"){
            cout << "Enter the prefix that you want to permanently deny: ";

            string prefix;
            getline(cin, prefix);

            if(!path.starts_with(prefix)){
                cout << "Path `" << path << "` doesn't start with prefix `" << prefix << "`\n";
                continue;
            }

            permanent_blacklist_path_prefix.push_back(prefix);
            return make_pair(false, path);

        }else{
            cout << "Invalid action `" << action << "`\n";
            continue;
        }
    }

}
