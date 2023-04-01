//
//  epsilonSolve.cpp
//  CPLEX_Test
//
//  Created by Romuald Duret on 25/02/2023.
//

/*!
 * @file epsilonSolve.cpp
 *
 * @section intro Introduction
 *
 * Ce fichier compote la fonction epsilonSolve(Data) et permet de réaliser une optimisation du problème initial à partir de 2 fichiers avec l'approche epsilon-contrainte.
 *
 * Entrée :
 * La fonction utilise le modèle qui est défini et rempli par la fonction
 * parsingData().
 
 * Sortie :
 * Il y a 2 types de sorties : la sortie console et la sortie fichier. La sortie console
 * correspond au retour console et indiquera certaines informations sur l’exécution de la
 * fonction telle que le temps d’exécution par exemple. La sortie fichier correspond à un fichier
 * JSON indiquant toutes les solutions obtenues, les valeurs des objectifs pour chaque solution
 * ainsi que l’ordonnancement pour chacune d’entre elle.
 *
 * Préconditions :
 * Le modèle doit être rempli avec la fonction parsingData().
 *
 * Postconditions :
 * Les solutions doivent respecter toutes les contraintes spécifiées par le
 * modèle.
 *
 * Priorité :
 * Cette fonction est primordiale et correspond à une des 2 approches de résolution
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

#include "epsilonSolve.hpp"
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
 Cette fonction est primordiale et correspond à une des 2 approches de résolution
*/
void epsilonSolve(Data * mydata){
    
    // VARIABLE : x_ikj
    IloArray<IloArray<IloNumVarArray>> Model_Var_x_ikj(mydata->env, mydata->m);
    for(int i = 0; i< mydata->m; i++){
        //cout << mydata->Model_s_i[i] << endl;
        Model_Var_x_ikj[i] = IloArray<IloNumVarArray>(mydata->env, mydata->Model_s_i[i]);
        for(int spot = 0; spot < mydata->Model_s_i[i]; spot++){
            Model_Var_x_ikj[i][spot] = IloNumVarArray(mydata->env, mydata->n, 0, 1, ILOBOOL);
        }
    }
    
    // Tuple indiquant les valeurs des solutions obtenues
    list<tuple<double,int, list<list<list<bool>>> > > solutions;
    
    // Valeur actuelle des objectif pour chaque occurence
    double f1 = 0.0;
    int f2 = 0;
    
    // Compteur de solutions
    int cpt = 0;
    
    // Booleen indiquant qu'une solution existe
    bool solutionFound = 1;
    
    // Booleen indiquant quelle solution est à ajouter
    int which_sol = 0;
    
    // Boucle de d'optimisation
    while(solutionFound){
        
        list<list<list<bool>>> tmp_solution1;
        
        /*##########################
         F1
        ###########################*/
        
        // Modele
        IloModel modelGRP(mydata->env);
        
    
        // Un spot ne peut être alloué qu'une seule fois
        for(int i = 0; i < mydata->Model_m; i++){
            for(int k = 0; k < mydata->Model_s_i[i]; k++){
                IloExpr Ctr1Expr(mydata->env);
                for (int j = 0; j < mydata->Model_n; j++){
                    Ctr1Expr += Model_Var_x_ikj[i][k][j];
                }
                modelGRP.add(Ctr1Expr <= 1);
            }
        }
        
        // Une même marque ne peut apparaitre qu'une seule fois par écran
        for(int i = 0; i < mydata->Model_m; i++){
            for (int j = 0; j < mydata->Model_n; j++){
                IloExpr Ctr0Expr(mydata->env);
                for(int k = 0; k < mydata->Model_s_i[i]; k++){
                    Ctr0Expr += Model_Var_x_ikj[i][k][j];
                }
                modelGRP.add(Ctr0Expr <= 1);
            }
        }
        
        // Ne pas dépasser le budget de chaque marque
        for(int j = 0; j < mydata->Model_n; j++){
            IloExpr Ctr3Expr(mydata->env);
            for(int i = 0; i < mydata->Model_m; i++){
                for(int k = 0; k < mydata->Model_s_i[i]; k++){
                    Ctr3Expr += Model_Var_x_ikj[i][k][j] * mydata->Model_p_ik[i][k] * mydata->Model_d_j[j];
                }
            }
            modelGRP.add(Ctr3Expr <= mydata->Model_b_j[j]);
        }
        
        
        
        // Ne pas dépasser la durée de l'écran publicitaire
        for (int i = 0; i < mydata->Model_m; i++){
            IloExpr Ctr4Expr(mydata->env);
            for(int k = 0; k < mydata->Model_s_i[i]; k++){
                for(int j = 0; j < mydata->Model_n; j++){
                    Ctr4Expr += Model_Var_x_ikj[i][k][j] * mydata->Model_d_j[j];
                }
            }
            modelGRP.add(Ctr4Expr <= mydata->Model_T_i[i]);
        }
         
        
        // Pas de marques compétititives au sein d'un même écran
        for(int j1 = 0; j1 < mydata->Model_n; j1++){
            for(int j2 = 0; j2 < mydata->Model_n; j2++){
                
                if((j1 != j2) && mydata->Model_compare_brands[j1][j2]){
                    for (int i = 0; i < mydata->Model_m; i++){
                        IloExpr Ctr5Expr(mydata->env);
                        for(int k = 0; k < mydata->Model_s_i[i]; k++){
                            Ctr5Expr += Model_Var_x_ikj[i][k][j1] + Model_Var_x_ikj[i][k][j2];
                        }
                        modelGRP.add(Ctr5Expr <= 1);
                    }
                }
            }
        }
        
        
        // Atteindre un niveau de GRP minimal pour chaque marque
        for(int j = 0; j < mydata->Model_n; j++){
            IloExpr Ctr6Expr(mydata->env);
            for (int i = 0; i < mydata->Model_m; i++){
                for(int k = 0; k < mydata->Model_s_i[i]; k++){
                    Ctr6Expr += Model_Var_x_ikj[i][k][j] * mydata->Model_grp_ij[i][j];
                }
            }
            modelGRP.add(Ctr6Expr >= mydata->Model_minGRP_j[j]);
        }
        
        // Ne pas dépasser le pourcentage d'allocation maximal pour prime
        for(int j = 0; j < mydata->Model_n; j++){
            IloExpr Ctr8Expr(mydata->env);
            for (int i = 0; i < mydata->Model_m; i++){
                if(mydata->Model_prime_i[i]){
                    for(int k = 0; k < mydata->Model_s_i[i]; k++){
                        Ctr8Expr += Model_Var_x_ikj[i][k][j] * mydata->Model_p_ik[i][k] * mydata->Model_d_j[j];
                    }
                }
            }
            modelGRP.add(Ctr8Expr <= (mydata->Model_maxPRIME_j[j] / 100) * mydata->Model_b_j[j]);
        }
        
        
        // Ne pas dépasser le pourcentage d'allocation maximal pour premimum
        for(int j = 0; j < mydata->Model_n; j++){
            IloExpr Ctr7Expr(mydata->env);
            for (int i = 0; i < mydata->Model_m; i++){
                for(int k = 0; k < mydata->Model_s_i[i]; k++){
                    if(mydata->Model_premium_ik[i][k]){
                        Ctr7Expr += Model_Var_x_ikj[i][k][j] * mydata->Model_p_ik[i][k] * mydata->Model_d_j[j];
                    }
                }
            }
            modelGRP.add(Ctr7Expr <= (mydata->Model_maxPREMIUM_j[j] / 100) * mydata->Model_b_j[j]);
        }
        
        // Ajout de l'epsilon contrainte sur f2
        if(cpt != 0){
            IloExpr Ctr10Expr(mydata->env);
            
            for(int j = 0; j < mydata->Model_n; j++){
                for (int i = 0; i < mydata->Model_m; i++){
                    for(int k = 0; k < mydata->Model_s_i[i]; k++){
                        Ctr10Expr += Model_Var_x_ikj[i][k][j] * mydata->Model_d_j[j] * mydata->Model_p_ik[i][k];
                    }
                }
            }
            modelGRP.add(Ctr10Expr >= (f2 + 1));
        }
        
        
        // Fonction objectif -> F1 (GRP)
        IloExpr objGRP(mydata->env);
        for(int j = 0; j < mydata->Model_n; j++){
            for (int i = 0; i < mydata->Model_m; i++){
                for(int k = 0; k < mydata->Model_s_i[i]; k++){
                    objGRP += Model_Var_x_ikj[i][k][j] * mydata->Model_grp_ij[i][j];
                }
            }
        }
        
        modelGRP.add(IloMaximize(mydata->env, objGRP));
        objGRP.end();
        
        
        // Résolution F1
        IloCplex monoGRP(mydata->env);
        monoGRP.extract(modelGRP);
        monoGRP.setParam(IloCplex::Threads, 1);
        monoGRP.setParam(IloCplex::SimDisplay, 1);
        monoGRP.setParam(IloCplex::TiLim, 3600);
        monoGRP.setParam(IloCplex::MIPDisplay, 0);
         
        //monoGRP.exportModel("/Users/romu/Desktop/Projets/Stage2022/modelTV.lp");
        
        if (!monoGRP.solve()) {
            mydata->env.error() << "Echec ... Non Lineaire?" << endl;
            solutionFound = 0;
        }
        
        monoGRP.out() << "Solution status: " << monoGRP.getStatus() << endl;
        if (monoGRP.getStatus() == IloAlgorithm::Unbounded){
            monoGRP.out() << "F.O. non born�e." << endl;
            solutionFound = 0;
        }else
        {
            if (monoGRP.getStatus() == IloAlgorithm::Infeasible){
                monoGRP.out() << "Non-realisable." << endl;
                solutionFound = 0;
            }else
            {
                if (monoGRP.getStatus() != IloAlgorithm::Optimal)
                    monoGRP.out() << "Solution Optimale." << endl;
                else
                    monoGRP.out() << "Solution realisable." << endl;
                
                // Valeur des objectifs obtenus
                f1 = (double)(monoGRP.getObjValue());
                
                int tmpF2 = 0;
                
                // Ajout de la solution dans une liste
                for (int i = 0; i < mydata->Model_m; i++)
                {
                    
                    list<list<bool>> break_solution;
                    for(int k = 0; k < mydata->Model_s_i[i]; k++){
                        
                        list<bool> spot_sol;
                        for (int j = 0; j < mydata->n; j++)
                        {
                            tmpF2 += monoGRP.getValue(Model_Var_x_ikj[i][k][j]) * mydata->Model_d_j[j] * mydata->Model_p_ik[i][k];
                            
                            if(monoGRP.getValue(Model_Var_x_ikj[i][k][j]) <= 0.5){
                                spot_sol.push_back(0);
                            }else{
                                spot_sol.push_back(1);
                            }
                        }
                        break_solution.push_back(spot_sol);
                    }
                    tmp_solution1.push_back(break_solution);
                    
                    // On adapte la valeur de la solution à ajouter
                    which_sol = 1;
                }
                
                f2 = tmpF2;
                cout  << f1 << ";" << f2 << endl;
            }
        }
        
        /*##########################
         F2 -> revenus
        ###########################*/
        if(solutionFound){
            
            list<list<list<bool>>> tmp_solution2;
            
            // Modele
            IloModel modelTV(mydata->env);
            
            // Un spot ne peut être alloué qu'une seule fois
            for(int i = 0; i < mydata->Model_m; i++){
                for(int k = 0; k < mydata->Model_s_i[i]; k++){
                    IloExpr Ctr1Expr(mydata->env);
                    for (int j = 0; j < mydata->Model_n; j++){
                        Ctr1Expr += Model_Var_x_ikj[i][k][j];
                    }
                    modelTV.add(Ctr1Expr <= 1);
                }
            }
            
            // Une même marque ne peut apparaitre qu'une seule fois par écran
            for(int i = 0; i < mydata->Model_m; i++){
                for (int j = 0; j < mydata->Model_n; j++){
                    IloExpr Ctr0Expr(mydata->env);
                    for(int k = 0; k < mydata->Model_s_i[i]; k++){
                        Ctr0Expr += Model_Var_x_ikj[i][k][j];
                    }
                    modelTV.add(Ctr0Expr <= 1);
                }
            }
            
            // Ne pas dépasser le budget de chaque marque
            for(int j = 0; j < mydata->Model_n; j++){
                IloExpr Ctr3Expr(mydata->env);
                for(int i = 0; i < mydata->Model_m; i++){
                    for(int k = 0; k < mydata->Model_s_i[i]; k++){
                        Ctr3Expr += Model_Var_x_ikj[i][k][j] * mydata->Model_p_ik[i][k] * mydata->Model_d_j[j];
                    }
                }
                modelTV.add(Ctr3Expr <= mydata->Model_b_j[j]);
            }
            
            
            
            // Ne pas dépasser la durée de l'écran publicitaire
            for (int i = 0; i < mydata->Model_m; i++){
                IloExpr Ctr4Expr(mydata->env);
                for(int k = 0; k < mydata->Model_s_i[i]; k++){
                    for(int j = 0; j < mydata->Model_n; j++){
                        Ctr4Expr += Model_Var_x_ikj[i][k][j] * mydata->Model_d_j[j];
                    }
                }
                modelTV.add(Ctr4Expr <= mydata->Model_T_i[i]);
            }
            
            
            // Pas de marques compétititives au sein d'un même écran
            for(int j1 = 0; j1 < mydata->Model_n; j1++){
                for(int j2 = 0; j2 < mydata->Model_n; j2++){
                    
                    if((j1 != j2) && mydata->Model_compare_brands[j1][j2]){
                        for (int i = 0; i < mydata->Model_m; i++){
                            IloExpr Ctr5Expr(mydata->env);
                            for(int k = 0; k < mydata->Model_s_i[i]; k++){
                                Ctr5Expr += Model_Var_x_ikj[i][k][j1] + Model_Var_x_ikj[i][k][j2];
                            }
                            modelTV.add(Ctr5Expr <= 1);
                        }
                    }
                }
            }
            
            
            // Atteindre un niveau de GRP minimal pour chaque marque
            for(int j = 0; j < mydata->Model_n; j++){
                IloExpr Ctr6Expr(mydata->env);
                for (int i = 0; i < mydata->Model_m; i++){
                    for(int k = 0; k < mydata->Model_s_i[i]; k++){
                        Ctr6Expr += Model_Var_x_ikj[i][k][j] * mydata->Model_grp_ij[i][j];
                    }
                }
                modelTV.add(Ctr6Expr >= mydata->Model_minGRP_j[j]);
            }
            
            
            // Ne pas dépasser le pourcentage d'allocation maximal pour premimum
            for(int j = 0; j < mydata->Model_n; j++){
                IloExpr Ctr7Expr(mydata->env);
                for (int i = 0; i < mydata->Model_m; i++){
                    for(int k = 0; k < mydata->Model_s_i[i]; k++){
                        if(mydata->Model_premium_ik[i][k]){
                            Ctr7Expr += Model_Var_x_ikj[i][k][j] * mydata->Model_p_ik[i][k] * mydata->Model_d_j[j];
                        }
                    }
                }
                modelTV.add(Ctr7Expr <= (mydata->Model_maxPREMIUM_j[j] / 100) * mydata->Model_b_j[j]);
            }
            
            
            // Ne pas dépasser le pourcentage d'allocation maximal pour prime
            for(int j = 0; j < mydata->Model_n; j++){
                IloExpr Ctr8Expr(mydata->env);
                for (int i = 0; i < mydata->Model_m; i++){
                    if(mydata->Model_prime_i[i]){
                        for(int k = 0; k < mydata->Model_s_i[i]; k++){
                            Ctr8Expr += Model_Var_x_ikj[i][k][j] * mydata->Model_p_ik[i][k] * mydata->Model_d_j[j];
                        }
                    }
                }
                modelTV.add(Ctr8Expr <= (mydata->Model_maxPRIME_j[j] / 100) * mydata->Model_b_j[j]);
            }
            
            
            // Ajout de l'epsilon contrainte sur F2
            IloExpr Ctr10Expr(mydata->env);
            for(int j = 0; j < mydata->Model_n; j++){
                for (int i = 0; i < mydata->Model_m; i++){
                    for(int k = 0; k < mydata->Model_s_i[i]; k++){
                        Ctr10Expr += Model_Var_x_ikj[i][k][j] * mydata->Model_d_j[j] * mydata->Model_p_ik[i][k];
                    }
                }
            }
            modelTV.add(Ctr10Expr >= (f2 + 1));
            
            
            // Ajout contrainte d'égalité sur F1
            IloExpr Ctr11Expr(mydata->env);
            for(int j = 0; j < mydata->Model_n; j++){
                for (int i = 0; i < mydata->Model_m; i++){
                    for(int k = 0; k < mydata->Model_s_i[i]; k++){
                        Ctr11Expr += Model_Var_x_ikj[i][k][j] * mydata->Model_grp_ij[i][j];
                    }
                }
            }
            modelTV.add(Ctr11Expr <= f1+0.0001);
            modelTV.add(Ctr11Expr >= f1-0.0001);
            
            
            // Fonction objectif -> F2 (TV)
            IloExpr objTV(mydata->env);
            for(int j = 0; j < mydata->Model_n; j++){
                for (int i = 0; i < mydata->Model_m; i++){
                    for(int k = 0; k < mydata->Model_s_i[i]; k++){
                        objTV += Model_Var_x_ikj[i][k][j] * mydata->Model_d_j[j] * mydata->Model_p_ik[i][k];
                    }
                }
            }
            modelTV.add(IloMaximize(mydata->env, objTV));
            objTV.end();
            
            // Résolution F2 bis
            IloCplex monoTV(mydata->env);
            monoTV.extract(modelTV);
            monoTV.setParam(IloCplex::Threads, 1);
            monoTV.setParam(IloCplex::SimDisplay, 1);
            monoTV.setParam(IloCplex::TiLim, 3600);
            monoTV.setParam(IloCplex::MIPDisplay, 0);
            
            // Resolution
            if (!monoTV.solve()) {
                mydata->env.error() << "Echec ... Non Lineaire?" << endl;
            }
            
            monoTV.out() << "Solution status: " << monoTV.getStatus() << endl;
            if (monoTV.getStatus() == IloAlgorithm::Unbounded){
                monoTV.out() << "F.O. non bornée." << endl;
            }else
            {
                if (monoTV.getStatus() == IloAlgorithm::Infeasible){
                    monoTV.out() << "Non-realisable." << endl;
                }else
                {
                    if (monoTV.getStatus() != IloAlgorithm::Optimal)
                        monoTV.out() << "Solution Optimale." << endl;
                    else
                        monoTV.out() << "Solution realisable." << endl;
                    
                    // Valeur des objectifs obtenus
                    f2 = (int)(monoTV.getObjValue());
                    
                    double tempF1 = 0;
                    
                    // Ajout de la solution dans une liste
                    for (int i = 0; i < mydata->Model_m; i++)
                    {
                        list<list<bool>> break_solution;
                        
                        for(int k = 0; k < mydata->Model_s_i[i]; k++){
                            
                            list<bool> spot_sol;
                            for (int j = 0; j < mydata->n; j++)
                            {
                                tempF1 += monoTV.getValue(Model_Var_x_ikj[i][k][j]) * mydata->Model_grp_ij[i][j];
          
                                if(monoTV.getValue(Model_Var_x_ikj[i][k][j]) <= 0.5){
                                    spot_sol.push_back(0);
                                }else{
                                    spot_sol.push_back(1);
                                }
                                
                            }
                            break_solution.push_back(spot_sol);
                        }
                        tmp_solution2.push_back(break_solution);
                        
                        // On adapte la valeur de la solution à ajouter
                        which_sol = 2;
                    }
                    f1 = tempF1;
                }
            }
            
            // Permet d'ajouter le type de solution obtenu pour cette occurrence
            if(which_sol == 1){
                // Solution optimale obtenue dès le début
                solutions.push_back(make_tuple(f1, f2, tmp_solution1));
            }else{
                // Solution obtenue de base optimisée
                solutions.push_back(make_tuple(f1, f2, tmp_solution2));
            }
            
            // Incrémentation du nombre d'occurences
            cpt++;
        }
    }
    
    // Affichage des valeurs des objectifs pour chaque solution
    for(auto cpt = solutions.begin(); cpt != solutions.end(); cpt++){
        cout << get<0>(*cpt) << " ; " << get<1>(*cpt) << endl;
        
    }
    
    // Fin epsilon
    cout << "Finish epsilon optimization" << endl;
    cout << "Number of occurence : " << cpt << endl;
    

    /*##########################
     JSON Result file
    ###########################*/
    json solution_file;
    
    
    // Parcours de chaque solution pour l'ajouter dans le fichier de solution final
    int cpt_final = 0;
    for(auto cpt_sol = solutions.begin(); cpt_sol != solutions.end(); cpt_sol++){
        
        // Ecriture dans le fichier de la valeur des objectifs obtenus
        solution_file["Solution "+ to_string(cpt_final)]["GRP"] = get<0>(*cpt_sol);
        solution_file["Solution "+ to_string(cpt_final)]["revenus globaux"] = get<1>(*cpt_sol);
        
        list<list<list<bool>>> repart = get<2>(*cpt_sol);
        
        int break_cpt = 0;
        for(auto break_tmp = repart.begin(); break_tmp != repart.end(); break_tmp++){
            
            list<list<bool>> & spots = *break_tmp;
            
            int spot_cpt = 0;
            for(auto spot_tmp = spots.begin(); spot_tmp != spots.end(); spot_tmp++){
                
                list<bool> & brands = *spot_tmp;
                
                int brand_cpt = 0;
                for(auto brand_tmp = brands.begin(); brand_tmp != brands.end(); brand_tmp++){
                    
                    // Ecriture dans le fichier de l'information de la presence ou non de chaque marque, pour chaque spot de chaque ecran
                    solution_file["Solution "+ to_string(cpt_final)]["Allocation"]["Ecran num "+ to_string(break_cpt)]["Spot num "+ to_string(spot_cpt)]["Marque num "+ to_string(brand_cpt)] = *brand_tmp;
                    
                    brand_cpt++;
                }
                
                spot_cpt++;
            }
            
            break_cpt++;
        }
        
        cpt_final++;
    }
    
    // Ajout du nombre de solutions obtenues dans l'instance d'optimisation
    solution_file["Nombre solutions total"] = cpt_final;
    
    // Nom du fichier
    string file_name = "result_file_epsilon.json";
   
    // Ecriture de la solution obtenue dans un fichier JSON
    std::ofstream file(file_name);
    
    // Permet au fichier d'etre correctement indenté
    file << std::setw(4) << solution_file << std::endl;
    
    cout << "Results file written on : " << file_name << endl;
}
