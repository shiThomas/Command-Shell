#ifndef methods_H
#define methods_H
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

//the following fucntion is for execution with no path input:  GetPath and parse_path;
std::vector<std::string> parse_path(std::vector<std::string> path_raw_string);

/*Here I check if files are contained in each blocks of vector<string>, return the one that has the file
  empty the vector string, only keep the desired string

*/
//This method push items onto map;
std::map<std::string, std::string> map_push(std::string var, std::string val) {
  std::map<std::string, std::string> map_var;
  map_var[var] = val;
  return map_var;
}

/*parse elemtn acquired from environ into two parts: var and val
push every element onto vars map for futural operations

*/

std::map<std::string, std::string> map_init() {
  std::map<std::string, std::string> map_var;
  std::string delimeter = "=";
  size_t pos;
  char ** envlist = environ;

  while (*envlist != NULL) {
    std::string envlist_string(*envlist);
    if ((pos = envlist_string.find(delimeter)) != std::string::npos) {
      //set element before delimeter to variable name ;
      std::string var = envlist_string.substr(0, pos);
      //set element after to value;
      std::string val = envlist_string.substr(pos + 1);
      map_var[var] = val;
    }
    envlist++;
  }
  return map_var;
}

//Check if the variable name only contains alphebetical characters or underscores
bool check_valid(std::string var) {
  for (std::string::iterator it = var.begin(); it != var.end(); it++) {
    if (!isalnum(*it) && (*it) != '_') {
      return false;
    }
  }
  return true;
}
/*
This method check if files is in the current directory.
I have parsed path string, which contains multiple path string.
find files iterate through each element, and find the one that
have the file, and return that path.

If path is not find, then path string is set to empty string.
 */
std::string find_files(std::string argv_0, std::vector<std::string> parsed_path_string) {
  //  const char * argv0 = new char[argv_0.length() + 1];

  std::string result;
  std::vector<std::string> copy_path = parsed_path_string;
  size_t index = 0;
  struct dirent * dir;

  while (index != copy_path.size()) {
    DIR * d = opendir(parsed_path_string[index].c_str());
    if (d == NULL) {  //not sure if I need this error check
      perror("Open DIR error");
    }
    while ((dir = readdir(d)) != NULL) {
      if (strcmp(dir->d_name, argv_0.c_str()) == 0) {
        closedir(d);
        //example ls: /bin + '/'+ ls
        result = parsed_path_string[index] + "/" + argv_0;

        return result;
      }
    }
    closedir(d);
    //go to the next index of vector that contains path
    index++;
  }
  //return empty string, if empty string is placed inside
  return "";
}
//use std::vector to eliminate effect of memory leak
std::vector<std::string> getpath(std::vector<std::string> argv_string) {
  char * path_raw;
  std::vector<std::string> argv_temp;
  std::vector<std::string> path_processed_string;
  std::vector<char *> argv_processed;
  std::vector<std::string> path_raw_string;
  for (std::vector<std::string>::iterator it = argv_string.begin(); it != argv_string.end(); ++it) {
    argv_temp.push_back(&(*it)[0]);
  }

  std::string path = argv_string[0];
  //if no '/' appears
  if (path.find('/') == std::string::npos) {
    //originally set to PATH, ECE551PATH is set when myshell is initialized.
    path_raw = getenv("ECE551PATH");
    if (path_raw == NULL) {
      std::cout << "Command" << path << " not found" << std::endl;
    }
    else {
      std::string path_str(path_raw);
      path_raw_string.push_back(path_str);
      //get the parsed address, and place into index one of argv_processed.
      path_processed_string = parse_path(path_raw_string);
      argv_temp[0] = find_files(path, path_processed_string);
    }
  }

  return argv_temp;
}
/*convert char* to string for easier operation,
 then check for ":", and push back to std::vector<string>
*/
std::vector<std::string> parse_path(
    std::vector<std::string> path_raw_string) {  //checked corectness

  std::vector<std::string> path_processed_string;
  std::string delimiter = ":";
  std::string token;
  size_t pos;

  while ((pos = path_raw_string[0].find(delimiter)) != std::string::npos) {
    //acquire string before ':'
    token = path_raw_string[0].substr(0, pos);
    //place string into string array
    path_processed_string.push_back(token);
    //delete things including the first ':'
    path_raw_string[0].erase(0, pos + 1);
  }  //check empty string
  if (path_raw_string[0].length() != 0) {
    path_processed_string.push_back(path_raw_string[0]);
  }
  return path_processed_string;
}
//check if a string is \\ or " "
bool Isspaceorslash(std::string input) {
  if (input != "\\" && input != " ") {
    return true;
  }
  else {
    return false;
  }
}

