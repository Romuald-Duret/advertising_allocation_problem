//
//  sumponSolve.hpp
//  CPLEX_Test
//
//  Created by Romuald Duret on 31/03/2023.
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

#ifndef sumponSolve_hpp
#define sumponSolve_hpp


/*
 Racourcis
*/
using namespace std;
// orderered_json sinon il va trier par ordre alphabetique
using json = nlohmann::ordered_json;

void sumponSolve(Data * mydata);

#endif /* sumponSolve_hpp */
