#include <vector>
#include <iostream>
#include <regex>
#include <string>
// #include <boost/algorithm/string.hpp>
using namespace std;

struct string_finding
{
  string value;
  int start_index;
  int length;
};

struct param
{
  string name;
  string value;
};

class tag_error
{
public:
  string msg;
  string what()
  {
    return msg;
  }
  tag_error(string &msg)
  {
    this->msg = msg;
  }
};


enum type_enum
{
  tag_enum_type = 1,
  text_enum_type = 2
};

template <class T>
class content_object
{
public:
  type_enum type;
  string text;
  T tag;
  content_object(): tag(){}
};

class tag_object
{
public:
  vector<param> params;
  vector<content_object<tag_object>> children;
  string name;
  string source;
  int scanner_index;

  tag_object(): children(0), params(0){}

  bool findall(regex &expression, string &text, vector<string_finding> &findings, int start_index = -1, int length = -1)
  {

    findings.clear();

    if (start_index == -1 && length == -1)
    {
      length = text.length();
      start_index = 0;
    }
    // printf("findall: %d %d are the starting and ending searching ranges\n", start_index, length );
    // printf("findall: %s is the string provided\n", text.data());
    string scanning_string = text.substr(start_index, length);
    smatch res;
    while (regex_search(scanning_string, res, expression))
    {
      string_finding finding;

      finding.value = res.str();
      finding.start_index = start_index + res.position();
      finding.length = res.str().length();

      findings.push_back(finding);

      start_index += res.str(0).length() + res.position(0);
      if (start_index >= text.length())
      {
        return true;
      }
      scanning_string = text.substr(start_index);
      // // cout<< "findall: result found while searching on index "<<start_index<< "  the result string is "<< res.str()<<" of length "<< res.str().size() <<endl;
    }
    if (findings.size() == 0)
    {
      return false;
    }
    else
    {
      return true;
    }
  }

  void find_starting_tag()
  {
    string to_be_scanned = this->source.substr(this->scanner_index);
    regex find_tagname_regex("<[a-z_A-z0-9\\-]+");
    smatch res;
    if (regex_search(to_be_scanned, res, find_tagname_regex))
    {
      this->name = res.str(0).substr(1);
      this->scanner_index += res.position() + res.str().length();
      // printf("this tag is named %s and starts at index %d", this->name.data(), this->scanner_index);
    }
    else
    {
      string msg = string("cannot find starting tag");
      throw tag_error(msg);
    }
  }
  
  string removeSpaces(string str)
  {
      stringstream s(str);
      string temp;
      str = "";
      while (getline(s, temp, ' ')) {
          str = str + temp;
      }
      return str;
  }
  
  void append_param(string_finding &finding)
  {
    regex find_value_regex("[a-z_A-Z][a-z_A-Z0-9\\-]*=\\\"");
    finding.value =this->removeSpaces(finding.value);
    finding.length = finding.value.length();
    // cout<<"appending paramters from finding"<< finding.value<<endl;
    vector<string_finding> findings(0);
    if (findall(find_value_regex, finding.value, findings))
    {
      param temp;
      temp.name = this->removeSpaces(findings[0].value.substr(0, findings[0].length - 2));
      // cout<< "the finding value and length compared "<< finding.value.length() <<finding.length;
      temp.value = finding.value.substr(findings[0].start_index + findings[0].length, finding.length - 1 - findings[0].start_index - findings[0].value.length());
      this->params.push_back(temp);
    }
    else
    {
      string msg("could not find param name while adding param");
      throw tag_error(msg);
    }
  }

  void find_params()
  {
    string to_be_scanned = this->source.substr(this->scanner_index);

    regex ending_of_tag(">");
    smatch ending_range_res;
    int range_end_length = to_be_scanned.length();

    if(regex_search(to_be_scanned, ending_range_res, ending_of_tag)){
      range_end_length = ending_range_res.position();
    }else{
      string message = "no ending of tag found";
      throw tag_error(message);
    }

    to_be_scanned = this->source.substr(this->scanner_index, range_end_length);

    regex find_vals_regex("[a-z_A-Z][a-z_A-Z0-9\\-]*\\s*=\\s*\\\"[^\\\"]*\\\"");
    vector<string_finding> findings(0);

    if (findall(find_vals_regex, to_be_scanned, findings))
    {
      for (int i = 0; i < findings.size(); i++)
      {
        append_param(findings[i]);
      }
      this->scanner_index += findings.back().length + findings.back().start_index;
      // cout<<endl<< "scanner index after finding params is "<< this->scanner_index<<endl;
    }else{
      // cout<<"find_params: no params found"<<endl;
    }
  }

