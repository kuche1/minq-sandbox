
pair<bool, string> handle_syscall_mkdirat(Sandbox_settings& settings, pid_t pid, int dir_fd, char *pidmem_filename, __attribute__((unused)) mode_t mode){

    // https://man7.org/linux/man-pages/man2/mkdirat.2.html

    string path = process_read_cstr_as_string(pid, pidmem_filename);

    return is_unresolved_node_allowed(settings, pid, dir_fd, path);
}
