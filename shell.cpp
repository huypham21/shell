#include <set>
#include <iostream>
#include <utility>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include "shell.h"


char** Subprocess::get_argv() const {
    char ** argv = new char*[argsVect.size() + 2];

    argv[0] = new char[command.size() + 1];
    strcpy(argv[0], command.c_str());

    argv[argsVect.size() + 1] = NULL;

    for(size_t i = 0; i < argsVect.size(); i++){
        argv[i + 1] = new char[argsVect[i].size() + 1];
        strcpy(argv[i + 1], argsVect[i].c_str());
    }

    return argv;
}

void Subprocess::free_argv(char** argv) const {
    for(size_t i = 0; i < argsVect.size() + 1; i++) {
        delete [] argv[i];
    }

    delete [] argv;
}

void Subprocess::exec(int fd_in, int fd_out) {
    // Replace or close stdin
    if (fd_in == -1) {
        close(0);
    } else if (fd_in != 0) {
        if (dup2(fd_in, 0) == -1) {
            cerr << "failed to dup2 stdin" << endl;
            exit(1);
        }
        close(fd_in);
    }

    // Replace or close stdout
    if (fd_out == -1) {
        close(1);
    } else if (fd_out != 1) {
        if (dup2(fd_out, 1) == -1) {
            cerr << "failed to dup2 stdout" << endl;
            exit(1);
        }
        close(fd_out);
    }

    // Set up pathname and argv for execvp()
    const char* pathname = command.data();
    char** argv = get_argv();

    //execvp() 
    execvp(pathname, argv);

    cerr << "Command failed to execute: "<< command << endl;

    free_argv(argv);
}

pid_t Subprocess::run(int fd_in, int fd_out) {
    pid_t pid; 

    if ((pid = fork ()) == 0) {
        exec(fd_in, fd_out);
        exit(1);
    }

    return _pid = pid;
}


// Add a process object
void ProcessFlow::add_process(Subprocess& process) {
    process._flow = this;
    _flow.push_back(process);
}

// Assign a file to read stdin from.
void ProcessFlow::set_input(const string& filename) {
    _input_filename = filename;
}

// Assign a file to write stdout to.
void ProcessFlow::set_output(const string& filename) {
    _output_filename = filename;
    
}

// Opens files as a prepare stage.
bool ProcessFlow::prepare() {

    // Attach stdin to file if requested
    if (!_input_filename.empty()) {
        _input_handle = fopen(_input_filename.c_str(), "rb");
         if (_input_handle == NULL) cerr << "File not found";
    }

    // Attach stdout to file if requested
    if (!_output_filename.empty()) {
        _output_handle = fopen(_output_filename.c_str(),"wb");
         if (_output_handle == NULL) cerr << "File not found";

    }

    _ready = true;
    return _ready;
}

// Fork and exec
pid_t ProcessFlow::run(bool background) {
    if (_flow.empty()) return -1;

    int last_proc = _flow.size() - 1,
        fd_in  = 0,
        fd_out = 1,
        pipes[2];

    for (int i=0 ; i <= last_proc ; i++) {
        Subprocess& process = _flow[i];

        // Create pipe, if needed
        if (i != last_proc && pipe(pipes) != 0)
            cerr<<"pipe creation failed";

        if (i == 0) {
            // Attach stdin to file if requested
            if (_input_handle != NULL)
                fd_in = fileno(_input_handle);
            // Close stdin of first process if backgrounding and no redirect
            else if (background)
                fd_in = -1;
        }

        // Attach stdout to file if requested
        if (i == last_proc) {
            if (_output_handle != NULL)
                fd_out = fileno(_output_handle);
            else
                fd_out = 1;
        } else {
            fd_out = pipes[1];
        }

        process.run(fd_in, fd_out);

        if (i != last_proc) {
            fd_in = pipes[0]; // Set for the next loop
            close(pipes[1]);  // Close parent end, only used by child
        }

        // Close input/output files if we opened them
        if (i == 0 && _input_handle != NULL) {
            fclose(_input_handle);
            _input_handle = NULL;
        }

        if (i == last_proc && _output_handle != NULL) {
            fclose(_output_handle);
            _output_handle = NULL;
        }
    }

    // Return the last process in the chain
    if (background)
        return _flow.back()._pid;

    int saved_stdin = dup(0);
    close(0);
    int status;
    pid_t wait_pid = _flow.back()._pid;
    if (wait_pid > 0) 
        waitpid(wait_pid, &status, 0);
    dup2(saved_stdin, 0);
}

