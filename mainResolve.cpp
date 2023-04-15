//
//  main_resolve.cpp
//  CPLEX_Test
//
//  Created by Romuald Duret on 18/01/2023.
//

/*!
 * @file mainResolve.cpp
 *
 * @section intro Introduction
 *
 * Ce fichier est le fichier principal. Il comporte la fonction main(). Cette méthode est pour l'instant à remplir en fonction des besoins. L'idéal serait d'y créer un menu afin de faciliter et automatiser l'exécution.
 *
 * @section libraries Libraries
 *
 * This file depends on 3 CPLEX libraries specified on the git.
 *
 * @section author Author
 *
 * Written by Romuald DURET.
 *
 */

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
#include "dataParsing.hpp"
#include "epsilonSolve.hpp"
#include "testSolution.hpp"
#include "sumponSolve.hpp"

ILOSTLBEGIN

/*
 Raccourcis
*/
using namespace std;


/*
 Entrée :
 2 fichiers au format JSON correspondant respectivement aux résultats obtenus par
 l’approche ε-contrainte et somme pondérée pour un même jeu de données via leur chemin
 d’accès précisé.
 
 Sortie :
 Il y a une sortie console indiquant les résultats obtenus pour chaque élément calculé
 pour chaque approche ainsi qu’une estimation de quelle approche semble être la meilleure.
 Préconditions : Les 2 fichiers précisés doivent bien correspondre aux 2 approches différentes
 pour un même jeu de données.
 
 Postconditions :
 Aucune.
 
 Priorité :
 Cette fonction est importante car permet d’analyser les résultats obtenus par les
 2 approches. Par ailleurs cette fonction peut être un outil permettant de détecter s’il existe
 une anomalie dans une des 2 approches développées grâce à l’analyse des résultats obtenus
*/
void compareSolves(){
    
}

/*
 Fonction main du programme. Elle est pour l'instant à remplir avec les différentes fonctions disponibles.
*/
int main(int argc, char **argv)
{
    // Données
    Data mydata;
    
    // Parsing des données
    parsingData(&mydata,
            "/Users/romu/Desktop/Projets/Stage2022/CPLEX_Test/CPLEX_Test/test_files/break.json",
                "/Users/romu/Desktop/Projets/Stage2022/CPLEX_Test/CPLEX_Test/test_files/brands.json");
    
    // Epsilon solve
    epsilonSolve(&mydata);
    
    // WeightSum solve
    sumponSolve(&mydata);
    
    // Solution test
    testSolution("result_file_epsilon.json", &mydata);
    testSolution("result_file_weightsum.json", &mydata);
    
    return 0;
}
