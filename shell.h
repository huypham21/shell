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

class ProcessFlow;
class Controller;

using dirstack_t = deque<string>;

// Represents a process
class Subprocess {
    friend class Controller;
    friend class ProcessFlow;
public:
    pid_t run(int fd_in=0, int fd_out=1);
    void exec(int fd_in=0, int fd_out=1);

    const pid_t& get_pid() const { return _pid; }
    const string& get_command() const { return command; }
    const vector<string>& get_args() const { return argsVect; }
    const vector<string>& get_tokens() const { return tokensVect; }
    ProcessFlow& get_flow() const { return *_flow; }

    char** get_argv() const;
    void free_argv(char** argv) const;
private:
    pid_t _pid = -1;
    ProcessFlow* _flow;
    string command;
    vector<string> argsVect, tokensVect;
};


class ProcessFlow {
    friend class Controller;
public:
    void add_process(Subprocess& proc);
    void set_input(const string& filename);
    void set_output(const string& filename);

    Controller& get_controller() const { return *_controller; }
    const vector<Subprocess>& get_flow() const { return _flow; }

    bool prepare();
    pid_t run(bool background = false);

    ~ProcessFlow();
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
    void parse(const vector<string>& tokens);
    void reset_pending() { _pending.clear(); }

    void enqueue_job(shared_ptr<ProcessFlow> flow, bool background = false);
    shared_ptr<ProcessFlow> get_job(int job_id) const;
    const vector<shared_ptr<ProcessFlow>>& get_jobs() const { return _jobs; }
    dirstack_t& get_dirstack() { return _dirstack; }
    const bool is_background_pid(int pid) const;

    void run();

    void update_child(pid_t pid, int status);


private:
    // Holds jobs while parsing. Second value in pair is background flag.
    deque<pair<shared_ptr<ProcessFlow>, bool>> _pending;
    dirstack_t _dirstack;

    vector<shared_ptr<ProcessFlow>>
        _flows, // Foreground commands in order
        _jobs;  // Background jobs indexed by ID - 1
};

