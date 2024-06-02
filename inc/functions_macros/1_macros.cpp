
//////////////
////////////////
/////////////////// generic
////////////////
/////////////

#define PATH_MAXLEN (4096+1) /* +1 for the ending \0 */

#define ERR_FAILED_CALL(to_what) { \
    cout << "Failed call to `" << to_what << "` in file `" << __FILE__ << "` at line " << __LINE__ << '\n'; \
    exit(1); \
}

// ptrace

#define PTRACE(...) { \
    if(ptrace(__VA_ARGS__)){ \
        ERR_FAILED_CALL("ptrace"); \
    } \
}

// `RW` means that we can both read and write to the register
// `R` means that the register is read-only
#if __WORDSIZE == 64
    #define CPU_REG_RW_SYSCALL_ID(regs) regs.orig_rax
    #define CPU_REG_R_SYSCALL_ARG0(regs) regs.rdi
    #define CPU_REG_R_SYSCALL_ARG1(regs) regs.rsi
    #define CPU_REG_R_SYSCALL_ARG2(regs) regs.rdx
    #define CPU_REG_R_SYSCALL_ARG3(regs) regs.r10
    #define CPU_REG_RW_SYSCALL_RET(regs) regs.rax // the return code of the syscall
#else
    #error Only 64bit is supported for now
#endif

//////////////
////////////////
/////////////////// seccomp
////////////////
/////////////


#define SECCOMP_ATTR_SET(...) { \
    if(seccomp_attr_set(__VA_ARGS__)){ \
        ERR_FAILED_CALL("seccomp_attr_set"); \
    } \
}

// `-EACCES` means that the rule conflicts with the filter (for example, the rule action equals the default action of the filter)
#define SECCOMP_RULE_ADD(...) { \
    int ret = seccomp_rule_add(__VA_ARGS__); \
    if((ret != 0) && (ret != -EACCES)){ \
        ERR_FAILED_CALL("seccomp_rule_add"); \
    } \
}

#define SECCOMP_LOAD(...) { \
    if(seccomp_load(__VA_ARGS__)){ \
        ERR_FAILED_CALL("seccomp_load"); \
    } \
}

//////////////
////////////////
/////////////////// exec
////////////////
/////////////


#define EXECVP(...) { \
    execvp(__VA_ARGS__); \
    ERR_FAILED_CALL("execvp"); \
}

//////////////
////////////////
/////////////////// signals
////////////////
/////////////


#define KILL(...) {\
    if(kill(__VA_ARGS__)) {\
        ERR_FAILED_CALL("kill"); \
    } \
}
