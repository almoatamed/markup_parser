#include <vector>
#include <iostream>
#include <regex>
#include <string> 
using namespace std;

class param
{
public:
  string name;
  string value;
};

class tag
{
  vector<tag> children;
  vector<param> params;

  tag(): children(), params() {} 
};

vector<tag> markup_file;



int main()
{
  vector<string> markup_lines(0);
  int n,q;
  scanf("%d %d \n",&n,&q);
  string temp = "";
  for(int i =0; i<n; i++){
    getline(cin,temp);
    markup_lines.push_back(temp);
  }
  for( int i=0; i<n; i++){
    // cout << markup_lines[i]<<endl;
  }
  return 0;
}
