//
//  testSolution.hpp
//  CPLEX_Test
//
//  Created by Romuald Duret on 28/03/2023.
//

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

#ifndef testSolution_hpp
#define testSolution_hpp

ILOSTLBEGIN

/*
 Racourcis
*/
using namespace std;
// orderered_json sinon il va trier par ordre alphabetique
using json = nlohmann::ordered_json;

void testSolution(string filename, Data * mydata);


#endif /* testSolution_hpp */

