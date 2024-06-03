
tuple<char*, char**> parse_cmdline(int argc, char**argv){

    if(argc <= 1){
        cout << "You need to specify the executable that you want to run, and the argument that are to be passed to it\n";
        exit(1);
    }

    char* executable = argv[1];
    char** executable_args = argv + 1;

    return make_tuple(executable, executable_args);
}
