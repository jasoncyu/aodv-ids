using namespace std;

class Table
{
ostringstream
TableHeaders(vector<string> header_names) {
  ostringstream oss;
  oss << "#";
  
  //header name and width pairs

  vector<string>::iterator itr = header_names.begin();  
  while (itr != header_names.end())
  {
    widths.push_back(itr->size());
    oss << setw(itr->size()) << *itr;
    itr++;
    if (itr != header_names.end())
      oss << " | ";
  }

  oss << std::endl;

  return oss;
}

void
TableValues(TrafficList tl) {
  ostringstream oss;

  TrafficList::iterator itr = tl.begin();
  while (itr != tl.end() {
    Traffic traffic = *itr;
    for (int i = 0; i < traffic.size(); ++i)
    {
      oss << setw(widths[i]) << traffic[i];
    }
    oss << endl;
    itr++;
  }
}
//header_names: a vector of x headers
//variable arguments are each a vector of doubles
// and there should be exactly x of them, one for 
//each row of the table
void
CreateTables(string file_name, vector<string> header_names, Cluster::TrainingData td ) {
  ostringstream oss;

  int num = itr->first;
  TrafficList tl = itr->second;

  oss << "Monitor Node #" << num << std::endl; 
  oss << "-------------------------------------------" << std::endl;
  oss << TableHeaders(header_names);
  oss << TableValues(tl);
}

};

