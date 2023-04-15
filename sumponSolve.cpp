//
//  sumponSolve.cpp
//  CPLEX_Test
//
//  Created by Romuald Duret on 31/03/2023.
//

#include "sumponSolve.hpp"
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include<string.h>
#include "json.hpp"
#include <list>
#include <stack>
#include <tuple>
#include <iterator>
#include <ilcplex/ilocplex.h>
#include "Data.hpp"



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
void sumponSolve(Data * mydata){
    
    // Compteur de solutions
    int cpt = 0;
    
    // Structure d'une solution
    struct Solution {
        double grp;
        int revenus_TV;
        list<list<list<bool>>> allocation;
    };
    
    // Liste des solutions
    list<Solution> solutions;
    
    /*
     pile de Solution
     */
    stack<tuple<Solution, Solution>> pile_solutions;
    
    /*
     Variable de resolution CPLEX
     */
    // VARIABLE : x_ikj
    IloArray<IloArray<IloNumVarArray>> Model_Var_x_ikj(mydata->env, mydata->m);
    for(int i = 0; i< mydata->m; i++){
        Model_Var_x_ikj[i] = IloArray<IloNumVarArray>(mydata->env, mydata->Model_s_i[i]);
        for(int spot = 0; spot < mydata->Model_s_i[i]; spot++){
            Model_Var_x_ikj[i][spot] = IloNumVarArray(mydata->env, mydata->n, 0, 1, ILOBOOL);
        }
    }
    
    
    /*
     Résoudre mono-objectif revenus -> sol2
     */
    Solution sol2;
    
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
    
    
    // Résolution monoTV
    IloCplex monoTV(mydata->env);
    monoTV.extract(modelTV);
    monoTV.setParam(IloCplex::Threads, 1);
    monoTV.setParam(IloCplex::SimDisplay, 1);
    monoTV.setParam(IloCplex::TiLim, 3600);
    monoTV.setParam(IloCplex::MIPDisplay, 0);
    
    if (!monoTV.solve()) {
        mydata->env.error() << "Echec ... Non Lineaire?" << endl;
    }
    
    monoTV.out() << "Solution status: " << monoTV.getStatus() << endl;
    if (monoTV.getStatus() == IloAlgorithm::Unbounded){
        monoTV.out() << "F.O. non born�e." << endl;
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
            
            list<list<list<bool>>> solutionTV;
            
            // Valeur des objectifs obtenus
            sol2.revenus_TV = (int)(monoTV.getObjValue());
            
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
                solutionTV.push_back(break_solution);
            }
            
            sol2.grp = tempF1;
            sol2.allocation = solutionTV;
        }
    }
    
    /*
     Résoudre mono-objectif GRP -> sol1
     */
    
    // Modele
    IloModel modelGRP(mydata->env);
    
    Solution sol1;

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
        //solutionFound = 0;
    }
    
    monoGRP.out() << "Solution status: " << monoGRP.getStatus() << endl;
    if (monoGRP.getStatus() == IloAlgorithm::Unbounded){
        monoGRP.out() << "F.O. non born�e." << endl;
        //solutionFound = 0;
    }else
    {
        if (monoGRP.getStatus() == IloAlgorithm::Infeasible){
            monoGRP.out() << "Non-realisable." << endl;
            //solutionFound = 0;
        }else
        {
            if (monoGRP.getStatus() != IloAlgorithm::Optimal)
                monoGRP.out() << "Solution Optimale." << endl;
            else
                monoGRP.out() << "Solution realisable." << endl;
            
            list<list<list<bool>>> solutionGRP;
            
            // Valeur des objectifs obtenus
            sol1.grp = (double)(monoGRP.getObjValue());
            
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
                solutionGRP.push_back(break_solution);
            }
            
            sol1.revenus_TV = tmpF2;
            
            sol1.allocation = solutionGRP;
        }
    }
    
    
    // Ajouter tuple de solution dans la pile
    
    cout << sol2.grp << ";" << sol2.revenus_TV << endl;
    cout << sol1.grp << ";" << sol1.revenus_TV << endl;
    
    //make_tuple(f1, f2, tmp_solution2)
    
    pile_solutions.push(make_tuple(sol2,sol1));
    
    
    //  271.797;5751700
    
    
    /*
     A faire tant que la pile n'est pas vide
     */
    while(!pile_solutions.empty()){
        
        cout << "#### NEW ITERATION " << cpt <<" ####" << endl;
        
        // Prendre premier tuple (sol1,sol2) de la pile (top() et pop())
        tuple<Solution, Solution> tuple_sol = pile_solutions.top();
        pile_solutions.pop();
        cout << "sol2 : "<< get<0>(tuple_sol).grp << ";" << get<0>(tuple_sol).revenus_TV << endl;
        cout << "sol1 : "<<  get<1>(tuple_sol).grp << ";" << get<1>(tuple_sol).revenus_TV << endl;
        
        // Calculer coef (yb-ya/xa-xb)
        float coef = (get<1>(tuple_sol).revenus_TV - get<0>(tuple_sol).revenus_TV)/(get<0>(tuple_sol).grp - get<1>(tuple_sol).grp);
        cout << "COEF : " << coef << endl;
        
        // Determiner alpha{alpha1,alpha2}
        tuple<float, float> alpha = make_tuple((coef/(coef+1)),(1/(coef+1)));
        cout << "ALPHA : " <<  get<0>(alpha) << " ; "<<  get<1>(alpha) << endl;
        
        // MILP avec fo avec alpha
            Solution sol0;
            
            bool solutionFound = 1;
            
            // Modele
            IloModel modelSum(mydata->env);
            
            // Un spot ne peut être alloué qu'une seule fois
            for(int i = 0; i < mydata->Model_m; i++){
                for(int k = 0; k < mydata->Model_s_i[i]; k++){
                    IloExpr Ctr1Expr(mydata->env);
                    for (int j = 0; j < mydata->Model_n; j++){
                        Ctr1Expr += Model_Var_x_ikj[i][k][j];
                    }
                    modelSum.add(Ctr1Expr <= 1);
                }
            }
            
            // Une même marque ne peut apparaitre qu'une seule fois par écran
            for(int i = 0; i < mydata->Model_m; i++){
                for (int j = 0; j < mydata->Model_n; j++){
                    IloExpr Ctr0Expr(mydata->env);
                    for(int k = 0; k < mydata->Model_s_i[i]; k++){
                        Ctr0Expr += Model_Var_x_ikj[i][k][j];
                    }
                    modelSum.add(Ctr0Expr <= 1);
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
                modelSum.add(Ctr3Expr <= mydata->Model_b_j[j]);
            }
            
            
            
            // Ne pas dépasser la durée de l'écran publicitaire
            for (int i = 0; i < mydata->Model_m; i++){
                IloExpr Ctr4Expr(mydata->env);
                for(int k = 0; k < mydata->Model_s_i[i]; k++){
                    for(int j = 0; j < mydata->Model_n; j++){
                        Ctr4Expr += Model_Var_x_ikj[i][k][j] * mydata->Model_d_j[j];
                    }
                }
                modelSum.add(Ctr4Expr <= mydata->Model_T_i[i]);
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
                            modelSum.add(Ctr5Expr <= 1);
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
                modelSum.add(Ctr6Expr >= mydata->Model_minGRP_j[j]);
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
                modelSum.add(Ctr8Expr <= (mydata->Model_maxPRIME_j[j] / 100) * mydata->Model_b_j[j]);
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
                modelSum.add(Ctr7Expr <= (mydata->Model_maxPREMIUM_j[j] / 100) * mydata->Model_b_j[j]);
            }
            
            // Fonction objectif -> WeightSum (alpha1 * (F1)) + (alpha2 * (F2))
            IloExpr objSumFinal(mydata->env);
            
            IloExpr objSumGRP(mydata->env);
            for(int j = 0; j < mydata->Model_n; j++){
                for (int i = 0; i < mydata->Model_m; i++){
                    for(int k = 0; k < mydata->Model_s_i[i]; k++){
                        objSumGRP += Model_Var_x_ikj[i][k][j] * mydata->Model_grp_ij[i][j];
                    }
                }
            }
            objSumGRP *= get<0>(alpha);
            
            objSumFinal += objSumGRP;
            
            IloExpr objSumTV(mydata->env);
            for(int j = 0; j < mydata->Model_n; j++){
                for (int i = 0; i < mydata->Model_m; i++){
                    for(int k = 0; k < mydata->Model_s_i[i]; k++){
                        objSumTV += Model_Var_x_ikj[i][k][j] * mydata->Model_d_j[j] * mydata->Model_p_ik[i][k];
                    }
                }
            }
            objSumTV *= get<1>(alpha);
            
            objSumFinal += objSumTV;
            
            modelSum.add(IloMaximize(mydata->env, objSumFinal));
            objSumFinal.end();
            
            
            // Résolution WeightSum
            IloCplex monoSum(mydata->env);
            monoSum.extract(modelSum);
            monoSum.setParam(IloCplex::Threads, 1);
            monoSum.setParam(IloCplex::SimDisplay, 1);
            monoSum.setParam(IloCplex::TiLim, 3600);
            monoSum.setParam(IloCplex::MIPDisplay, 0);
            
            //monoGRP.exportModel("/Users/romu/Desktop/Projets/Stage2022/modelTV.lp");
            
            if (!monoSum.solve()) {
                mydata->env.error() << "Echec ... Non Lineaire?" << endl;
                solutionFound = 0;
            }
            
            monoSum.out() << "Solution status: " << monoSum.getStatus() << endl;
            if (monoSum.getStatus() == IloAlgorithm::Unbounded){
                monoSum.out() << "F.O. non born�e." << endl;
                solutionFound = 0;
            }else
            {
                if (monoSum.getStatus() == IloAlgorithm::Infeasible){
                    monoSum.out() << "Non-realisable." << endl;
                    solutionFound = 0;
                }else
                {
                    if (monoSum.getStatus() != IloAlgorithm::Optimal)
                        monoSum.out() << "Solution Optimale." << endl;
                    else
                        monoSum.out() << "Solution realisable." << endl;
                
                    list<list<list<bool>>> solutionSUM;;
                    
                    float tmpF1 = 0.;
                    
                    int tmpF2 = 0;
                    
                    // Ajout de la solution dans une liste
                    for (int i = 0; i < mydata->Model_m; i++)
                    {
                        
                        list<list<bool>> break_solution;
                        
                        for(int k = 0; k < mydata->Model_s_i[i]; k++){
                            
                            list<bool> spot_sol;
                            for (int j = 0; j < mydata->n; j++)
                            {
                                tmpF2 += monoSum.getValue(Model_Var_x_ikj[i][k][j]) * mydata->Model_d_j[j] * mydata->Model_p_ik[i][k];
                                
                                tmpF1 += monoSum.getValue(Model_Var_x_ikj[i][k][j]) * mydata->Model_grp_ij[i][j];
                                
                                if(monoGRP.getValue(Model_Var_x_ikj[i][k][j]) <= 0.5){
                                    spot_sol.push_back(0);
                                }else{
                                    spot_sol.push_back(1);
                                }
                            }
                            break_solution.push_back(spot_sol);
                        }
                        solutionSUM.push_back(break_solution);
                        
                    }
                    
                    sol0.revenus_TV = tmpF2;
                    sol0.grp = tmpF1;
                    sol0.allocation = solutionSUM;
                }
            }
            
            
        // Si solution  trouvée sol0 et que la solution est différente
        if(solutionFound && (get<0>(tuple_sol).revenus_TV != sol0.revenus_TV && get<0>(tuple_sol).grp < sol0.grp) && (get<1>(tuple_sol).revenus_TV < sol0.revenus_TV && get<1>(tuple_sol).grp != sol0.grp)){
            
            
            cout  << "Solution found for " << cpt << " : " << sol0.grp << ";" << sol0.revenus_TV << endl;
            pile_solutions.push(make_tuple(get<0>(tuple_sol),sol0));
            pile_solutions.push(make_tuple(sol0,get<1>(tuple_sol)));
            
            
            // Ajouter dans liste de solution sol0
            solutions.push_back(sol0);
            
            // Incrémentation du compteur de solutions
            cpt++;

            
        }
        
    }
        
    
    
    // Affichage des valeurs des objectifs pour chaque solution
    for(auto cpt = solutions.begin(); cpt != solutions.end(); cpt++){
        cout << (*cpt).grp << " ; " << (*cpt).revenus_TV << endl;
        
    }
    
    
    // Fin epsilon
    cout << "Finish weight sum optimization" << endl;
    cout << "Number of occurence : " << cpt << endl;
    

    /*##########################
     JSON Result file
    ###########################*/
    json solution_file;
    
    
    // Parcours de chaque solution pour l'ajouter dans le fichier de solution final
    int cpt_final = 0;
    for(auto cpt_sol = solutions.begin(); cpt_sol != solutions.end(); cpt_sol++){
        
        // Ecriture dans le fichier de la valeur des objectifs obtenus
        solution_file["Solution "+ to_string(cpt_final)]["GRP"] = (*cpt_sol).grp;
        solution_file["Solution "+ to_string(cpt_final)]["revenus globaux"] = (*cpt_sol).revenus_TV;
        
        list<list<list<bool>>> repart = (*cpt_sol).allocation;
        
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
    string file_name = "result_file_weightsum.json";
   
    // Ecriture de la solution obtenue dans un fichier JSON
    std::ofstream file(file_name);
    
    // Permet au fichier d'etre correctement indenté
    file << std::setw(4) << solution_file << std::endl;
    
    cout << "Results file written on : " << file_name << endl;
}