/* This method parse command regarding different situation when seeing current as " ",
 "\\" or others.
Each time, I check one element ahead to determine what is the current behavior


*/
std::vector<std::string> parse_command(std::string command_input) {  //correct
  std::vector<std::string> argv_string;
  std::string command_curr = "";
  /*I check whether first element is character other than \\ and " "
    perform nothting if is not*/
  if (Isspaceorslash(command_input.substr(0, 1))) {
    command_curr += command_input.substr(0, 1);
  }

  for (size_t i = 1; i < command_input.length(); i++) {
    //current digit is " "
    if (command_input.substr(i, 1) == " ") {
      if (command_input.substr(i - 1, 1) == " ") {
        //this condition considers \\+space+space
        if (i >= 2 && command_input.substr(i - 2, 1) == "\\") {
          argv_string.push_back(command_curr);
          //empty that string to avoid unwanted behavior
          command_curr = "";
        }
      }

      else if (command_input.substr(i - 1, 1) == "\\") {
        command_curr += " ";
      }

      //previous item is actual string
      else {
        argv_string.push_back(command_curr);
        command_curr = "";
      }
    }
    //current digit is "\\"
    else if (command_input.substr(i, 1) == "\\") {
      {  // "\\"+"\\"
        if (command_input.substr(i - 1, 1) == "\\") {
          command_curr += "\\";
          if (i == command_input.length() - 1) {
            argv_string.push_back(command_curr);
            command_curr = "";
          }
        }
      }
    }
    //current digit is other charactre
    else {
      //see things other than ' ' or "\\", simply push that thing into vector
      command_curr += command_input.substr(i, 1);
      //check last digit
      if (i == command_input.length() - 1) {
        argv_string.push_back(command_curr);
        command_curr = "";
      }
    }
  }
  return argv_string;
}

void child(std::vector<std::string> argv_temp) {
  std::vector<char *> argv;
  /*convert std::vector<string> to std::vector<char*>
    This prevent memory leak*/
  for (std::vector<std::string>::iterator it = argv_temp.begin(); it != argv_temp.end(); ++it) {
    argv.push_back(&(*it)[0]);
  }
  //last element is NULL;
  argv.push_back(NULL);
  execve(argv[0], &argv[0], environ);
  perror("execve");
  return;
}

