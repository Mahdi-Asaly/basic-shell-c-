//Assignement 2 in OS
//Building a simple shell

/*  Name: Mahdi Asali ID : 206331795 , Elon Avisror ID: 305370801  */

//////////////////////////
//part1
#include <unistd.h>
#include <iostream>
#include <string>     // std::string, std::to_string
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <stdio.h>
//part2
#include <sys/types.h>
#include <sys/wait.h>
#include <regex>
#include <vector>
using namespace std;
//////////////////////////

void evaluate(string,int &); //evaluating the commands from user input.
void listen(); //just listen to user!
void ParseVars(vector <string> &tokens,int &Status);
char ** Token_toArr(vector<string>& tokens); //execvp (const *file,char *)
bool external=false; //for &
/////////////////////////

int main()
{
  //evaluating is listing to user and it's inifinit till user exit.
  listen();
}


//DELETE HOME AND REPLACE TO ~
void listen()
{
    //char cmd[128];
    string cmd;
    string path;
    int Status=0; //debugger, tell us the returned value.
    cout<<"Welcome to my shell!"<<endl<<"Use -help for more commands"<<endl;
    while(1) {
      external = false; // Reset flag in the next iteration.
      path=getcwd(NULL,0); 
      path.replace(0,strlen(getenv("HOME")),"~");
      cout << "OS SHell: "<<path << "> " << flush;
      getline(cin, cmd);
      if(cin.eof()||cmd=="exit")
        break;
      evaluate(cmd,Status);
    }
    cout<<endl<<"c ya"<<endl;
}

//$?




  vector<string> split(const string &s, char delim= ' ') {
          stringstream ss(s);
          string item;
          vector<string> tokens;
          while (ss >> item) {
              tokens.push_back(item);
          }
          return tokens;
      }
//cd $HOME


void ParseVars(vector<string>& tokens,int& Status)
{ 

  //1)
  //cd Desktop
  //2)
  //cd $VAR
  //3)
  //cd $?
	// this function handles all the $ expressions //
	regex variable("\\$[_a-zA-Z][_a-zA-Z0-9]*|\\$\\?");
	smatch m; // used for regex
	string varName;
	size_t startPos;
	char* value;
	int varLength;
  
	for (unsigned int index=0;index<tokens.size();index++)
	{
    //if there are "~" in the input , we should then replace it with HOME path.
    if(tokens[index].find("~")!= string::npos)
    {
      tokens[index].replace(tokens[index].find("~"),1,getenv("HOME"));
    }
		startPos=0;
		while(regex_search(tokens[index],m,variable))
		{
			varName=m[0]; //get variable name from m
			startPos=tokens[index].find(varName); //find first index of var in tokens[i]
			varLength=varName.length(); //find length
			varName.replace(0,1,""); //remove $ from varName
			// after deleting $:
			if(varName == "?")
				tokens[index].replace(startPos,varLength,to_string(Status)); 
      else // if its a variable
			{
				value=getenv(varName.data()); // get the value of the variable
				if(value != NULL) // if there was a variable
					tokens[index].replace(startPos,varLength,value);//expand into the value of environment variable
				else // expand into empty string if there is no such environment variable
					tokens[index].replace(startPos,varLength,"");
			}
		}
	}
}


void evaluate(string input, int &Status)
{
  //if no input
  if(input.length()==0)
    return;
  pid_t pid, pid_son;
  int status;
  
  vector<string> tokens;
  tokens=split(input); //to split the input into tokens.
  ParseVars(tokens,Status); //we parse variables and replace into real env value.
  //check &
  string token=tokens[tokens.size()-1];   //cd Desktop&
  if(tokens[tokens.size()-1]=="&" || token[token.size()-1]=='&'){
    external=true;  //boolean variable for the "&" existance.
    //now we should remove the & to avoid errors in execvp
    //example: sleep 5 & 
    if(tokens[tokens.size()-1]=="&"){
        tokens.erase(tokens.begin()+tokens.size()-1); //we remove the '&' 

    }
    //example: sleep 5&
    else{
        tokens[tokens.size()-1].replace(tokens[tokens.size()-1].find("&"),1,""); 

    }
  }

  /*cout<<"tokens:"<<endl;
for(unsigned int i=0;i<tokens.size();i++)
  cout<<tokens[i]<<endl;
  */
  if(tokens[0]=="cd")
  {
    //we've already the path and it's parsed , now we should change the directory
    //note that chdir accept only const char .
    
    if(tokens.size()==1)
    {
      Status=chdir(getenv("HOME"));
      return;
    }
    //if we have ~ in the begining, we should expand into user HOME directory.
    

    //fix that , to work to every input.
    
    if(tokens[1][0]=='~')
    { 
      //cout<<"Founded ~ in the right place"<<endl;
      tokens[1].replace(tokens[1].find('~'),1,getenv("HOME"));
    }


    vector <string> copy=tokens;
    copy[0].replace(0,copy[0].size(),"");

    std::string tokens_tostring;
    for (auto const& s : copy) { tokens_tostring += s; }

    int res=chdir(tokens_tostring.c_str());
    if(res==-1)
    {
      Status=1;
      string msg="bash: cd:"+tokens_tostring;
      const char *msgg=msg.c_str();
      perror(msgg);
    }
    else{
      Status=res;
    }
    return;
  }

  else if( input =="help")
  {
    cout<<"_______________Available Commands________________"<<endl<<endl<<endl<<endl<<endl<<"-cd [option] - option may be a path or variable $var"<<endl<<endl<<endl<<"-exit    to exit from terminal"<<endl<<"Simple OS SHell Powered by Mahdi Asali And Elon Avi Sror 2018"<<endl<<endl<<endl<<endl<<endl;
  }

  //part2
  else{
    char **arrTokens=Token_toArr(tokens);
    pid=fork();
     if(pid == 0) //son
     {
       if (execvp(arrTokens[0],arrTokens) == -1)
       {
         cout << arrTokens[0] << ": command not found" << endl;
         exit(127);
       }
     }
     else{

       //father
       if(external==true)
        cout<<"["<<pid<<"]"<<endl;// means another process must continue due '&' .
       else // normal run (not background)
  			{
  				waitpid(pid,&status,0);

  				if(WIFEXITED(status)) // normal exit
  					Status = WEXITSTATUS(status); // returns the exit status of the child

  				else if (WIFSIGNALED(status)) // exit by signal
  					Status = WTERMSIG(status) + 128;
  			}
        while((pid_son = waitpid(-1,&status,WNOHANG)) > 0) // will check if any zombie-children exist
  			{
  				if (WIFSIGNALED(status)) // exit by signal
  					Status = WTERMSIG(status) + 128;

  				cout<<"["<< pid_son <<"] : exited, status="<<Status<<endl;
  				Status = 0; // exit status is reset to 0
  			}



     }
  }

  return;//stam
}
//function converts vector string to array (note that we need this because execvp accept only char * and const char *)
char ** Token_toArr(vector<string>& tokens)
{
	int length, size = tokens.size();
	char** array = new char*[size+1];
	for(int i=0;i<size;i++)
	{
		length = tokens[i].length();
		array[i] = new char[length+1];
		tokens[i].copy(array[i],length);
		array[i][length]='\0';
	}
	array[size]=NULL; // so the command 'execvp' could know when to stop
	return array;
}
