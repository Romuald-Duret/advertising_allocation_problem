//
//  dataParsing.hpp
//  CPLEX_Test
//
//  Created by Romuald Duret on 25/01/2023.
//

#ifndef dataParsing_hpp
#define dataParsing_hpp

#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include<string.h>
#include "json.hpp"
#include <list>
#include <tuple>
#include <iterator>
#include <ilcplex/ilocplex.h>
#include "Data.hpp"
ILOSTLBEGIN

using namespace std;
void parsingData(Data *, string, string);

#endif /* dataParsing_hpp */


