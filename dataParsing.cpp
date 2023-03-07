//
//  dataParsing.cpp
//  CPLEX_Test
//
//  Created by Romuald Duret on 25/01/2023.
//

#include "dataParsing.hpp"
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

/*
 Entrée :
 2 fichiers au format JSON correspondant respectivement aux demandes des marques
 et aux offres proposées par les propriétaires télévisuels via leur chemin d’accès précisé.
 
 Sortie :
 Aucune. La fonction modifie directement les structures de données utiles pour la
 définition du modèle de résolution.
 
 Préconditions :
 Il faut que les 2 fichiers existent et se situent sur le chemin d’accès précisé.
 
 Postconditions :
 Aucune.
 
 Priorité :
 Cette fonction est primordiale et sera utilisée pour les 2 approches à implémenter.
*/

void parsingData(Data * mydata ,string breaks_path, string brands_path){

    
    // Récupération des données JSON des spots
    ifstream bks(breaks_path);
    json breaks = json::parse(bks);
    mydata->m = (int) breaks.size();
    
    // Récupération des données JSON des marques
    ifstream bds(brands_path);
    json brands = json::parse(bds);
    mydata->n = (int) brands.size();
    
    
    // Remplissage données de la structure -> ecrans
    for (const auto& item : breaks.items()){
        
        // fill T_i
        mydata->T_i.push_back(item.value()["remaining_time"]);
        
        // fill prime_i
        string TMP_prime = item.value()["prime"];
        mydata->prime_i.push_back(stoi(TMP_prime));
        
        // Initialisation des listes internes pour p_ik et premium_ik
        list<int> tmp_p_ik;
        list<bool> tmp_premium_ik;
        
        // Permet de gerer les spots à ajouter ou non
        int cpt_spot = 0;
        int nb_spot_to_remove = 0;
        for(const auto& spot : item.value()["slots"]){
        
            // Les 2 premiers et 2 derniers spots sont premium
            if (cpt_spot <= 1 || cpt_spot > item.value()["slots"].size() -3){
                if(spot["available"] == "1"){
                    tmp_premium_ik.push_back(1);
                    tmp_p_ik.push_back(spot["price"]);
                }else{
                    nb_spot_to_remove++;
                }
                
            }else{
                if(spot["available"] == "1"){
                    tmp_premium_ik.push_back(0);
                    tmp_p_ik.push_back(spot["price"]);
                }else{
                    nb_spot_to_remove++;
                }
            }
            
            cpt_spot++;
        }
        
        // Ajout des listes internes des infos de chaque spot pour l'écran courant
        mydata->p_ik.push_back(tmp_p_ik);
        mydata->premium_ik.push_back(tmp_premium_ik);
        
        // Nombre de spots disponibles pour l'écran courant -> s_ik
        mydata->s_i.push_back((int) item.value()["slots"].size() - nb_spot_to_remove);
        
        // Ajout du GRP_ij
        list<double> tmp_grp_ij;
        for (const auto& brand : brands.items()){
            json cpt_grp = item.value()["grp"];
            for (const auto& g : cpt_grp.items()){
                
                // Si les audiances concordent
                if( g.key().compare(brand.value()["audience"]) == 0){
                    tmp_grp_ij.push_back(g.value());
                }
            }
        }
        // Ajout de la valeur GRP cherchée pour chaque marque dans l'écran courant
        mydata->grp_ij.push_back(tmp_grp_ij);
    }
    
    // Remplissage données de la structure -> marques
    for (const auto& brand : brands.items()){
        
        // Matrice de comparaison du type de marques -> attention à ne pas compter la marque courante à l'avenir
        list<bool> tmp_compare;
        for (const auto& brand2 : brands.items()){
            
            // Si les marques ne doivent pas être dans le même écran
            if( ((string) brand.value()["type"]).compare(brand2.value()["type"]) == 0){
                tmp_compare.push_back(1);
            }else{
                tmp_compare.push_back(0);
                
            }
            
        }
        // Ajout de la matrice de compatibilité de chaque marque.
        mydata->compare_brands.push_back(tmp_compare);
        
        // GRP_j
        mydata->minGRP_j.push_back(brand.value()["cost_grp"]);
        
        // Brand time
        mydata->d_j.push_back(brand.value()["format"]);
        
        // BUDGET_j
        mydata->b_j.push_back(brand.value()["budget"]);
        
        // PRIME_j
        mydata->maxPRIME_j.push_back(brand.value()["ratio_prime"]);
        
        // Premium_j
        mydata->maxPREMIUM_j.push_back(brand.value()["ratio_premium"]);
    }
    
    
    /*
     Remplissage Modele CPLEX
    */
    
    // Constantes
    mydata->Model_m = mydata->m;
    mydata->Model_n = mydata->n;
    
    // Ecran
    mydata->Model_T_i = IloNumArray(mydata->env, mydata->m);
    mydata->Model_prime_i = IloNumArray(mydata->env, mydata->m);
    mydata->Model_s_i = IloNumArray(mydata->env, mydata->m);

    // Marques
    mydata->Model_d_j = IloNumArray(mydata->env, mydata->n);
    mydata->Model_b_j = IloNumArray(mydata->env, mydata->n);
    mydata->Model_minGRP_j = IloNumArray(mydata->env, mydata->n);
    mydata->Model_maxPREMIUM_j = IloNumArray(mydata->env, mydata->n);
    mydata->Model_maxPRIME_j = IloNumArray(mydata->env, mydata->n);

    // Modèle 2D
    mydata->Model_grp_ij =  IloArray<IloNumArray>(mydata->env, mydata->m);
    mydata->Model_p_ik =  IloArray<IloNumArray>(mydata->env, mydata->m);
    mydata->Model_premium_ik =  IloArray<IloNumArray>(mydata->env, mydata->m);
    mydata->Model_compare_brands = IloArray<IloNumArray>(mydata->env, mydata->n);
    
    
    // Données des écrans
    int cpt = 0;
    auto prime = mydata->prime_i.begin();
    auto grp = mydata->grp_ij.begin();
    auto s_i = mydata->s_i.begin();
    auto p_ik = mydata->p_ik.begin();
    auto premium_ik = mydata->premium_ik.begin();
    for(auto j=mydata->T_i.begin();j!=mydata->T_i.end();j++)
    {
        
        // Model_T_i
        mydata->Model_T_i[cpt] = *j;
        
        // Model_prime_i
        mydata->Model_prime_i[cpt] = *prime;
        prime++;
        
        // Model_s_i
        mydata->Model_s_i[cpt] = *s_i;
        
        
        
        
        // Model_grp_ij
        mydata->Model_grp_ij[cpt] = IloNumArray(mydata->env, mydata->n);
        list<double> & tmp_grp = *grp;
        int marque_cpt = 0;
        for(auto marque = tmp_grp.begin(); marque!= tmp_grp.end(); marque++){
            mydata->Model_grp_ij[cpt][marque_cpt] = *marque;
            marque_cpt++;
        }
        grp++;
        
        
        // Model_p_ik et Model_premium_ik
        mydata->Model_p_ik[cpt] = IloNumArray(mydata->env, *s_i);
        mydata->Model_premium_ik[cpt] = IloNumArray(mydata->env, *s_i);
        
        list<int> & tmp_p_ik = *p_ik;
        list<bool> & tmp_premium_ik = *premium_ik;
        
        auto prem_spot = tmp_premium_ik.begin();
        int spot_cpt = 0;
        
        for(auto spot = tmp_p_ik.begin(); spot!= tmp_p_ik.end(); spot++){
            
            mydata->Model_p_ik[cpt][spot_cpt] = *spot;
            mydata->Model_premium_ik[cpt][spot_cpt] = *prem_spot;
            
            prem_spot++;
            spot_cpt++;
        }
        
        premium_ik++;
        p_ik++;
        s_i++;
        
        cpt++;
    }
    

    
    // Données des marques
    cpt = 0;
    auto duree = mydata->d_j.begin();
    auto minGRP = mydata->minGRP_j.begin();
    auto maxprime = mydata->maxPRIME_j.begin();
    auto maxpremium = mydata->maxPREMIUM_j.begin();
    auto brand1 = mydata->compare_brands.begin();
    for(auto budget =mydata->b_j.begin();budget!=mydata->b_j.end();budget++){
        
        // b_j
        mydata->Model_b_j[cpt] = *budget;
        
        // d_j
        mydata->Model_d_j[cpt] = *duree;
        duree++;
        
        // min_GRP_j
        mydata->Model_minGRP_j[cpt] = *minGRP;
        minGRP++;
        
        // maxPRIME_j
        mydata->Model_maxPRIME_j[cpt] = *maxprime;
        maxprime++;
        
        // maxPREMIUM_j
        mydata->Model_maxPREMIUM_j[cpt] = *maxpremium;
        maxpremium++;
        
        // Compare brands
        int cpt2 = 0;
        mydata->Model_compare_brands[cpt] = IloNumArray(mydata->env, mydata->n);
        for(auto brands2 = (*brand1).begin(); brands2 != (*brand1).end(); brands2++){
            mydata->Model_compare_brands[cpt][cpt2] = *brands2;
            cpt2++;
        }
        brand1++;
        
        cpt++;
    }

    
}
