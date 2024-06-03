
// SCMP_ACT_ALLOW - allow the syscall
// SCMP_ACT_LOG - allow but log
// SCMP_ACT_TRACE(69) - trigger a ptrace breakpoint

{

    // allow all syscalls by default
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    if(ctx == NULL){
        ERR_FAILED_CALL("seccomp_init");
    }

    // do not send SIGSYS upon coming across an invalid syscall
    SECCOMP_ATTR_SET(ctx, SCMP_FLTATR_ACT_BADARCH, SCMP_ACT_ALLOW);

    // monitor file openings
    SECCOMP_RULE_ADD(ctx, SCMP_ACT_TRACE(69), SCMP_SYS(open),   0);
    SECCOMP_RULE_ADD(ctx, SCMP_ACT_TRACE(69), SCMP_SYS(openat), 0);

    {
        uint32_t action = SCMP_ACT_TRACE(69);

        if(networking_enable){
            action = SCMP_ACT_ALLOW;
        }

        // monitor socket creation
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
