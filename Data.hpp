//
//  Data.hpp
//  CPLEX_Test
//
//  Created by Romuald Duret on 25/01/2023.
//

#ifndef Data_hpp
#define Data_hpp

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
ILOSTLBEGIN

class Data{
    public :
    
        /*
         Structure
        */
            // Tailles
        int m; // m = Nombre d'écrans
        int n; // N = Nombre de marques

            // Ecrans
        list<int> T_i; // T_i = durée de l'écran i en secondes
        list<list<float>> grp_ij; // grp_ij = Gain de GRP pour j dans l'écran i
        list<bool> prime_i;
        list<int> s_i;
        list<list<int>> p_ik;
        list<list<bool>> premium_ik;

    
    
            // Marques
        list<int> b_j;
        list<float> minGRP_j;
        list<int> d_j;
        list<float> maxPRIME_j;
        list<float> maxPREMIUM_j;
        list<list<bool>> compare_brands;
            
        /*
         Structure CPLEX
        */

        // Modèle CPLEX
        IloEnv env;

            // Taille
        IloNum Model_m;
        IloNum Model_n;

            // Modele ecrans
        IloNumArray Model_T_i;
        IloNumArray Model_prime_i;
        IloNumArray Model_s_i;

            // Modele marques
        IloNumArray Model_d_j;
        IloNumArray Model_b_j;
        IloNumArray Model_minGRP_j;
        IloNumArray Model_maxPREMIUM_j;
        IloNumArray Model_maxPRIME_j;

            // Modèle 2D
        IloArray<IloNumArray> Model_grp_ij;
        IloArray<IloNumArray> Model_p_ik;
        IloArray<IloNumArray> Model_premium_ik;
        IloArray<IloNumArray> Model_compare_brands;
};

#endif /* Data_hpp */