void execute(std::vector<std::string> argv_temp) {
  pid_t cpid, w;
  int status;
  //check if directory is valid
  std::string arg_print = argv_temp[0];
  if (access(argv_temp[0].c_str(), 1) == -1) {
    std::cout << "Command not found" << std::endl;
    return;
  }

  cpid = fork();
  if (cpid == -1) {
    perror("fork");
    return;
  }

  /* Code executed by child */
  if (cpid == 0) {
    child(argv_temp);
  }
  /* Code executed by parent */
  else {
    do {
      w = waitpid(cpid, &status, WUNTRACED | WCONTINUED);
      if (w == -1) {
        perror("waitpid");
        return;
      }

      if (WIFEXITED(status)) {
        printf(" Program exited with status %d\n", WEXITSTATUS(status));
      }
      else if (WIFSIGNALED(status)) {
        printf("Program was killed by signal %d\n", WTERMSIG(status));
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
}

/*this function perform if argment size =1 and 2
size =1 is for HOME, 2 is for directory change
*/

void run_cd(std::vector<std::string> parsed_command) {
  std::string dest1;
  std::string dir_curr;
  //check single input cd
  if (parsed_command.size() == 1) {
    if (chdir(getenv("HOME")) != 0) {
      perror("cannot open home");
    }
    else {
      //change current directory to HOME
      std::string dest(getenv("HOME"));
      dir_curr = dest;
    }
  }
  //check two inputs
  else if (parsed_command.size() == 2) {
    dest1 = parsed_command[1];
    //check if able to open directory
    if (chdir(dest1.c_str()) != 0) {
      perror(" cannot open directory");
      return;
    }
    else {
      char cwd[CHAR_MAX];
      getcwd(cwd, CHAR_MAX);
      std::string dest2(cwd);
      dir_curr = dest2;
    }
  }

  //only accept 2 element input
  else if (parsed_command.size() > 2) {
    perror("too many arguments");
    return;
  }
  /*set original PWD value to OLDPWD
  and replace PWD with current directory
  */
  setenv("OLDPWD", getenv("PWD"), 1);
  if (setenv("PWD", dir_curr.c_str(), 1) != 0) {
    perror("cannot change env");
    return;
  }
}

/*

In my case, I need to declare a map in main to store all the var and val;
run_set push values onto a map for furtural use;
*/
void run_set(std::vector<std::string> parsed_command,
             std::string command_string,
             std::map<std::string, std::string> & map_var) {
  size_t pos_set;
  size_t pos_var;
  size_t pos_val;
  size_t pos_val_temp;
  size_t pos_set_end_plus1;
  size_t pos_var_end_plus1;
  std::string var;
  std::string val;
  //size has to be 3
  var = parsed_command[1];
  if (parsed_command.size() < 3) {
    perror("too few arguments for set");
    return;
  }

  if (!check_valid(var)) {
    perror("invalid variable name");
    return;
  }
  //eliminate influence of " ", get starting position of set command;
  pos_set = command_string.find(parsed_command[0]);
  //get the end index of set command.
  pos_set_end_plus1 = pos_set + parsed_command[0].length();
  pos_var = command_string.find(parsed_command[1], pos_set_end_plus1);
  //where variable name ends
  pos_var_end_plus1 = pos_var + parsed_command[1].length();
  //Starting position of value
  pos_val_temp = pos_var_end_plus1 + 1;
  pos_val = command_string.find_first_not_of('_', pos_val_temp);
  //including everthing after argv[1] including whitespace
  val = command_string.substr(pos_val);

  std::map<std::string, std::string>::iterator it = map_var.find(var);
  //if exist, change that value with a new value
  if (it != map_var.end()) {
    map_var[var] = val;
  }
  else {
    //push two values onto map
    map_var[var] = val;
  }
}
/*Export value to env*/
void run_export(std::vector<std::string> parsed_command,
                std::map<std::string, std::string> map_var) {
  //variable name;
  std::string var;
  //value
  std::string val;
  if (parsed_command.size() < 2 || parsed_command.size() > 2) {
    perror("Invalid arguments amount!");
    return;
  }
  var = parsed_command[1];
  if (!check_valid(var)) {
    perror("Invalid variable name!");
    return;
  }

  std::map<std::string, std::string>::iterator it = map_var.find(var);
  if (it != map_var.end()) {
    val = it->second;
  }
  else {
    perror("cannot find value");
    return;
  }  //Here I export value to varible name and push to env
  if (setenv(var.c_str(), val.c_str(), 1) != 0) {
    perror("Fail to export env");
    return;
  }
}
/* this function, I search map for certain var, and acquire value.*/
std::string search_map(std::map<std::string, std::string> map_var, std::string var) {
  std::string value;
  std::map<std::string, std::string>::iterator it = map_var.find(var);
  if (it != map_var.end()) {
    value = it->second;
  }
  else {
    value = "";
  }

  return value;
}
/* Pass parsed command, 
   for each '$', get value from map,
   can be $arv1$arv2
   and return the new command vector.*/
//add if need to evaluate in main function;

std::vector<std::string> evaluate(std::map<std::string, std::string> & map_var,
                                  std::vector<std::string> parsed_command) {
  std::string result_command;
  std::string command_cpy;
  size_t pos;
  size_t end;
  std::string var;
  std::string token;
  std::vector<std::string> result_vector;

  for (size_t i = 0; i < parsed_command.size(); i++) {
    command_cpy = parsed_command[i];
    result_command = "";
    //if $ exists
    if (command_cpy.find("$") != std::string::npos) {
      while ((pos = command_cpy.find("$")) != std::string::npos) {
        //if position not 0,push and  delete everything before that $, and continue
        if (pos != 0) {
          token = command_cpy.substr(0, pos);
          result_command += token;
          command_cpy.erase(0, pos);
        }
        else {
          //if position is 0, find next $, and get substring
          if ((end = command_cpy.find("$", pos + 1)) != std::string::npos) {
            //get variable name
            var = command_cpy.substr(1, end - 1);
            token = search_map(map_var, var);
            result_command += token;
            //erase stops before $
            command_cpy.erase(0, end);
          }
          //no more $ is found;
          else {
            size_t len = command_cpy.length();
            //check if last one is '-'
            if ((command_cpy[len - 1]) == '-') {
              var = command_cpy.substr(1, len);
              token = search_map(map_var, var);
              result_command += token;
              result_command += '-';
              command_cpy = "";
            }
            else {
              var = command_cpy.substr(1);
              token = search_map(map_var, var);
              result_command += token;

              command_cpy = "";
            }
          }
        }
      }
    }
    else {
      result_command = parsed_command[i];
    }

    result_vector.push_back(result_command);
  }
  return result_vector;
}
/* this function is a helper function for addition when 
   number is negative*/
int nega_add(std::string val) {
  int result;
  int result_temp = atoi(val.substr(1).c_str());
  if (result_temp == 1) {
    result = 0;
  }
  else {
    result = result_temp - 1;
  }
  return result;
}
//change int type to string type
std::string itos(int input) {
  std::string result_string;
  std::ostringstream s;
  s << input;
  const std::string i_as_string(s.str());
  result_string = i_as_string;
  return result_string;
}

/*for each var in map
  check if its value is a base10 number, or if var is in the map
  add 1 to that value, or chagne it to 1 if conditions not matched.
  check if is negative, fist check first element is -, if so,
  check every digit after it using isdigit.

*/
void inc(std::map<std::string, std::string> & map_var, std::vector<std::string> parsed_command) {
  std::string var;
  std::string val;
  int result;
  //  int result_temp;
  std::string result_string;
  var = parsed_command[1];
  std::map<std::string, std::string>::iterator it = map_var.find(var);
  if (it != map_var.end()) {
    val = it->second;

    if (isdigit(val[0])) {
      for (size_t i = 1; i < val.length(); i++) {
        //1-23 is not valid
        if (!isdigit(val[i])) {
          map_var[var] = "1";
          return;
        }
        //is digit
      }
      //change string to int and increment by 1
      result = atoi(val.c_str()) + 1;
      //change int back to string
      map_var[var] = itos(result);

      return;
    }
    else if (val.substr(0, 1) == "-") {
      for (size_t i = 1; i < val.length(); i++) {
        if (!isdigit(val[i])) {
          map_var[var] = "1";
          return;
        }
        //is digit
      }
      int result = nega_add(val);
      //change int back to string
      result_string = itos(result);
      //if value =0, should not be -0
      if (result_string != "0") {
        map_var[var] = "-" + result_string;
      }
      else {
        map_var[var] = result_string;
      }
      return;
    }

    else {
      //first digit is not '-' or number
      map_var[var] = "1";
      return;
    }
  }

  else {
    //value not exist in the map
    map_var[var] = "1";
    return;
  }
}
#endif
