
pair<bool, string> handle_syscall_openat(Sandbox_settings& settings, pid_t pid, int dir_fd, char *pidmem_filename, __attribute__((unused)) int flags, __attribute__((unused)) mode_t mode){

    // https://man7.org/linux/man-pages/man2/openat.2.html

    // do we permit or deny the syscall if the path cannot be resolved
    // denying the syscall might cause an app to fail (example: python3)
    //
    // perhaps this has to do something with the way we are currently
    // blocking the syscalls (by invalidating the ID)
    constexpr bool cant_resolve_path = true;

    // read and sanitise parameter `path` from process memory
    // this can, in fact, be a file or a folder

    string path = process_read_cstr_as_string(pid, pidmem_filename);

    if(dir_fd == AT_FDCWD){ // relative to CWD

        auto [failed, resolved_path] = resolve_path(pid, path);
        if(failed){
            return make_pair(cant_resolve_path, resolved_path);
        }
        path = resolved_path;
    
    }else{

        auto [failure_fd_path, fd_path] = process_get_fd_path(pid, dir_fd);
        if(failure_fd_path){
            return make_pair(cant_resolve_path, fd_path);
        }

        auto [failure_resolved_path, resolved_path] = resolve_path(pid, path, fd_path);
        if(failure_resolved_path){
            return make_pair(cant_resolve_path, resolved_path);
        }

        path = resolved_path;

    }

    // we actually won't be parsing the flags since it's too much of a pain for the end user
    // to have to specify all the files AND the flags
    {
        // // parse parameter `flags`

        // // `flags` must include one of these: O_RDONLY (read only), O_WRONLY (write only), O_RDWR (read and write)
        // // other flags can be bitwise OR-ed
        // bool read_only = (flags | O_RDONLY);
        // bool write_only = (flags | O_WRONLY);
        // bool read_write = (flags | O_RDWR);
    }

    // return

    return make_pair(is_node_allowed(settings, path), path);

}
