#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <set>
#include <cmath>
using namespace std;

#define UNDEF -1
#define TRUE 1
#define FALSE 0

uint numVars;
uint numClauses;
vector<vector<int> > clauses;
vector<int> model;
vector<int> modelStack;
uint indexOfNextLitToPropagate;
uint decisionLevel;

struct varsHFreq {
    uint lit;
    uint freq;
    //aqui se pueden poner futuros atributos de heuristicas mas interesantes supongo?
};

bool CompH (varsHFreq a, varsHFreq b) {
    return (a.freq > b.freq);
}//funcion sujeta a futuras modificaciones segun mas atributos se incorporen?

uint constant_X; //constante para cambiar entre modos de heuristica
set<pair<int, int>>setHAct;//set para la actividad
vector<uint> varFreq; //Frequencia de las variables (sin distinguir si estan negadas)
vector<varsHFreq> vectorHFreq; //Vector para ordenar heuristicamente las variables
vector< vector<uint> > vecOcurrPos;//vec de clausulas que contienen la variable para el acceso aleatorio en la propagacion
vector< vector<uint> > vecOcurrNeg;

void computeHeuristic () { //inicializacion de la heuristica
    vectorHFreq.resize(numVars);
    for (uint i = 1; i < numVars+1; i++) {
        vectorHFreq[i-1].lit = i;
        vectorHFreq[i-1].freq = varFreq[i];
    }
    sort(vectorHFreq.begin(),vectorHFreq.end(),CompH); //sort de mayor a menor frequencia
}

void updateHeuristic (int indexClause) {
    if ( (modelStack.size()-decisionLevel) > constant_X) {
        
        
    }
}

void readClauses () {
  // Skip comments
  char c = cin.get();
  while (c == 'c') {
    while (c != '\n') c = cin.get();
    c = cin.get();
  }  
  // Read "cnf numVars numClauses"
  string aux;
  cin >> aux >> numVars >> numClauses;
  clauses.resize(numClauses);
  varFreq.resize(numVars+1,0);
  vecOcurrPos.resize(numVars+1);
  vecOcurrNeg.resize(numVars+1);
  constant_X = 5*ceil(log10((float)numVars));
  // Read clauses
  for (uint i = 0; i < numClauses; i++) {
    int lit;
    while (cin >> lit and lit != 0) {
        clauses[i].push_back(lit);
        varFreq[abs(lit)] += 1; //leer frequencia
        //ocurrencias
        if (lit < 0) vecOcurrPos[abs(lit)].push_back(i);
        else vecOcurrNeg[abs(lit)].push_back(i);
    }
  }
}

int currentValueInModel (int lit) {
  if (lit >= 0) return model[lit];
  else {
    if (model[-lit] == UNDEF) return UNDEF;
    else return 1 - model[-lit];
  }
}

void setLiteralToTrue (int lit) {
  modelStack.push_back(lit);
  if (lit > 0) model[lit] = TRUE;
  else model[-lit] = FALSE;		
}

bool propagateGivesConflict () {
  while ( indexOfNextLitToPropagate < modelStack.size() ) {
    int LitToPropagate = modelStack[indexOfNextLitToPropagate];
    vector<uint>* vecOcurrOpuesta; //puntero al vector de las clausulas positivas o negativas correspondientemente
    if (LitToPropagate > 0) {
        vecOcurrOpuesta = &vecOcurrPos[abs(LitToPropagate)];
    } else {
        vecOcurrOpuesta = &vecOcurrNeg[abs(LitToPropagate)];
    }
    for (uint i = 0; i < vecOcurrOpuesta->size(); ++i) {
      bool someLitTrue = false;
      int lastLitUndef = 0;
      uint numUndefs = 0;
      uint indexClause = (*vecOcurrOpuesta)[i];
      //if numUndefs >= 2 propagation is not possible
      for (int k = 0; not someLitTrue and numUndefs < 2 and k < clauses[indexClause].size(); ++k){
          int val = currentValueInModel(clauses[indexClause][k]);
          if (val == TRUE) someLitTrue = true;
          else if (val == UNDEF){
              ++numUndefs;
              lastLitUndef = clauses[indexClause][k];
              //comapre temp in local clause
        }
      }
      if (not someLitTrue and numUndefs == 0) {
          updateHeuristic(indexClause);
          return true; // conflict! all lits false
      } else if (not someLitTrue and numUndefs == 1) setLiteralToTrue(lastLitUndef);
    }
    indexOfNextLitToPropagate++;
  }
  return false;
}

void backtrack () {
    int lit = 0;
    uint i = modelStack.size() -1;
    while (modelStack[i] != 0){ // 0 is the DL mark
        lit = modelStack[i];
        model[abs(lit)] = UNDEF;
        modelStack.pop_back();
        --i;
    }
    // at this point, lit is the last decision
    modelStack.pop_back(); // remove the DL mark
    --decisionLevel;
    indexOfNextLitToPropagate = modelStack.size();
    setLiteralToTrue(-lit);  // reverse last decision
}

// Heuristic for finding the next decision literal:
uint getNextDecisionLiteral () {
    //solo dedicate a perder tiempo revisando todo si estas al principio (pocas decisiones y pocas propagaciones hechas) ya que recorres solo la parte primera del vector
    //tambien es un for exaustivo que no he comprobado si actua de hole filler por si la prediccion anterior falla
    uint nextLit_index;
    for (uint i = 0; i < numVars; i++){
        nextLit_index = vectorHFreq[i].lit;
        if (model[nextLit_index] == UNDEF) {
            return nextLit_index;
        }
    }
    return 0;
}

void checkmodel () {
  for (uint i = 0; i < numClauses; ++i) {
    bool someTrue = false;
    for (uint j = 0; not someTrue and j < clauses[i].size(); ++j)
      someTrue = (currentValueInModel(clauses[i][j]) == TRUE);
    if (not someTrue) {
      cout << "Error in model, clause is not satisfied:";
      for (uint j = 0; j < clauses[i].size(); ++j) cout << clauses[i][j] << " ";
      cout << endl;
      exit(1);
    }
  }  
}

int main () {
  readClauses(); // reads numVars, numClauses and clauses
  model.resize(numVars+1,UNDEF);
  indexOfNextLitToPropagate = 0;  
  decisionLevel = 0;
  computeHeuristic(); //Prepara y ordena el vector vectorHFreq
  // Take care of initial unit clauses, if any
  for (uint i = 0; i < numClauses; ++i)
    if (clauses[i].size() == 1) {
      int lit = clauses[i][0];
      int val = currentValueInModel(lit);
      if (val == FALSE) {cout << "UNSATISFIABLE" << endl; return 10;}
      else if (val == UNDEF) setLiteralToTrue(lit);
    }
  // DPLL algorithm
  while (true) {
    while ( propagateGivesConflict() ) {
      if ( decisionLevel == 0) { cout << "UNSATISFIABLE" << endl; return 10; }
      backtrack();
    }
    uint decisionLit = getNextDecisionLiteral();
    if (decisionLit == 0) { checkmodel(); cout << "SATISFIABLE" << endl; return 20; }
    // start new decision level:
    modelStack.push_back(0); // push mark indicating new DL
    ++indexOfNextLitToPropagate;
    ++decisionLevel;
    setLiteralToTrue(decisionLit); // now push decisionLit on top of the mark
  }
}  
