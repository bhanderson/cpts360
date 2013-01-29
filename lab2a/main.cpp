#include <iostream>
#include <string>
#include <vector>

using namespace::std;

typedef struct Node{
	struct Node *siblingPtr, *childPtr;
	string name;
	char type;
}Node;

Node *root, *pwd;
string input, command, pathname, dirname, basename;

void init(void);
int cmd(void);

int main(){
	cmd();
}

void init(){
	root = new Node;
	root->childPtr = 0;
	root->siblingPtr = 0;
	root->name = "/";
	root->type = 'd';
	pwd = root;
}

int cmd(void){
	cout << ("Enter a command (q to quit): ");
	vector<string> v;
	char *token;
	token = strtok(input, " ");
	while(token != NULL){
		cout << token;
		v.push_back(token);
		token = strtok(NULL, " ");
	}
