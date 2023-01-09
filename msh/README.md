## Mini Shell (msh)

---

- In this project, a mini shell was developed using C and common system calls in GNU C library.
- `./msh` in command line to run the msh
  - `make` in command line if `msh` is not compiled from source code

---

### Usage (in msh)

---

- `cdir path` - change the current directory of `msh` to `path`
  - Support environment variables such as `$HOME`
- `pdir` - output the absoulte path of the curernt directory
- `run process_name [ARG1] [ARG2] [ARG3] [ARG4`- spawn a new process for `process_name`
  - `[ARG1] [ARG2] ...` are arguments for running process `process_name`
    - The maximum arguments for `process_name` is 4
  - The maximum number of processes `msh` can run is 32
    - Once maximum number of processes is reached, `msh` will not spawn more processes until some processes are terminated
- `stop pid` - stop a process based on the given `pid`
- `continue pid` - continue a stopped process based on the given `pid`
- `terminate pid` - terminate a process based on the given pid
- `check` - display all the processes `msh` spawned and their status
- `exit` - close `msh` with clean up such as terminate all the processes `msh` spawned
- `quit` - close `msh` without cleaning up
