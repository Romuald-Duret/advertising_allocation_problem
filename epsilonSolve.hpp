//
//  epsilonSolve.hpp
//  CPLEX_Test
//
//  Created by Romuald Duret on 25/02/2023.
//

#ifndef epsilonSolve_hpp
#define epsilonSolve_hpp

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

/*
 Racourcis
*/
using namespace std;
// orderered_json sinon il va trier par ordre alphabetique
using json = nlohmann::ordered_json;

void epsilonSolve(Data * mydata);

#endif /* epsilonSolve_hpp */
