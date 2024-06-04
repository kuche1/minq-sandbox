
pair<bool, string> handle_syscall_openat(Sandbox_settings& settings, pid_t pid, int dir_fd, char *pidmem_filename, __attribute__((unused)) int flags, __attribute__((unused)) mode_t mode){

    // https://man7.org/linux/man-pages/man2/openat.2.html

    constexpr bool ret_value_if_cant_resolve_path = true;
    // probably file doesn't exist
    // denying the syscall would be the safer choice, however I've seen apps break because of this
    // as the app crashes thinkin that the OS doesnt support the `open` syscall (as of writing this the
    // method for invalidating syscalls that we use is to ivalidate the syscall id) (example: python3)

    string txt_cant_resolve_path = "cannot resolve path: ";
    string txt_cant_get_fd = "cannot determine FD related to: ";
    string txt_allowed_path = "permitted: ";

    // read and sanitise parameter `path` from process memory
    // this can, in fact, be a file or a folder

    string path = process_read_cstr_as_string(pid, pidmem_filename);

    if(dir_fd == AT_FDCWD){ // relative to CWD

        auto [failed, resolved_path] = resolve_path_at_cwd(path);
        if(failed){
            return make_pair(ret_value_if_cant_resolve_path, txt_cant_resolve_path + path);
        }
        path = resolved_path;
    
    }else{

        if(path.starts_with("/")){

            auto [failed, resolved_path] = resolve_path_at_cwd(path);
            if(failed){
                return make_pair(ret_value_if_cant_resolve_path, txt_cant_resolve_path + path);
            }
            path = resolved_path;

        }else{

            auto [failed, fd_path] = process_get_fd_path(pid, dir_fd);
            if(failed){
                return make_pair(ret_value_if_cant_resolve_path, txt_cant_get_fd + path);
            }

            // we know that `path` doesn't start with `/`, so the only thing to check is `fd_path`
            if(!fd_path.ends_with("/")){
                fd_path += "/";
            }
    
            // now combine the folder path and the filename
            path = fd_path + path;

        }

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

    return make_pair(is_node_allowed(settings, path), txt_allowed_path + path);

}
