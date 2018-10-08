#include <array>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <utility>
#include <deque>
#include <memory>
#include <deque>

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include <functional>

using namespace std;

class Process;
class Controller;

using dirstack_t = deque<string>;

// Represents a process
class Subprocess {
    friend class Controller;
    friend class Process;
public:
    pid_t run(int fd_in=0, int fd_out=1);
    void exec(int fd_in=0, int fd_out=1);

     pid_t& get_pid()  { return _pid; }
     string& get_command()  { return command; }
     vector<string>& get_args()  { return argsVect; }
     vector<string>& get_tokens()  { return tokensVect; }
    Process& get_flow()  { return *_flow; }

    char** get_argv() ;
    void free_argv(char** argv) ;
private:
    pid_t _pid = -1;
    Process* _flow;
    string command;
    vector<string> argsVect, tokensVect;
};


class Process {
    friend class Controller;
public:
    void add_process(Subprocess& proc);
    void set_input( string& filename);
    void set_output( string& filename);

    Controller& get_controller()  { return *_controller; }
     vector<Subprocess>& get_flow()  { return _flow; }

    bool prepare();
    pid_t run(bool background = false);

    ~Process();
private:
    Controller* _controller;
    string _input_filename,
                _output_filename;
    FILE *_input_handle = NULL,
         *_output_handle = NULL;
    bool _detach_all = false,
         _ready = false;
    vector<Subprocess> _flow;
};

class Controller {
public:
    void tokenizer( vector<string>& tokens);
    void reset() { _pending.clear(); }

    void enqueue_job(shared_ptr<Process> flow, bool background = false);
    dirstack_t& get_dirstack() { return _dirstack; }
     bool is_background_pid(int pid) ;

    void run();

    void update_child(pid_t pid, int status);


private:
    // Holds jobs while parsing. Second value in pair is background flag.
    deque<pair<shared_ptr<Process>, bool>> _pending;
    dirstack_t _dirstack;

    vector<shared_ptr<Process>>_jobs;  // Background jobs indexed by ID - 1
};

