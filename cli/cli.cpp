#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

using namespace std;

#define NO_ERROR			0
#define NO_ALG_WAS_CHOSEN	1
#define FORK_FAILED			2
#define BAD_INPUT_PARAMS	3
#define COMBINATION_FAILED	4
#define EXEC_FAILED			5
#define PIPE_ERROR			6
#define MEMORY_ERROR		7

#define NO_ALG_TYPE				0
#define OWCTY_TYPE				1
#define DISTR_MAP_TYPE			2
#define OWCTY_REVERSED_TYPE		3


void version()
{
  cout <<"      NewBioDiVinE ";
  cout <<"version 1.0 build 1  (2014/May/5 16:00)"<<endl;
}


void cli_usage()
{
  cout <<"-----------------------------------------------------------------"<<endl;
  cout <<"                    BioDiVinE Tool Set"<<endl;
  cout <<"-----------------------------------------------------------------"<<endl;
  version();
  cout <<"-----------------------------------------------------------------"<<endl;

  cout <<"Usage: newbiodivine [-h, --help] [algorithm] [options] model_file.bio property_file.ltl\n"<<endl;
  
  cout <<"Algorithm:\n";
  cout <<"    owcty\n";
  cout <<"    distr_map\n";
  cout <<"    owcty_reversed\n\n";
  
  cout <<"Options: "<<endl;
  cout <<"  For owcty:\n";
  cout <<"      -v,--version       show version"<<endl;
  cout <<"      -h,--help          show this help"<<endl;
  cout <<"      -H x,--htsize x    set the size of hash table to ( x<33 ? 2^x : x )"<<endl;
  cout <<"      -V,--verbose       print some statistics"<<endl;
  cout <<"      -q,--quiet         quite mode"<<endl;
  cout <<"      -f, --fast         use faster but less accurate algorithm for abstraction" << endl;  
  cout <<"      -c, --statelist    show counterexample states"<<endl;
  cout <<"      -C x,--compress x  set state compression method"<<endl;
  cout <<"                         (0 = no compression, 1 = Huffman static compression)"<<endl;
  cout <<"      -t,--trail         produce trail file"<<endl;
  cout <<"      -r,--report        produce report file"<<endl;
  cout <<"      -s,--simple        perform simple reachability only"<<endl;
  cout <<"      -L,--log           produce logfiles (log period is 1 second)"<<endl;
  cout <<"      -X w               sets base name of produced files to w (w.trail,w.report,w.00-w.N)"<<endl;
  cout <<"      -Y                 reserved for GUI"<<endl;
  cout <<"      -Z                 reserved for GUI"<<endl;
  cout << endl;
  
  cout <<"  For distr_map:\n";
  cout <<"      -v, --version      show distr_map version"<<endl;
  cout <<"      -h, --help         show this help"<<endl;
  cout <<"      -r, --report       produce report (file.distr_map.report)"<<endl;
  cout <<"      -t, --trail        produce trail file (file.distr_map.trail)"<<endl;
  cout <<"      -c, --statelist    show counterexample states"<<endl;
  cout <<"      -f, --fast         use faster but less accurate algorithm for abstraction" << endl;
  cout <<"      -H x, --htsize x   set the size of hash table to ( x<33 ? 2^x : x )"<<endl;
  cout <<"      -S, --printstats   print some statistics"<<endl;
  cout <<"      -q, --quiet        quiet mode (do not print anything - overrides all except -h and -v)"<<endl;
  cout <<"      -L                 perform logging"<<endl;
  cout <<"      -X w               sets base name of produced files to w (w.trail,w.report,w.00-w.N)"<<endl;
  cout <<"      -Y                 reserved for GUI"<<endl;
  cout <<"      -Z                 reserved for GUI"<<endl;  
  cout << endl;
  
  cout <<"  For owcty_reversed:\n";
  cout <<"      -v, --version      show version"<<endl;
  cout <<"      -h, --help         show this help"<<endl;
  cout <<"      -H x, --htsize x   set the size of hash table to ( x<33 ? 2^x : x )"<<endl;
  cout <<"      -V, --verbose      print some statistics"<<endl;
  cout <<"      -q, --quiet        quite mode"<<endl;
  cout <<"      -c, --statelist    show counterexample states"<<endl;
  cout <<"      -t, --trail        produce trail file"<<endl;
  cout <<"      -f, --fast         use faster but less accurate algorithm for abstraction" << endl;
  cout <<"      -r, --report       produce report file"<<endl;
  cout <<"      -R, --remove       removes transitions while backpropagating (very slow, saves memory)" << endl;
  cout <<"      -s, --simple       perform simple reachability only"<<endl;
  cout <<"      -L, --log          produce logfiles (log period is 1 second)"<<endl;
  cout <<"      -X w               sets base name of produced files to w (w.trail,w.report,w.00-w.N)"<<endl;
  cout <<"      -Y                 reserved for GUI"<<endl;
  cout <<"      -Z                 reserved for GUI"<<endl;
  cout <<"-----------------------------------------------------------------"<<endl;
}


void maintenance(char ** array) {
/*
	for(int i = 0; array[i] != NULL; i++) {
		delete array[i];
	}
*/
	delete[] array;
}



