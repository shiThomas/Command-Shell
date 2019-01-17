#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include "methods.h"
void print_prompt() {
  std::cout << "myShell:" << getenv("PWD") << " $ ";
}

int check_exit(std::vector<std::string> parsed_command) {
  int check;
  //Check if EOF is reached
  if (std::cin.eof()) {
    check = 0;
  }
  //only check argv[0] only if size !=0
  if (parsed_command.size() != 0) {
    if (parsed_command[0] == "exit") {
      check = 0;
    }

    else {
      check = 1;
    }
  }
  return check;
}

/*Here I check if my argv[0] is command not in exceve
  if true go to excute function I wrote*/
bool check_function(std::vector<std::string> parsed_command,
                    std::string command_input,
                    std::map<std::string, std::string> & map_var) {
  if (parsed_command[0] == "cd") {
    run_cd(parsed_command);
  }
  else if (parsed_command[0] == "set") {
    run_set(parsed_command, command_input, map_var);
  }
  else if (parsed_command[0] == "export") {
    run_export(parsed_command, map_var);
  }

  else if (parsed_command[0] == "inc") {
    inc(map_var, parsed_command);
  }
  else {
    return false;
  }

  return true;
}

int main() {
  std::string command_input;
  std::vector<std::string> argv_temp;
  std::vector<std::string> argv;
  std::map<std::string, std::string> map_var;
  std::vector<std::string> argv_replace;
  //initilize map;
  setenv("ECE551PATH", getenv("PATH"), 1);
  map_var = map_init();
  while (1) {
    print_prompt();
    //initlaize command input to empty;
    command_input = "";
    getline(std::cin, command_input);
    //this is parsed commands in vector
    argv_temp = parse_command(command_input);

    //only exit if check_exit returns 0
    if (check_exit(argv_temp) == 0) {
      return 0;
    }
    else {
      //jump to start of the loop if size =0
      if (argv_temp.size() == 0) {
        continue;
      }
      //replace command if needed
      argv_replace = evaluate(map_var, argv_temp);
      if (check_function(argv_replace, command_input, map_var) == false) {
        //if check_function returns true, jump to commands "cd, set, export, inc"
        argv = getpath(argv_replace);
        //for testing full path
        execute(argv);
      }
    }
  }
}
