# POSIX SHELL

### INTRODUCTION

This is a POSIX compliant shell that allows user to interact with the OS via a command line interface. It supports the following features, and is supported by all POSIX compliant systems:

1. Basic working: Support for shell commands like ls, echo, touch, mkdir, grep, pwd, cd, cat, head, tail, chmod, exit, history, clear and cp.
2. IO Redirection using `>` or `>>` for one source and destination only
3. Usage of `|` symbol to implement the pipe feature. At least 3 commands can be piped successfully.
4. `&` symbol at the end of a command to run it in the background, and the ability to use `fg` command to bring a specific process to foreground.
5. Storing entered commands in a history, also searchable via trie.
6. Recording entered commands into an output file.
7. Alarm feature for specific date/time.
8. Aliases.
9. Autocompletion of commands using trie.
10. Support for environment variables.
11. `$$` to print process id of shell
12. `$?` to print exit status of last command run

### FILE STRUCTURE

1. main.cpp
2. alarm.h
3. execute.h
4. file_mapping.h
5. histalias.h

### RUNNING

`main.cpp` needs to be compiled as follows (or via some g++ equivalent):

```
g++ main.cpp
```

Then, running the results `a.out` will run the shell.

### COMMANDS

Besides the basic commands, the following commands are supported:

1. `exit`: Exits the shell
2. `alarm`: The input format is strictly `alarm DD/MM/YYYY::HH/MM/SS::Message`
3. `proc`: Prints currently running processes and their resolved commands
4. `record`: Can be either `record start` or `record stop` to record the commands being run or save the recorded commands so far respectively.
5. `alarm_check`: Print currently set alarms
6. `fg`: Brings a process running in the background to foreground.
7. `export`: Allows to print all environment variables in `.bashrc` and `.myrc`. `export key=val` will create/overwrite a new environment variable with key `key` and value `value`.

### MISCELLANEOUS FILES

_If any of these files are absent, shell will create them_

1. `.myrc`: Stored in ~/Documents, this includes the `PS1` string to be used by shell and the cap on the history size named as `HIST_SIZE` (should be at least 2).
2. `.filemaprc`: Stored in ~/Documents. This will include the mappings for various file extensions and the programs to be used for executing them.
3. `.history`: Stored in ~/Documents, stores the last `HIST_SIZE` number of commands executed in this shell.
4. `alarms.txt`: Stored in ~/Documents, stores alarms in strictly one format `DD/MM/YYYY::HH:MM:SS::Message` in new lines. It can be filled in advance to manually set alarms outside of the shell. Any alarms missed will be shown in the start of the shell.
5. `record.txt`: Stored in ~/Documents, stores recorded commands.