int main(int argc, char** argv) {
	
	bool dbg = true;

	if( argc < 4 || (argc >= 2 && (string(argv[1]).compare("-h") == 0 || string(argv[1]).compare("--help") == 0) ) ) {
		cli_usage();
		return BAD_INPUT_PARAMS;
	}
	
	cout << "=========================================\n";
	cout << "------- Welcome to NewBioDiVinE ---------\n";
	cout << "=========================================\n\n";
	
	string propertyFile(argv[argc-1]);
	string modelFile(argv[argc-2]);
	
	if(modelFile.compare(modelFile.size()-4,string::npos,".bio") != 0) {
		cerr << "\nError: Incorrect model file. It has to be \"*.bio\" file\n";
		return BAD_INPUT_PARAMS;
	}
	
	if(propertyFile.compare(propertyFile.size()-4,string::npos,".ltl") != 0) {
		cerr << "\nError: Incorrect property file. It has to be \"*.ltl\" file\n";
		return BAD_INPUT_PARAMS;
	}
	
	string callingPath = string(argv[0]).substr(0,string(argv[0]).size()-12);
	if(dbg) cout << "calling path: " << callingPath << "\n";
	
	int pid1;
	int fd[2];
	vector<string> inputFiles;
	string programWithPath = callingPath + "divine.combine";
	if(dbg) cout << "Call combine: " << programWithPath << endl;
	
	if (pipe (fd) == -1) {
		cerr << "\nError: Pipe wasn't created\n";
		return PIPE_ERROR;
	}
	
	switch(pid1 = fork()) {
		case 0:
			close(fd[0]);
			dup2(fd[1],STDOUT_FILENO);
			close(fd[1]);
			execl(programWithPath.c_str(),"divine.combine",modelFile.c_str(),propertyFile.c_str());
			_exit(EXEC_FAILED);
			
		case -1:
			cerr << "\nError: fork() for combine failed\n";
			exit(FORK_FAILED);
			
		default:
			int status;
			
			close(fd[1]);
			dup2(fd[0],STDIN_FILENO);
			close(fd[0]);
			
			waitpid(pid1,&status,0);
			if(status == 0) {
				
				string file;
				while(cin.good() && !cin.eof()) {
					cin >> file;
					if(file.size() > 4 && file.compare(file.size() - 4, string::npos, ".bio") == 0) {
						if(dbg) cout << "File for processing: " << file << endl;
						inputFiles.push_back(file);
					}
				}
				
			} else {
				cerr << "\nError: Some error occured while combination of model and property was runnig\n";
				return COMBINATION_FAILED;
			}
	}
	
	if(inputFiles.empty()) {
		cerr << "\nError: No file to processing was found\n";
		return COMBINATION_FAILED;
	}
	
	
	int algorithm_type = NO_ALG_TYPE;
	int newArgc = argc - 1;
	if(dbg) cout << "size of newArgv with terminating char: " << newArgc << endl;	//only for testing-----------------
	
	char ** newArgv = new char* [newArgc];
	if(newArgv == nullptr) {
		cerr << "\nError: Memory couldnot be allocated\n";
		return MEMORY_ERROR;
	}
	
	int i = 0;
	for(i = 1; i < argc-2; i++) {
		newArgv[i-1] = argv[i];
		//cout << "newArgv[" << i-1 << "]: " << newArgv[i-1] << endl;	// only for testing-----------------
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	// Here starts cycle for all previously generated files, that will be input files for chosen algorithm
	for(int a = 0; a < inputFiles.size(); a++) {
	
		stringstream ss(inputFiles.at(a));	
		char name[100];
		ss >> name;
	
		cout << "============ PROCESSING " << name << " ============\n";
		
		i--;	
		newArgv[i++] = name;
		newArgv[i] = NULL;

		// for testing-----------------------
		if(dbg) {
			int s= 0;
			while(newArgv[s] != NULL) {
				cout << "new input argument of index " << s << ":" << newArgv[s] << endl;
				s++;
			}
		}
		// end of testing----------------
	

		if(string(argv[1]).compare("owcty") == 0) {
			algorithm_type = OWCTY_TYPE;
		}
		if(string(argv[1]).compare("distr_map") == 0) {
			algorithm_type = DISTR_MAP_TYPE;
		}
		if(string(argv[1]).compare("owcty_reversed") == 0) {
			algorithm_type = OWCTY_REVERSED_TYPE;
		}
	
		if(algorithm_type == NO_ALG_TYPE) {
			cerr << "\nError: No relevant algorithm was chosen\n";
			maintenance(newArgv);
			return (NO_ALG_WAS_CHOSEN);
		}
		
		programWithPath = callingPath + "divine." + newArgv[0];
		if(dbg) cout << "Call algorithm: " << programWithPath << endl;
		cout << "============ CALLING " << newArgv[0] << " ============\n";
		cout.flush();

		int pid;
		switch(pid = fork()) {
	
			case 0:		// in the Children process
				execv(programWithPath.c_str(),newArgv);
				maintenance(newArgv);
				_exit(EXEC_FAILED);	
			
			case -1:	// fork error
				cerr << "\nError: fork() for algorithm failed\n";
				maintenance(newArgv);
				exit(FORK_FAILED);
			
			default:	// in the Parent process
				int status;
				waitpid(pid,&status,0);
				switch(status) {
					case NO_ERROR:
						cout << "============ " << newArgv[0] << " DONE ============\n\n";
						break;
					case NO_ALG_WAS_CHOSEN:
					case EXEC_FAILED:
					case FORK_FAILED:
						cerr << "\nError: Some error occured while model checking was running\n";
						break;
				}
		}
	
		//TODO: maybe remove the combined file then
		

	} //Here ends cycle for all previously generated files, that was input files for chosen algorithm
	//-----------------------------------------------------------------------------------------------


	cout << "==================================================\n";
	cout << "--------- NewBioDiVinE successfully finish ---------\n";
	cout << "==================================================\n\n";
						
	maintenance(newArgv);
	return NO_ERROR;
}
