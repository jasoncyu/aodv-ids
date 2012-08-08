// table.h
//
#ifndef LZZ_table_h
#define LZZ_table_h

#include "common.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <stdint.h>

class Table
{
  static void TableHeaders (std::vector <std::string> header_names, std::ostringstream& oss);
  static void TableValues (Sample s, std::ostringstream& oss);

public:
  static std::vector<int> widths;
  static void CreateTables (std::string file_name, std::vector <std::string> header_names, Sample s);
};

#endif
