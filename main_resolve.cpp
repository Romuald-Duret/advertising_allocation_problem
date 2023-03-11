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
void testSolution(string filename, Data * mydata){
    
    // Récupération des données JSON des solutions
    ifstream sol_file(filename);
    json solutions = json::parse(sol_file);
    
    // Nombre de solutions trouvées
    int nb_sol = solutions["Nombre solutions total"].get<int>();
    
    // Structure d'une solution
    struct Solution {
        double grp;
        int revenus_TV;
        list<list<list<bool>>> allocation;
    };
    
    // Tableau de solution
    struct Solution tab_sol[nb_sol];

    // Parsing des solutions
    int tmp = 0;
    for (const auto& item : solutions.items()){
        
        if(tmp != nb_sol){
            
            struct Solution tmp_sol;
            
            tmp_sol.grp = item.value()["GRP"];
            tmp_sol.revenus_TV = item.value()["revenus globaux"];
            
            list<list<list<bool>>> allocation;
            
            for(const auto& ecran : item.value()["Allocation"].items()){
                
                list<list<bool>> tmp_ecran;
                for(const auto& spot : ecran.value().items()){
                    
                    list<bool> tmp_spot;
                    for(const auto& brand : spot.value().items()){
                        
                        tmp_spot.push_back(brand.value());
                    }
                    
                    tmp_ecran.push_back(tmp_spot);
                }
                
                allocation.push_back(tmp_ecran);
            }
            
            tmp_sol.allocation = allocation;
            
            tab_sol[tmp] = tmp_sol;
        }
    
        tmp ++;
        
    }
    
    
    // Verification contraintes
    for(int i = 0; i < nb_sol; i++){
        
        
        
        // Un spot ne peut être alloué qu'une seule fois
        int screen = 0;
        for(auto break_tmp = tab_sol[i].allocation.begin(); break_tmp != tab_sol[i].allocation.end(); break_tmp++){
            
            
            int spot = 0;
            list<list<bool>> & spots = *break_tmp;
            for(auto spot_tmp = spots.begin(); spot_tmp != spots.end(); spot_tmp++){
                
                
                int cpt = 0;
                
                list<bool> & brands = *spot_tmp;
                for(auto brand_tmp = brands.begin(); brand_tmp != brands.end(); brand_tmp++){
                    
                    //cout << *brand_tmp << endl;
                    
                    if(*brand_tmp){
                        cpt+=1;
                    }
                }
                
                if(cpt > 1){
                    cout << "Error at solution : " <<  i << " | On screen : " << screen << " | At spot : " << spot << " | On constraint 1" << endl;
                    throw -1;
                }
                
                spot++;
            }
            screen++;
        }
        
        
        // Une marque ne peut apparaitre qu'une fois sur un écran
        screen = 0;
        for(auto break_tmp = tab_sol[i].allocation.begin(); break_tmp != tab_sol[i].allocation.end(); break_tmp++){
            
            int value[mydata->n];
            for(int i=0; i< mydata->n; i++){
                value[i] = 0;
            }
            
            
            int spot = 0;
            list<list<bool>> & spots = *break_tmp;
            for(auto spot_tmp = spots.begin(); spot_tmp != spots.end(); spot_tmp++){
                
                int brand = 0;
                list<bool> & brands = *spot_tmp;
                for(auto brand_tmp = brands.begin(); brand_tmp != brands.end(); brand_tmp++){
                    
                    if(*brand_tmp){
                        value[brand] += 1;
                    }
                    
                    brand++;
                }
                spot++;
            }
            
            
            for(int i=0; i< mydata->n; i++){
                if(value[i] > 1){
                    cout << "Error at solution : " <<  i << " | On screen : " << screen << " | At spot : " << spot << " | On constraint 2" << " | Value : " << value[i] << endl;
                    throw -1;
                }
            }
            screen++;
        }
        
        
        
        
       
     
        
        
        
        
        
        
    }
    
    
    
    cout << "OK -> Toutes les solutions sont validées." << endl;
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
    //epsilonSolve(&mydata);
    
    // Solution test
    testSolution("result_file_epsilon.json", &mydata);
     
    return 0;
}
