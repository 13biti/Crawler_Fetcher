#include <stdlib.h>
#include <iostream>
#include <string>
#include <regex>
#include <fstream>

using namespace std; 
int main (){
/*
	ifstream filee("DataBaseLinkFile.txt");
	string line ;
	while (getline(filee , line)){
		if (regex_match (line , regex(".*10000000.*"))){
			cout<<"regex working"<<endl;
		}
	}
	filee.close();	

	return 0;
	*/
	/*
 
	while (1){
	*/
	int x ;
	cin >> x ;
	string pattern = " \".*10000000.*\" ";
	string commond ="grep  ";
	
	//int num = system(commond);
	//const char* finalcommond  = (commond + to_string(x)).c_str();
	const char* finalcommond  = (commond + pattern + "| echo ").c_str();
	
	string resoult = system(finalcommond);
	cout<<resoult;
	
	//cout << "hello";
	
//*/

}
