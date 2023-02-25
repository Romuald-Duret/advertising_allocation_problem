//
//  main_resolve.cpp
//  CPLEX_Test
//
//  Created by Romuald Duret on 18/01/2023.
//

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

ILOSTLBEGIN

/*
 Racourcis
*/
using namespace std;

/*
 Entrée :
 Aucune. La fonction utilise le modèle qui est défini et rempli par la fonction
 parsingData().
 
 Sortie :
 Il y a 2 types de sorties : la sortie console et la sortie fichier. La sortie console
 correspond au retour console et indiquera certaines informations sur l’exécution de la
 fonction telle que le temps d’exécution par exemple. La sortie fichier correspond à un fichier
 JSON indiquant toutes les solutions obtenues, les valeurs des objectifs pour chaque solution
 ainsi que l’ordonnancement pour chacune d’entre elle.
 
 Préconditions :
 Le modèle doit être rempli avec la fonction parsingData().
 
 Postconditions :
 Les solutions doivent respecter toutes les contraintes spécifiées par le
 modèle.
 
 Priorité :
 Cette fonction est importante et correspond à une des 2 approches de résolution.
*/
void sumponSolve(){
    
}


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
 Entrée :
 Un fichier au format JSON correspondant aux solutions obtenues pour une instance
 de résolution du problème peu importe l’approche utilisée.
 
 Sortie :
 Une sortie console indiquant par un chiffre si toutes les solutions sont bonnes ou
 non (respectivement 0 ou 1). Si 1, alors la fonction indique l’indice de la solution qui ne
 passe pas les tests ainsi qu’un numéro indiquant quelle contrainte la solution n’arrive-t-elle
 pas à satisfaire.
 
 Préconditions :
 Aucune.
 
 Postconditions :
 Si le programme retourne 0 c’est que toutes les solutions sont correctes,
 s’il retourne 1 alorse la solution indiquée par l’indice ne respecte pas la contrainte indiquée
 dans le code d’erreur affiché.
 
 Priorité :
 Cette fonction est importante car elle permet de valider si les résultats obtenus
 sont correctes.
*/
void testSolution(){
    
}



/*
 
 */
int main(int argc, char **argv)
{
    // Données
    Data mydata;
    
    // Parsing des données
    parsingData(&mydata,
            "/Users/romu/Desktop/Projets/Stage2022/CPLEX_Test/CPLEX_Test/break.json",
                "/Users/romu/Desktop/Projets/Stage2022/CPLEX_Test/CPLEX_Test/brands.json");
    
    // Epsilon solve
    epsilonSolve(&mydata);
     
    return 0;
}
