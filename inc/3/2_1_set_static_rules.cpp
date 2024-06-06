
// SCMP_ACT_ALLOW - allow the syscall
// SCMP_ACT_LOG - allow but log
// SCMP_ACT_TRACE(69) - trigger a ptrace breakpoint

void set_static_rules(Sandbox_settings& settings){

    // allow all syscalls by default
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    if(ctx == NULL){
        ERR_FAILED_CALL("seccomp_init");
    }

    // do not send SIGSYS upon coming across an invalid syscall
    SECCOMP_ATTR_SET(ctx, SCMP_FLTATR_ACT_BADARCH, SCMP_ACT_ALLOW);

    // filesystem
    // https://linasm.sourceforge.net/docs/syscalls/filesystem.php

    {
        uint32_t action = SCMP_ACT_TRACE(69);
        if(settings.fs_allow_all){
            action = SCMP_ACT_ALLOW;
        }

        uint32_t action_metadata = action;
        if(settings.fs_metadata_allow_all){
            action_metadata = SCMP_ACT_ALLOW;
        }

        // file operations

        // SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(close), 0); // harmless
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(creat), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(open), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(openat), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(name_to_handle_at), 0);
        // SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(open_by_handle_at), 0); // depends on `name_to_handle_at`
        // SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(memfd_create), 0); // the file lives in RAM
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(mknod), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(mknodat), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(rename), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(renameat), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(renameat2), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(truncate), 0);
        // SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(ftruncate), 0); // depends on `open`
        // SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(fallocate), 0); // depends on `open`

        // directory operations

        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(mkdir), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(mkdirat), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(rmdir), 0);
        // SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(getcwd), 0); // gives info that the app should already have
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(chdir), 0);
        // SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(fchdir), 0); // depends on `open`
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(chroot), 0);
        // SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(getdents), 0); // depend on `open`
        // SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(getdents64), 0); // depend on `open`
        // SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(lookup_dcookie), 0); // depends on `getdents64` for getting the cookie

        // link operations

        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(link), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(linkat), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(symlink), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(symlinkat), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(unlink), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(unlinkat), 0);
        SECCOMP_RULE_ADD(ctx, action_metadata, SCMP_SYS(readlink), 0);
        SECCOMP_RULE_ADD(ctx, action_metadata, SCMP_SYS(readlinkat), 0);

        // basic file attributes

        // SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(umask), 0); // depends on `open`
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(chmod), 0);
        // SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(fchmod), 0); // depends on `open`
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(fchmodat), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(chown), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(lchown), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(fchown), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(fchownat), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(utime), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(utimes), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(futimesat), 0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(utimensat), 0);
        // SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(futimens), 0); // depends on `open`

        // get file status
        SECCOMP_RULE_ADD(ctx, action_metadata, SCMP_SYS(stat), 0);
        SECCOMP_RULE_ADD(ctx, action_metadata, SCMP_SYS(lstat), 0);
        // SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(fstat), 0); // depends on `open`
        // SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(fstatat64), 0); // only 32bit
        SECCOMP_RULE_ADD(ctx, action_metadata, SCMP_SYS(newfstatat), 0);

        // check user permission for file
        SECCOMP_RULE_ADD(ctx, action_metadata, SCMP_SYS(access), 0);
        SECCOMP_RULE_ADD(ctx, action_metadata, SCMP_SYS(faccessat), 0);
        SECCOMP_RULE_ADD(ctx, action_metadata, SCMP_SYS(faccessat2), 0);

        // extended file attributes // TODO
    }

    // networking

    {
        uint32_t action = SCMP_ACT_TRACE(69);

        if(settings.networking_enable){
            action = SCMP_ACT_ALLOW;
        }

        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(socket),     0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(socketpair), 0);
    }

    // // harmless (by themselves) syscalls
    // SECCOMP_RULE_ADD(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read),  0);
    // SECCOMP_RULE_ADD(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);

    // // cleanup syscalls
    // SECCOMP_RULE_ADD(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit),       0);
    // SECCOMP_RULE_ADD(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);

    // load the rules
    SECCOMP_LOAD(ctx);

}
