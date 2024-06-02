
if(argc <= 1){
    cout << "You need to specify the executable that you want to run, and the argument that are to be passed to it\n";
    exit(1);
}

char* executable = argv[1];
char** executable_args = argv + 1;
