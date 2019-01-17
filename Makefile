all: test_myshell runtest
test_myshell: myShell.cpp methods.h 
	g++ -ggdb3 -Wall -Werror -pedantic -std=gnu++98 -o test_myshell myShell.cpp methods.h 

runtest: runtest.cpp 
	g++ -ggdb3 -Wall -Werror -pedantic -std=gnu++98 -o runtest runtest.cpp 


	