  bool find_start_tag_end(){
    string to_be_scanned = this->source.substr(this->scanner_index);
    regex find_ending_of_tag(">");
    smatch res;
    if(regex_search(to_be_scanned, res, find_ending_of_tag)){
      this->scanner_index += res.position() + 1;
      // cout<< "the last character before the ending of the start tag"<< this->source.substr(this->scanner_index - 2,2)<<endl;
      if(this->source.at(this->scanner_index - 2) == '/'){
        return false;      
      }else{
        return true;
      }
    }else{
      string message = "No closing for starting tag";
      throw tag_error(message);
    }
  }

  void print_content()
  {
    // printf("\nthis is %s tag\n\n", this->name.data());
    for (int i = 0; i < params.size(); i++)
    {
      // printf("parameter: {%s}  with value of: {%s} \n", params[i].name.data(), params[i].value.data());
    }
    if(children.size() >0){
      // printf("this tag children are:\n");
      for ( int i = 0; i< children.size(); i++){
        if(children[i].type == text_enum_type){
          // printf("child: of type text and its content is  {%s}\n", children[i].text.data());
        }else{
          // printf("child: of type tag and its name is {%s}\n", children[i].tag.name.data());
        }
      }
    }else{
      // printf("this tag has no children\n");
    }
  }
  
  void find_text_content(){
    string to_be_scanned =  this->source.substr(this->scanner_index);
    regex find_text_content_regex("^[^<]+");
    // // cout<< "searching for content in "<< to_be_scanned<< endl;
    vector<string_finding> findings(0);
    if(this->findall(find_text_content_regex, to_be_scanned, findings)){
      content_object<tag_object> temp;
      // cout<< " findings on text content: size and text are: "<< findings.size()<< "  \""<<findings[0].value<<"\""<<endl;
      temp.type = text_enum_type;
      temp.text = findings[0].value;
      this->children.push_back(temp);
      this->scanner_index += findings[0].start_index + findings[0].length;
    }else{ 
      // cout<< "no text content found"<<endl;
    }
  }

  bool find_child_tag(){
    string to_be_scanned = this->source.substr(this->scanner_index);

    // cout<<endl<<endl<< "find_child_tag: attempting to find the next tag "<<endl;
    // cout<< "find_child_tag: the proviced string is "<< to_be_scanned<<endl;
    
    regex find_next_opening_tag_regex("<");
    smatch res;

    if(regex_search(to_be_scanned, res, find_next_opening_tag_regex)){
      
      if(to_be_scanned.substr(res.position(),2) == "</"){
        // cout<< "find_child_tag: a closing tag found ( assuming its for the current tag) returning"<<endl;
        this->scanner_index += res.position();
        return true;
      }else{
        // cout<< "find_child_tag: a child tag found"<<endl;
        content_object<tag_object> temp;
        temp.type = tag_enum_type;
        temp.tag.from_string(to_be_scanned);
        this->children.push_back(temp);
        this->scanner_index += temp.tag.scanner_index;
        // cout<< "find_child_tag: a child tag added, current scanning index is "<<this->scanner_index<<endl;
        return false;
      }
    }else{
      string message("error while trying to find the children tag");
      throw tag_error(message);
    }
  }

  void find_content(){
    bool closing_tag_found = false;
    while(!closing_tag_found){
      this->find_text_content();
      // closing_tag_found = true;
      if(this->find_child_tag()){
        closing_tag_found = true;
      };
    }
  }

  void find_closing_tag_end(){
    string to_be_scanned = this->source.substr(this->scanner_index);
    regex find_ending_of_tag(">");
    smatch res;
    if(regex_search(to_be_scanned, res, find_ending_of_tag)){
      this->scanner_index += res.position() + 1;
    }else{
      string message = "No ending for closing tag";
      throw tag_error(message);
    }
  }
  
  void find_closing_tag(){
    string to_be_scanned = this->source.substr(this->scanner_index);
    string expression = "</"+this->name;
    // cout<< "find_closing_tag: looking for closing tag "<< expression<<endl;
    regex find_closing_tag(expression);
    smatch res;
    if(regex_search(to_be_scanned, res, find_closing_tag)){
      this->scanner_index += res.position()+ res.str().length();
      this->find_closing_tag_end();
    }else{
      string message = "no matching closing tag found";
      throw tag_error(message);
    }
  }

