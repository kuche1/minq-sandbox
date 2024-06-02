
// c inludes
#include <unistd.h>
#include <sys/ptrace.h>
#include <signal.h>
#include <seccomp.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <fcntl.h>

// c++ includes
#include <iostream>
#include <cassert>
#include <map>
#include <vector>
#include <iomanip>

using namespace std;
