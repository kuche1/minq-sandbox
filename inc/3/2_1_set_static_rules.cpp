
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

    {
        uint32_t action = SCMP_ACT_TRACE(69);

        if(settings.filesystem_allow_all){
            action = SCMP_ACT_ALLOW;
        }

        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(open),   0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(openat), 0);

        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(mkdir),   0);
        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(mkdirat), 0);

        SECCOMP_RULE_ADD(ctx, action, SCMP_SYS(rmdir), 0);
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
