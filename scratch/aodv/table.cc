#include "common.h"
#include "table.h"

using namespace std;

std::vector<int> Table::widths;

void
Table::TableHeaders(vector<string> header_names, ostringstream& oss) {
  oss << "#";
  
  //header name and width pairs

  vector<string>::iterator itr = header_names.begin();  
  while (itr != header_names.end())
  {
    widths.push_back(itr->size() + 3);
    oss << setw(itr->size()) << *itr;
    itr++;
    if (itr != header_names.end())
      oss << " | ";
  }

  oss << std::endl;
}

void
Table::TableValues(TrafficList tl, ostringstream& oss) {
  TrafficList::iterator itr = tl.begin();
  while (itr != tl.end()) {
    Traffic traffic = *itr;
    for (uint32_t i = 0; i < traffic.size(); ++i)
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
Table::CreateTables(string file_name, vector<string> header_names, TrainingData td ) {
  std::ofstream report;
  report.open(file_name.c_str());
  if (!report.is_open ()) {
    std::cout << "ERROR: could not open file" << std::endl;
  }

  ostringstream oss;

  TrainingData::iterator tditr = td.begin();
  int num = tditr->first;
  TrafficList tl = tditr->second;

  oss << "Monitor Node #" << num << std::endl; 
  oss << "-------------------------------------------" << std::endl;
  TableHeaders(header_names, oss);
  TableValues(tl, oss);

  report << oss.str() << std::endl; 
  report.close();
}