  void from_string(string& markup_string) 
  {
    // cout<<endl<<endl<<"######################## Scanning from string ########################"<<endl;
    this->source = markup_string;
    this->scanner_index = 0;
    // cout<< "### finding the starting tag"<<endl;
    this->find_starting_tag();

    // cout<<endl<<endl<<"### finding the parameters"<<endl;
    this->find_params();
    // cout<<endl<<endl<<"### finding the end of the starting tag"<<endl;
    if(this->find_start_tag_end()){
      // cout<<endl<<endl<<"### finding the content"<<endl;
      this->find_content();
      this->find_closing_tag();
    };
    // cout<<endl<<endl<<"### printing"<<endl;
    this->print_content();
    // cout<<endl<<"#################################################################################"<<endl<<endl<<endl;
    // this->find_content();
  }

  void wrap(string& markup_string, string& name){
    this->source = "<" + name + " >"+ markup_string + "</" + name +">";
    // cout<< "wrap: the target markup string after wrapping "<< this->source<<endl;
    this->from_string(this->source);
  }

  vector<string> split_query(string& query){
    string search = ".";
    vector<string> foundings;
    size_t found = query.find(search);
    size_t last_found = 0;
    while(found != string::npos){
      string tag_name = query.substr(last_found, found -last_found);
      last_found = found+1;
      foundings.push_back(tag_name);
      found = query.find(search, found+1);
    }
    string param_name = "";
    string last_tag_name = "";
    search = "~";
    found= query.find(search);
    if(found == string::npos){
      string message = "no param name found";
      throw tag_error(message);
    }else{
      param_name = query.substr(found+1);
      last_tag_name = query.substr(last_found, found - last_found);
      foundings.push_back(last_tag_name);
      foundings.push_back(param_name);
    }
    return foundings;
  }

  string join_query(vector<string>& list, int starting_index = 0){
    string temp = list[starting_index];
    for(int i=starting_index+1; i<(list.size() - 1); i++){
      temp += "."+ list[i];
    }
    temp +="~"+ list[list.size()-1];
    return temp;
  }

  string query_param_value(string& query){
    string search = ".";
    // cout<< "query_param_value: attempting to fetch the query "<< query << endl;
    vector<string> query_list = this->split_query(query);
    for(int i =0; i< query_list.size()-1; i++){
      // cout<< "query_param_value: a tag is " << query_list[i]<<endl;
    }
    // cout<< "query_param_value: param name is " << query_list[query_list.size()-1]<<endl;
    if(query_list.size() == 2){
      string target_tag_name = query_list[0];
      string target_param_name = query_list[1];
      if(this->name != target_tag_name){
        string message = "target tag not matching";
        throw tag_error(message);
      }else{
        for(int i=0; i<this->params.size();i++){
          if(this->params[i].name == target_param_name){
            return this->params[i].value;
          }
        }
        string message = "current tag has no such param";
        throw tag_error(message);
      }
    }else{
      string passed_query = this->join_query(query_list, 1);
      for(int i = 0; i< this->children.size(); i++){
        if(this->children[i].type == tag_enum_type){
          if(this->children[i].tag.name == query_list[1]){
            return this->children[i].tag.query_param_value(passed_query);
          }
        }
      }
      string message("no matching child found");
      throw tag_error(message);
    }
  }

  string query_param_value_as_wrapped(string& query){
    string wrapped_query =  this->name + "." + query;
    try
    {
      return this->query_param_value(wrapped_query);
    }
    catch(tag_error& e)
    {
      // cout<< e.what()<<endl;
      string message = "Not Found!";
      return message;
    }
  }

};



int main()
{
  string mark_up_string = "";

  int n,q;
  scanf("%d %d \n",&n,&q);
  
  string temp;
  for(int i =0; i<n; i++){
    getline(cin,temp);
    mark_up_string+=temp;
  }

  vector<string> queries(0);

  for(int i=0; i<q; i++){
    getline(cin,temp);
    queries.push_back(temp);
  }
  
  tag_object main_tag;
  string parent_name = "main_tag";
  // cout<<mark_up_string<<endl;
  try
  {  
    main_tag.wrap(mark_up_string, parent_name);
  }
  catch(tag_error& e)
  {
    std::cerr << e.what() << '\n';
    exit(2);
  }
  

  for(int i = 0; i<queries.size(); i++){
    cout<<main_tag.query_param_value_as_wrapped(queries[i])<<endl;
  }

  return 0;
}