ProcessFlow::~ProcessFlow() {
    if(_input_handle != NULL) {
        fclose(_input_handle);
        _input_handle = NULL;
    }

    if(_output_handle != NULL) {
        fclose(_output_handle);
        _output_handle = NULL;
    }
}



bool special_tokens(const string& token) {
    static const set<string> tokens {"&", ";", "|", "<", ">"};
    return tokens.count(token) > 0;
}

void Controller::update_child(pid_t pid, int status) {
}

void Controller::parse(const vector<string>& tokens) {
    int num_tokens = tokens.size(),
        last_index = num_tokens - 1;

    Subprocess current_cmd;
    shared_ptr<ProcessFlow> current_flow = make_shared<ProcessFlow>();
    bool current_special;

    for (int i=0; i < num_tokens; i++) {
        // cout<<"\n Tokens at "<< i<<" : "<<tokens.at(i)<<"\n";
        current_special = special_tokens(tokens[i]);
        current_cmd.tokensVect.push_back(tokens[i]);

        // Run as background job
        if (tokens[i] == "&") {
            current_flow->add_process(current_cmd);
            enqueue_job(current_flow, true);

            current_cmd = Subprocess();
            current_flow = make_shared<ProcessFlow>();
        }
        // Run as foreground job
        else if (tokens[i] == ";") {
            current_flow->add_process(current_cmd);
            enqueue_job(current_flow, false);

            current_cmd = Subprocess();
            current_flow = make_shared<ProcessFlow>();
        }
        // Pipe 
        else if (tokens[i] == "|") {
            current_flow->add_process(current_cmd);
            current_cmd = Subprocess();
        }
        // Write file
        else if (tokens[i] == ">") {
            current_flow->set_output(tokens[++i]);
            current_cmd.tokensVect.push_back(tokens[i]);
        }
        // Read file
        else if (tokens[i] == "<") {  
            current_flow->set_input(tokens[++i]);
            current_cmd.tokensVect.push_back(tokens[i]);
        }
        // Build command w/ args
        else {
            if(current_cmd.command.empty())
                current_cmd.command = tokens[i];
            else
                current_cmd.argsVect.push_back(tokens[i]);
        }
    }

    if (!current_cmd.command.empty())
        current_flow->add_process(current_cmd);
    if (!current_flow->_flow.empty())
        enqueue_job(current_flow);
}

void Controller::run() {
    try {
        for(auto &flow_bg : _pending) {
            flow_bg.first->prepare();
        }
    } catch (exception& e) {
        reset_pending();
        throw;
    }

    // Populate and start background and foreground jobs, in order
    while (!_pending.empty()) {
        pair<shared_ptr<ProcessFlow>, bool> front = _pending.front();
        shared_ptr<ProcessFlow> flow = front.first;
        bool background = front.second;

        if(background) {
            _jobs.push_back(flow);
            _jobs.back()->run(true);
        } else {
            flow->run();
        }

        _pending.pop_front();
    }
}

void Controller::enqueue_job(shared_ptr<ProcessFlow> flow, bool background) {
    flow->_controller = this;
    _pending.emplace_back(flow, background);
}

// Retreive a job
shared_ptr<ProcessFlow> Controller::get_job(int job_id) const {
    return _jobs[job_id - 1];
}

// Searches jobs for a PID. If it's a background PID, return true.
const bool Controller::is_background_pid(int pid) const {
    for (const auto &job : _jobs)
        for (const Subprocess& process : job->_flow)
            if (process._pid == pid) return true;
    return false;
}
