// table.h
//

#include <vector>

#ifndef LZZ_table_h
#define LZZ_table_h
class Table
{
  vector<int> widths;
  ostringstream TableHeaders (vector <string> header_names);
  void TableValues (TrafficList tl);
  void CreateTables (string file_name, vector <string> header_names, Cluster::TrainingData td);
};
#endif
