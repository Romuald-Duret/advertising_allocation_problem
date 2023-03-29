//
//  Data.hpp
//  CPLEX_Test
//
//  Created by Romuald Duret on 25/01/2023.
//

/*!
 * @file testSolution.cpp
 *
 * @section intro Introduction
 *
 * Ce fichier compote la classe Data qui stocke le modèle pour l'optimisation à partir de 2 fichiers JSON grace à la fonction parsingData() du fichier dataParsing.cpp
 *
 * @section author Author
 *
 * Written by Romuald DURET.
 *
 */


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
        int m; ///< Nombre d'écrans
        int n; ///< Nombre de marques

            // Ecrans
        list<int> T_i; ///< Durée de l'écran i en secondes
        list<list<double>> grp_ij; ///< Gain de GRP pour j dans l'écran i
        list<bool> prime_i; ///< Booleen indiquant si l'écran est prime ou non
        list<int> s_i; ///< Nombre de spots à l'intérieur de l'écran i
        list<list<int>> p_ik; ///< Prix par seconde d'allocation pour chaque spot k de l'écran i
        list<list<bool>> premium_ik; ///< Booleen indiquant si le spot k de l'ecran i est de type premium

    
    
            // Marques
        list<int> b_j; ///< Budget de la marque j
        list<float> minGRP_j; ///< Valeur minimale de GRP voulue pour la marque j
        list<int> d_j; ///< Durée en seconde de la publicité de la marque j
        list<float> maxPRIME_j; ///< Pourcentage maximum du budget à allouer pour la marque j sur des écrans PRIME
        list<float> maxPREMIUM_j; ///< Pourcentage maximum du budget à allouer pour la marque j sur des spots PREMIUM
        list<list<bool>> compare_brands; ///< 1 si 2 marques ne peuvent pas être dans le même écran 0 sinon
            
        /*
         Structure CPLEX
        */

        // Modèle CPLEX
        IloEnv env; ///< Modèle CPLEX. Les attributs suivants sont les mêmes que les précédents mais pour le modèle CPLEX.

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
