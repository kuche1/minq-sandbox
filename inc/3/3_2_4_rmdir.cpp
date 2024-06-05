
pair<bool, string> handle_syscall_rmdir(Sandbox_settings& settings, pid_t pid, char *pidmem_path){

    // https://man7.org/linux/man-pages/man2/rmdir.2.html

    string path = process_read_cstr_as_string(pid, pidmem_path);

    return is_unresolved_node_allowed(settings, pid, AT_FDCWD, path);
}
