//
//  testSolution.cpp
//  CPLEX_Test
//
//  Created by Romuald Duret on 28/03/2023.
//

/*!
 * @file testSolution.cpp
 *
 * @section intro Introduction
 *
 * Ce fichier compote la fonction testSolution(string, Data) et permet de tester les solutions obtenues et stockées dans un fichier à partir d'une approche epsilon-contrainte ou somme pondérée.
 *
 * Entrée :
 * Un fichier au format JSON correspondant aux solutions obtenues pour une instance
 * de résolution du problème peu importe l’approche utilisée.
 * La fonction utilise le modèle qui est défini et rempli par la fonction
 * parsingData().
 
 * Sortie :
 * Une sortie console indiquant par un chiffre si toutes les solutions sont bonnes ou
 * non (respectivement 0 ou 1). Si 1, alors la fonction indique l’indice de la solution qui ne
 * passe pas les tests ainsi qu’un numéro indiquant quelle contrainte la solution n’arrive-t-elle
 * pas à satisfaire.
 
 * Préconditions :
 * Aucune.
 
 * Postconditions :
 * Si le programme retourne 0 c’est que toutes les solutions sont correctes,
 * s’il retourne 1 alorse la solution indiquée par l’indice ne respecte pas la contrainte indiquée
 * dans le code d’erreur affiché.
 
 * Priorité :
 * Cette fonction est importante car elle permet de valider si les résultats obtenus
 * sont correctes.
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


#include "testSolution.hpp"


/*
 Entrée :
 Un fichier au format JSON correspondant aux solutions obtenues pour une instance
 de résolution du problème peu importe l’approche utilisée.
 La fonction utilise le modèle qui est défini et rempli par la fonction parsingData().
 
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
            
            // Récupération des valeurs des objtecifs de la solution temporaire
            tmp_sol.grp = item.value()["GRP"];
            tmp_sol.revenus_TV = item.value()["revenus globaux"];
            
            list<list<list<bool>>> allocation;
            
            // Parcours de la solution actuelle
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
    
    /*
    Verification contraintes
    */
    for(int i = 0; i < nb_sol; i++){
        
        
        /*
        Un spot ne peut être alloué qu'une seule fois
        */
        
        // Parcours de la solution
        int screen = 0;
        for(auto break_tmp = tab_sol[i].allocation.begin(); break_tmp != tab_sol[i].allocation.end(); break_tmp++){
            
            int spot = 0;
            list<list<bool>> & spots = *break_tmp;
            for(auto spot_tmp = spots.begin(); spot_tmp != spots.end(); spot_tmp++){
                
                
                int cpt = 0;
                list<bool> & brands = *spot_tmp;
                for(auto brand_tmp = brands.begin(); brand_tmp != brands.end(); brand_tmp++){
                    
                    // Si le spot est alloué -> +1 au compteur
                    if(*brand_tmp){
                        cpt+=1;
                    }
                }
                
                // Test vérifiant que chaque spot est alloué qu'une seule fois
                if(cpt > 1){
                    cout << "Error at solution : " <<  i << " | On screen : " << screen << " | At spot : " << spot << " | On constraint 1" << endl;
                    throw -1;
                }
                
                spot++;
            }
            screen++;
        }
        
        
        /*
        Une marque ne peut apparaitre qu'une fois sur un écran
        */
        
        // Parcours de la solution
        screen = 0;
        for(auto break_tmp = tab_sol[i].allocation.begin(); break_tmp != tab_sol[i].allocation.end(); break_tmp++){
            
            
            int unique_brand[mydata->n];
            for(int i=0; i< mydata->n; i++){
                unique_brand[i] = 0;
            }
            
            
            int spot = 0;
            list<list<bool>> & spots = *break_tmp;
            for(auto spot_tmp = spots.begin(); spot_tmp != spots.end(); spot_tmp++){
                
                int brand = 0;
                list<bool> & brands = *spot_tmp;
                for(auto brand_tmp = brands.begin(); brand_tmp != brands.end(); brand_tmp++){
                    
                    // Si la marque est allouée -> +1 au compteur de la marque pour l'écran
                    if(*brand_tmp){
                        unique_brand[brand] += 1;
                    }
                    
                    brand++;
                }
                spot++;
            }
            
            // Test vérifiant que la marque n'apparait qu'une fois maximum sur chaque écran
            for(int i=0; i< mydata->n; i++){
                if(unique_brand[i] > 1){
                    cout << "Error at solution : " <<  i << " | On screen : " << screen << " | On constraint 2" << " | Value : " << unique_brand[i] << endl;
                    throw -1;
                }
            }
            screen++;
        }
        
        
        
        
        /*
        Ne pas dépasser le budget de chaque marque
        */
        
        // Creation tableau contenant le budget de chaque marque dans chaque solution
        int brand_budget[mydata->n];
        for(int i=0; i< mydata->n; i++){
            brand_budget[i] = 0;
        }
        
        // Parcours de la solution
        screen = 0;
        for(auto break_tmp = tab_sol[i].allocation.begin(); break_tmp != tab_sol[i].allocation.end(); break_tmp++){
            
            
            int spot = 0;
            list<list<bool>> & spots = *break_tmp;
            for(auto spot_tmp = spots.begin(); spot_tmp != spots.end(); spot_tmp++){
                
                int brand = 0;
                list<bool> & brands = *spot_tmp;
                for(auto brand_tmp = brands.begin(); brand_tmp != brands.end(); brand_tmp++){
                    
                    // Si la marque est allouée sur ce spot alors on ajoute le budget de l'allocation
                    if(*brand_tmp){
                        brand_budget[brand] += mydata->Model_p_ik[screen][spot] * mydata->Model_d_j[brand];
                    }
                    
                    brand++;
                }
                spot++;
            }
            
            screen++;
        }
        
        // Test verifiant ou non que le budget obtenu est valide
        for(int i=0; i< mydata->n; i++){
            if(brand_budget[i] > mydata->Model_b_j[i]){
                cout << "Error for brand : " <<  i << " | On constraint 3" << " | Value : " << brand_budget[i] << " | Objective : " << mydata->Model_b_j[i] << endl;
                throw -1;
            }
        }
        
        
        
        
        
        /*
        Ne pas dépasser la durée de l'écran publicitaire
        */
        
        // Parcours de solution
        screen = 0;
        for(auto break_tmp = tab_sol[i].allocation.begin(); break_tmp != tab_sol[i].allocation.end(); break_tmp++){
            
            // Compteur du temps alloué pour cet écran
            int cpt_time = 0;
            
            int spot = 0;
            list<list<bool>> & spots = *break_tmp;
            for(auto spot_tmp = spots.begin(); spot_tmp != spots.end(); spot_tmp++){
                
                int brand = 0;
                list<bool> & brands = *spot_tmp;
                for(auto brand_tmp = brands.begin(); brand_tmp != brands.end(); brand_tmp++){
                    
                    // Si la marque est allouée sur cet écran alors on ajoute la durée de pub dans le compteur
                    if(*brand_tmp){
                        cpt_time += mydata->Model_d_j[brand];
                    }
                    
                    brand++;
                }
                
                spot++;
            }
            
            // Test vérifiant que le temps alloué est inférieur à la limite de l'écran
            if(cpt_time > mydata->Model_T_i[screen]){
                cout << "Error for brand : " <<  i << " | On constraint 4" << " | Value : " << cpt_time << " | Objective : " <<  mydata->Model_T_i[screen] << endl;
                throw -1;
            }
            
            screen++;
        }
        
        
        /*
        Pas de marques compétititives au sein d'un même écran
        */
        // Parcours de solution
        screen = 0;
        for(auto break_tmp = tab_sol[i].allocation.begin(); break_tmp != tab_sol[i].allocation.end(); break_tmp++){
            
            // Liste permettant de sauvegarder la présence d'une marque dans un écran (0,1)
            int brand_pres[mydata->n];
            for(int i=0; i< mydata->n; i++){
                brand_pres[i] = 0;
            }
            
            int spot = 0;
            list<list<bool>> & spots = *break_tmp;
            for(auto spot_tmp = spots.begin(); spot_tmp != spots.end(); spot_tmp++){
                
                int brand = 0;
                list<bool> & brands = *spot_tmp;
                for(auto brand_tmp = brands.begin(); brand_tmp != brands.end(); brand_tmp++){
                    
                    // Si la marque est allouée
                    if(*brand_tmp){
                        brand_pres[brand] = 1;
                    }
                    
                    brand++;
                }
                
                spot++;
            }
            
            // On parcours le tableau de présence de solution
            for(int i=0; i< mydata->n; i++){
                
                // Si une marque est allouée
                if(brand_pres[i] == 1){
                    
                    // On parcours une 2eme fois
                    for(int i2 = 0; i2 < mydata->n; i2++){
                        
                        // Afin de regarder si une autre marque allouée est compétitive avec la première
                        if(i!=i2 && brand_pres[i2] && mydata->Model_compare_brands[i][i2]){
                            
                            cout << "Error for brand : " <<  i << " and brand : "<< i2 << " | At screen " << screen << " | On constraint 5"  << endl;
                            throw -1;
                        }
                        
                    }
                }
            }
            
            screen++;
        }
        
        
        
        /*
        Atteindre un niveau de GRP minimal pour chaque marque
        */
        
        // Liste permettant de sauvegarder le grp de chaque marque
        float brand_grp[mydata->n];
        for(int i=0; i< mydata->n; i++){
            brand_grp[i] = 0.;
        }
        
        // Parcours de solution
        screen = 0;
        for(auto break_tmp = tab_sol[i].allocation.begin(); break_tmp != tab_sol[i].allocation.end(); break_tmp++){
            
            
            int spot = 0;
            list<list<bool>> & spots = *break_tmp;
            for(auto spot_tmp = spots.begin(); spot_tmp != spots.end(); spot_tmp++){
                
                int brand = 0;
                list<bool> & brands = *spot_tmp;
                for(auto brand_tmp = brands.begin(); brand_tmp != brands.end(); brand_tmp++){
                    
                    // Si la marque est allouée sur cet écran alors on ajoute le grp obtenu
                    if(*brand_tmp){
                        brand_grp[brand] += mydata->Model_grp_ij[screen][brand];
                    }
                    
                    brand++;
                }
                
                spot++;
            }
            
            screen++;
        }
        
        // On parcours le tableau grp de chaque marque
        for(int i=0; i< mydata->n; i++){
            if(brand_grp[i] < mydata->Model_minGRP_j[i]){
                cout << "Error for brand : " <<  i << " | On constraint 6" << " | GRP_min required : " << mydata->Model_minGRP_j[i] << " | Acutal value : " << brand_grp[i] << endl;
                throw -1;
            }
        }
        
        /*
        Ne pas dépasser le pourcentage d'allocation maximal pour premimum
        */
        
        int brand_premium[mydata->n];
        for(int i=0; i< mydata->n; i++){
            brand_premium[i] = 0;
        }
        
        // Parcours de solution
        screen = 0;
        for(auto break_tmp = tab_sol[i].allocation.begin(); break_tmp != tab_sol[i].allocation.end(); break_tmp++){
            
            
            int spot = 0;
            list<list<bool>> & spots = *break_tmp;
            for(auto spot_tmp = spots.begin(); spot_tmp != spots.end(); spot_tmp++){
                
                int brand = 0;
                list<bool> & brands = *spot_tmp;
                for(auto brand_tmp = brands.begin(); brand_tmp != brands.end(); brand_tmp++){
                    
                    // Si la marque est allouée sur cet écran alors on ajoute le grp obtenu
                    if(*brand_tmp && mydata->Model_premium_ik[screen][spot]){
                        brand_premium[brand] += mydata->Model_p_ik[screen][spot] * mydata->Model_d_j[brand];
                    }
                    
                    brand++;
                }
                
                spot++;
            }
            
            screen++;
        }
        
        for(int i=0; i< mydata->n; i++){
            if(brand_premium[i] > ((mydata->Model_maxPREMIUM_j[i] / 100) * mydata->Model_b_j[i])){
                cout << "Error for brand : " <<  i << " | On constraint 7" << " | PREMIUM_max required : " << mydata->Model_maxPREMIUM_j[i] << " represant :  " << (mydata->Model_maxPREMIUM_j[i] / 100) * mydata->Model_b_j[i] << " | Acutal value : " << brand_premium[i] << endl;
                throw -1;
            }
        }
        
        
        /*
        Ne pas dépasser le pourcentage d'allocation maximal pour prime
        */
        
        int brand_prime[mydata->n];
        for(int i=0; i< mydata->n; i++){
            brand_prime[i] = 0;
        }
        
        
        // Parcours de solution
        screen = 0;
        for(auto break_tmp = tab_sol[i].allocation.begin(); break_tmp != tab_sol[i].allocation.end(); break_tmp++){
            
            
            int spot = 0;
            list<list<bool>> & spots = *break_tmp;
            for(auto spot_tmp = spots.begin(); spot_tmp != spots.end(); spot_tmp++){
                
                int brand = 0;
                list<bool> & brands = *spot_tmp;
                for(auto brand_tmp = brands.begin(); brand_tmp != brands.end(); brand_tmp++){
                    
                    // Si la marque est allouée sur cet écran alors on ajoute le grp obtenu
                    if(*brand_tmp && mydata->Model_prime_i[screen]){
                        brand_prime[brand] += mydata->Model_p_ik[screen][spot] * mydata->Model_d_j[brand];
                    }
                    
                    brand++;
                }
                
                spot++;
            }
            
            screen++;
        }
        
        for(int i=0; i< mydata->n; i++){
            if(brand_prime[i] > ((mydata->Model_maxPRIME_j[i] / 100) * mydata->Model_b_j[i])){
                cout << "Error for brand : " <<  i << " | On constraint 8" << " | PREMIUM_max required : " << mydata->Model_maxPRIME_j[i] << " represant :  " << (mydata->Model_maxPRIME_j[i] / 100) * mydata->Model_b_j[i] << " | Acutal value : " << brand_prime[i] << endl;
                throw -1;
            }
        }
        
        
    }
    
    
    // Si le code arrive ici c'est que toutes les solutions sont valides et respectent le modèle.
    cout << "OK -> Toutes les solutions sont validées." << endl;
}
