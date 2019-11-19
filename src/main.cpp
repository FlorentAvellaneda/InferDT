
#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <tuple>
#include <cassert>
#include <memory>
#include <deque>

#include "xdot.h"

#include "coutUtil.h"

#include "Chrono.h"
#include "CSV.h"
#include "Moy.h"

#include "CLI11.hpp"

#include <minisat/core/Solver.h>


using namespace MaLib;
using namespace std;
using namespace Minisat;





unsigned int pow2(unsigned int p) {
    return 1 << p;
}


bool isDouble(string s) {
    bool virgule = false;
    bool first=true;
    int e=0;
    for(auto c: s) {
        if(e==1) {
            if(c=='-' || c=='+') {
                e++;
                continue;
            }
            return false;
        }
        if(first) {
            if(c=='-')
                continue;
            first=false;
        } else {
            if(e==0) {
                if(c=='e') {
                    e++;
                    continue;
                }
            }


        }
        if(c>='0' && c<='9')
            continue;
        if(c=='.')
            if(!virgule) {
                virgule = true;
                continue;
            }
        return false;
    }
    return true;
}

void replace(std::vector<int> &tree, unsigned long a, unsigned long b) {
    std::deque< tuple<unsigned long, unsigned long> > aRemplacer = { make_tuple(a, b) };

    while( aRemplacer.size() ) {

        std::tie(a,b) = aRemplacer.front();
        aRemplacer.pop_front();

        if(b < tree.size()) {
            tree[a] = tree[b];
            aRemplacer.push_back( make_tuple( a*2, b*2 ) );
            aRemplacer.push_back( make_tuple( a*2+1, b*2+1 ) );
        } else {
            tree[a] = std::numeric_limits<int>::min()+1;
        }

    }
}


void simplifyTree(std::vector<int> &tree) {

    for(unsigned long i=tree.size()-1; i>0; i--) {

        if(tree[i] == std::numeric_limits<int>::min()) {
            unsigned long frere;
            if(i%2) {
                frere = i-1;
            } else {
                frere = i+1;
            }

            replace(tree, i/2, frere);
        }
    }

}

std::string tree2dot(std::vector<int> tree) {
    ostringstream oss;

oss << "digraph mon_graphe {" << endl;

    if(tree.size() != 0) {


        for(unsigned int i=1; i<tree.size(); i++) {

            if(tree[i] < 0) {
                if(tree[i] != std::numeric_limits<int>::min()+1)
                    oss << "Q" << i << " [label=\"" << (-tree[i]-1) <<"\"];" << std::endl;
            } else {
                oss << "Q" << i << " [label=\"Feature "<< tree[i] <<"?\"];" << std::endl;

                oss << "Q" << i << " -> Q" << (i*2) << " [label=\"no\"];" << std::endl;
                oss << "Q" << i << " -> Q" << (i*2+1) << " [label=\"yes\"];" << std::endl;
            }

        }
    }

    oss << "}" << endl;

    return oss.str();
}





bool verifierAppartient(std::vector<int> &inferedTree, std::vector<int> &e, int classe) {

    std::vector<std::pair<unsigned int, std::vector<int>>> pos = { std::pair(1, std::vector<int>(e.size()))};

    while(pos.size()) {
        if(inferedTree[pos[0].first] < 0) {
            if( -inferedTree[pos[0].first]-1 != classe ) {
                return false;
            }
            pos[0] = pos.back();
            pos.pop_back();
        } else {
            assert( inferedTree[pos[0].first] >= 0);
            unsigned int feature = static_cast<unsigned int>(inferedTree[pos[0].first]);

            if( e[feature] == 1 ) {
                pos[0].first = pos[0].first * 2 + 1;
            } else if( e[feature] == -1 ) {
                pos[0].first = pos[0].first * 2;
            } else {
                assert(e[feature] == 0);

                if( pos[0].second[feature] == 1 ) {
                    pos[0].first = pos[0].first * 2 + 1;
                } else if( pos[0].second[feature] == -1 ) {
                    pos[0].first = pos[0].first * 2;
                } else {
                    pos[0].second[feature] = -1;
                    pos[0].first = pos[0].first * 2;
                    pos.push_back( std::pair(pos[0].first+1, pos[0].second));
                    pos.back().second[feature] = 1;
                }
            }
        }
    }

    return true;
}


bool verifier(unsigned int nbFeature, std::vector<int> &generateur, std::vector<int> &inferedTree, unsigned int pos = 1, std::vector<int> element = std::vector<int>()) {

    if(element.size() == 0)
        element.resize(nbFeature);

    if(generateur[pos] < 0) {
        int classe = -generateur[pos]-1;
        return verifierAppartient(inferedTree, element, classe);
    }

    assert( generateur[pos] >= 0 );
    unsigned int feature = static_cast<unsigned int>(generateur[pos]);

    if(element[feature] == 1)
        return verifier(nbFeature, generateur, inferedTree, pos*2+1, element);
    if(element[feature] == -1)
        return verifier(nbFeature, generateur, inferedTree, pos*2, element);

    element[feature] = 1;
    if( !verifier(nbFeature, generateur, inferedTree, pos*2+1, element) )
        return false;

    element[feature] = -1;
    if( !verifier(nbFeature, generateur, inferedTree, pos*2, element))
        return false;

    element[feature] = 0;

    return true;
}

bool verifier(std::vector<int> &tree, const vector< vector< vector<bool> > > &S) {

    for(unsigned int classe=0; classe<S.size(); classe++) {

        for(auto &e: S[classe]) {

            unsigned int pos = 1;

            while( tree[pos] >= 0) {

                assert( tree[pos] >= 0 );
                unsigned int feature = static_cast<unsigned int>( tree[pos] );

                if( e[feature] ) {
                    pos = pos * 2 + 1;
                } else {
                    pos = pos * 2;
                }

            }

            assert( -tree[pos]-1 >= 0 );
            if( static_cast<unsigned int>( -tree[pos]-1 ) != classe ) {
                return false;
            }
        }
    }

    return true;
}



static unsigned int nombreElement = 0;
static unsigned int nbClauses = 0;
static unsigned int kFind = 0;
static unsigned int nbNodes = 0;


class Infering {
    Minisat::Solver * solver;

    class VarSAT {
        static Minisat::Solver* solver;
     public:
        int id;
        VarSAT() {
            id = solver->newVar();
        }

        static void init(Minisat::Solver* solver) {
            VarSAT::solver = solver;
        }

        static int newVar() {
            return solver->newVar();
        }

        friend class Infering;
    };

    class VarAux {
        static Minisat::Solver* solver;
     public:
        int id;
        VarAux() {
            id = solver->newVar(l_Undef, false);
        }

        static void init(Minisat::Solver* solver) {
            VarAux::solver = solver;
        }

        static int newVar() {
            return solver->newVar(l_Undef, false);
        }

        friend class Infering;
    };





    bool toUpdate=true;
    bool lastResult=false;
    unsigned int k;
    unsigned int nbFeature;
    unsigned int nbClasse;


    std::vector<std::string> name;


    std::vector<std::vector<VarSAT>> X;
    int getVarX(unsigned int e, unsigned int j) {
        if(e >= X.size())
            X.resize(e+1);

        if(j >= X[e].size())
            X[e].resize(j+1);

        return X[e][j].id;
    }

    std::vector<std::vector<VarSAT>> F;
    int getVarF(unsigned int q, unsigned int j) {
        if(q >= F.size())
            F.resize(q+1);

        if(j >= F[q].size())
            F[q].resize(j+1);

        return F[q][j].id;
    }

    std::vector<std::vector<VarSAT>> CL;
    int getVarCL(unsigned int q, unsigned int cl) {
        if(q >= CL.size())
            CL.resize(q+1);

        if(cl >= CL[q].size())
            CL[q].resize(cl+1);

        return CL[q][cl].id;
    }




    std::vector< VarAux > USED;
    int getVarUsed(unsigned int feuille) {
        if(feuille >= USED.size()) {
            USED.resize(feuille+1);
        }

        return USED[feuille].id;
    }

    std::vector< std::vector<VarSAT> > COMPTER;
    int getVarCompter(unsigned int i, unsigned int j) {
        if(i >= COMPTER.size())
            COMPTER.resize(i+1);

        if(j >= COMPTER[i].size())
            COMPTER[i].resize(j+1);

        return COMPTER[i][j].id;
    }



public:




    Infering(const unsigned int k, unsigned int nbFeature, unsigned int nbClasse)
        : solver(new Minisat::Solver()), k(k), nbFeature(nbFeature), nbClasse(nbClasse) {

        nbClauses=0;
        nombreElement=0;

        VarSAT::solver = solver;
        VarAux::solver = solver;


        for(unsigned int i=1; i<pow2(k); i++) {

            for(unsigned int f1=0; f1<nbFeature; f1++) {
                for(unsigned int f2=f1+1; f2<nbFeature; f2++) {

                    nbClauses++;
                    solver->addClause(
                        Minisat::mkLit( getVarF(i, f1), true),
                        Minisat::mkLit( getVarF(i, f2), true)
                     );
                }
            }
        }


        for(unsigned int i=1; i<pow2(k); i++) {
            Minisat::vec<Minisat::Lit> clause;

            for(unsigned int f1=0; f1<nbFeature; f1++) {
                clause.push(Minisat::mkLit( getVarF(i, f1), false));
            }

            nbClauses++;
            solver->addClause(clause);

        }




    for(unsigned int q=0; q<pow2(k+1); q++)
        for(unsigned int c1=0; c1<nbClasse; c1++)
            getVarCL(q, c1);


        // TIME OUT DE 10 MIN
        //solver->set_timeout_all_calls(10);


    }

    void setToUpdate() {
        toUpdate = true;
    }

    void addContraiteMaxFeuille() {
        for(unsigned int q=0; q<pow2(k+1); q++) {
            for(unsigned int c=0; c<nbClasse; c++) {

                nbClauses++;
                solver->addClause(
                    Minisat::mkLit( getVarCL(q, c), true),
                    Minisat::mkLit( getVarUsed(q), false)
                );
            }
        }

        nbClauses++;
        solver->addClause( Minisat::mkLit( getVarCompter(0, 0), false) );

        for(unsigned int q=0; q<pow2(k+1); q++) {
            for(unsigned int j=0; j<q+1; j++) {
                nbClauses++;
                solver->addClause(
                    Minisat::mkLit( getVarUsed(q), true),
                    Minisat::mkLit( getVarCompter(q, j), true),
                    Minisat::mkLit( getVarCompter(q+1, j+1), false)
                );

                nbClauses++;
                solver->addClause(
                    Minisat::mkLit( getVarCompter(q, j), true),
                    Minisat::mkLit( getVarCompter(q+1, j), false)
                );


            }
        }
    }

    void addContraiteMaxFeuille(int nombreFeuille) {

        toUpdate = true;


        if(nombreFeuille != -1) {
            for(unsigned int q=0; q<pow2(k+1); q++) {
                for(unsigned int c=0; c<nbClasse; c++) {

                    nbClauses++;
                    solver->addClause(
                        Minisat::mkLit( getVarCL(q, c), true),
                        Minisat::mkLit( getVarUsed(q), false)
                    );
                }
            }

            nbClauses++;
            solver->addClause( Minisat::mkLit( getVarCompter(0, 0), false) );

            for(unsigned int q=0; q<pow2(k+1); q++) {
                for(unsigned int j=0; j<q+1; j++) {
                    nbClauses++;
                    solver->addClause(
                        Minisat::mkLit( getVarUsed(q), true),
                        Minisat::mkLit( getVarCompter(q, j), true),
                        Minisat::mkLit( getVarCompter(q+1, j+1), false)
                    );

                    nbClauses++;
                    solver->addClause(
                        Minisat::mkLit( getVarCompter(q, j), true),
                        Minisat::mkLit( getVarCompter(q+1, j), false)
                    );


                }
            }

            nbClauses++;
            solver->addClause( Minisat::mkLit( getVarCompter(pow2(k+1), static_cast<unsigned int>(nombreFeuille+1)), true) );
        }

    }

    ~Infering() {
        delete solver;
    }




    void generateContraintesFeature(const vector<bool> &newElement, unsigned int nb, Minisat::vec<Minisat::Lit> &clause, unsigned int q, unsigned int lvl) {

        if(nb == 0) {
            return;
        }

        unsigned idNewElement = nombreElement;

        clause.push( Minisat::mkLit( getVarX(idNewElement, lvl), false) );

            for(unsigned int feature=0; feature < newElement.size(); feature++) {
                if( newElement[feature] ) {
                    clause.push( Minisat::mkLit( getVarF(q, feature), true) );
                    nbClauses++;
                    solver->addClause(clause);

                    clause.pop();
                }
            }

            generateContraintesFeature(newElement, nb-1, clause, q*2, lvl+1);
        clause.pop();


        clause.push( Minisat::mkLit( getVarX(idNewElement, lvl), true) );

            for(unsigned int feature=0; feature < newElement.size(); feature++) {
                if( !newElement[feature] ) {
                    clause.push( Minisat::mkLit( getVarF(q, feature), true) );
                    nbClauses++;
                    solver->addClause(clause);

                    clause.pop();
                }
            }

            generateContraintesFeature(newElement, nb-1, clause, q*2+1, lvl+1);
        clause.pop();
    }

    void generateContraintesClasse(Minisat::vec<Minisat::Lit> &clause, unsigned int q, unsigned int lvl, unsigned int cl) {

        if(lvl == k) {
            clause.push( Minisat::mkLit( getVarCL(q, cl), false) );
            nbClauses++;
            solver->addClause(clause);

            clause.pop();

            for(unsigned int c=0; c<nbClasse; c++) {
                if(c!=cl) {
                    clause.push( Minisat::mkLit( getVarCL(q, c), true) );
                    nbClauses++;
                    solver->addClause(clause);
                    clause.pop();
                }
            }

            return;
        }

        unsigned int idNewElement = nombreElement;

        clause.push( Minisat::mkLit( getVarX(idNewElement, lvl), false) );

            generateContraintesClasse(clause, q*2, lvl+1, cl);

        clause.pop();


        clause.push( Minisat::mkLit( getVarX(idNewElement, lvl), true) );

            generateContraintesClasse(clause, q*2+1, lvl+1, cl);

        clause.pop();
    }

    void reduceNMAX(unsigned int nombreMaxFeuille) {

        nbClauses++;
        solver->addClause( Minisat::mkLit( getVarCompter(pow2(k+1), nombreMaxFeuille+1), true) );
        toUpdate = true;
    }


    void add(const vector<bool> &newElement, unsigned int label) {
        assert(nbFeature == newElement.size());
        assert(label > 0);

        Minisat::vec<Minisat::Lit> clause;
        generateContraintesFeature(newElement, k, clause, 1, 0);


        assert(clause.size() == 0);

        generateContraintesClasse(clause, 0, 0, label-1);

        nombreElement++;

        toUpdate = true;
    }


    bool inferModel(unsigned int nombreMaxFeuille=0) {

        if(!toUpdate)
            return lastResult;
        toUpdate = false;

        //solver->simplify();

        if(nombreMaxFeuille > 0) {
            //int nombre
            lastResult = solver->solve( Minisat::mkLit( getVarCompter( pow2(k+1), static_cast<unsigned int>(nombreMaxFeuille+1)), true) );
        } else {
            lastResult = solver->solve();
        }

        return lastResult;
    }

    std::vector<int> getModel() {
        inferModel();

        std::vector<int> tree;

        tree.resize(pow2(k+1), std::numeric_limits<int>::min());

        for(unsigned int q=1; q<pow2(k); q++) {
            for(unsigned int f=0; f<nbFeature; f++) {
                if(solver->model[ getVarF(q, f) ] == l_True)
                    tree[q] = static_cast<int>(f);
            }
        }

        for(unsigned int q=0; q<pow2(k); q++) {
            for(unsigned int cl=0; cl<nbClasse; cl++) {
                if(solver->model[ getVarCL(q, cl) ] == l_True) {
                    tree[pow2(k) + q] = -static_cast<int>(cl+1);
                }
            }
        }

        return tree;
    }

    unsigned int getNombreVar() {
        return static_cast<unsigned int>(solver->model.size());
    }

    unsigned int getNombreClause() {

        return static_cast<unsigned int>(solver->nClauses());
    }


};



Minisat::Solver* Infering::VarSAT::solver = nullptr;
Minisat::Solver* Infering::VarAux::solver = nullptr;


tuple<vector<bool>, unsigned int> distinger(const std::vector<int> &tree, const vector< vector< vector<bool> > > &S) {


    for(unsigned int i=0; i<S.size(); i++) {
        for(auto &e: S[i]) {
            unsigned int pos = 1;

            for(;;) {
                assert(pos < tree.size());
                int feature = tree[pos];
                if( feature < 0 ) {

                    if( (-feature-1) != static_cast<int>(i) )
                        return std::make_tuple(e, (i+1));
                    break;
                }

                assert(feature >= 0);
                if( e[static_cast<unsigned int>(feature)] )
                    pos = pos*2+1;
                else
                    pos = pos*2;
            }
        }
    }


    return make_tuple(vector<bool>(), 0);
}


std::vector<int> inferTreeReduceNode(unsigned int k, const vector< vector< vector<bool> > > &S, bool verbose = false) {
    std::vector<int> tree;
    std::vector<int> saveTree;

    unsigned int nbFeature=0;


    for(auto &s: S) {
        if(s.size()) {
            nbFeature = static_cast<unsigned int>(s[0].size());
            break;
        }
    }

    assert(nbFeature > 0);


    Infering infer(k, nbFeature, static_cast<unsigned int>(S.size()));


    while( infer.inferModel() ) {
        tree = infer.getModel();



        std::vector<bool> element;
        unsigned int label;
        std::tie(element, label) = distinger(tree, S);

        if(element.size() == 0) {
            assert(label == 0);


            if(verbose)
                std::cout << "Tree Found!" << std::endl;
            saveTree = tree;
            infer.addContraiteMaxFeuille();

            unsigned int FMin = 1;

            unsigned int FMax = pow2(k);

            unsigned int last_nombreMaxFeuille=0;

            while(FMin != FMax) {

                unsigned int nombreMaxFeuille = (FMax+FMin)/2;

                if(verbose)
                    if(last_nombreMaxFeuille != nombreMaxFeuille)
                        std::cout << "Try with MaxNodes=" << (2*(nombreMaxFeuille)-1) << std::endl;
                last_nombreMaxFeuille = nombreMaxFeuille;

                infer.setToUpdate();

                if( infer.inferModel(nombreMaxFeuille) ) {
                    tree = infer.getModel();

                    std::tie(element, label) = distinger(tree, S);

                    if(element.size() == 0) {
                        FMax = nombreMaxFeuille;
                        saveTree = tree;
                        if(verbose)
                            std::cout << "Solution" << std::endl;
                    } else {
                        infer.add(element, label);
                    }

                } else {
                    FMin = nombreMaxFeuille+1;
                    if(verbose)
                        std::cout << "No solution" << std::endl;
                }

            }

            nbNodes = (2*(FMin)-1);

            if(verbose)
                std::cout << "Best solution: " << nbNodes << " nodes." << std::endl;

            return saveTree;
        }

        infer.add(element, label);
    }



    return {};
}



bool resultClause(const vector<bool> &clause, std::vector<bool> input ) {

    for(unsigned int i=0; i<clause.size(); i++) {
        if( clause[i] ) {
            if( input[ i/2 ] == i%2 ) {
                return false;
            }
        }
    }

    return true;
}



std::vector<int> inferTree(unsigned int k, const vector< vector< vector<bool> > > &S, int nombreMaxState=-1) {

    std::vector<int> tree;

    unsigned int nbFeature=0;


    for(auto &s: S) {
        if(s.size()) {
            nbFeature = static_cast<unsigned int>( s[0].size() );
            break;
        }
    }

    assert(nbFeature > 0);

    Infering infer(k, nbFeature, static_cast<unsigned int>(S.size()));

    if(nombreMaxState != -1)
        infer.addContraiteMaxFeuille( nombreMaxState/2+1 );


    while( infer.inferModel() ) {
        tree = infer.getModel();

        std::vector<bool> element;
        unsigned int label;
        std::tie(element, label) = distinger(tree, S);

        if(element.size() == 0) {
            assert(label == 0);

            return tree;
        }


        infer.add(element, label);
    }

    return {};
}




/**
 * @brief getData : get data using mapping for return binary features
 * @param link
 * @param string2bin
 * @param double2bin
 * @param classes
 * @return data
 */
vector< vector< vector<bool> > > getData(
        const std::string &link,
        const vector< map<string, vector<bool> >  > &string2bin,
        const vector< map<double, vector<bool> >  > &double2bin,
        const map<string, unsigned int> &classes) {
    vector< vector< vector<bool> > >  result;

    CSV csv(link, ';');
    csv.next();

    while(csv.next()) {
        vector<string> tmp =  csv.get();
        if(tmp.size() == 0)
            continue;
        vector<bool> features;

        for(unsigned int i=0; i<static_cast<unsigned int>(tmp.size()-1); i++) {
            vector<bool> vBin;

            if( isDouble(tmp[i]) ) {
                istringstream istr( tmp[i] );
                double d;
                istr >> d;
                vBin = double2bin.at(i).at(d);
            } else {
                vBin = string2bin.at(i).at(tmp[i]);
            }

            features.insert(features.end(), vBin.begin(), vBin.end());
        }

        if( classes.at(tmp.back()) >= result.size() ) {
            result.resize( classes.at(tmp.back())+1 );
        }

        result[ classes.at(tmp.back()) ].push_back( features );
    }

    assert(result.size() == classes.size());

    return result;
}

/**
 * @brief updateSetOfType : update the set of different type for each feature
 * @param link
 * @param elementString
 * @param elementDouble
 * @param classes
 * @return Number of elements scanned
 */
unsigned int updateSetOfType(const string &link, vector< set<string> > &elementString, vector< set<double> > &elementDouble, map<string, unsigned int> &classes) {
    unsigned int nbElements=0;
    unsigned int line=1;
    CSV csv(link, ';');
    csv.next();

    if(csv.size() == 0) {
        std::cerr << "CSV format error." << std::endl;
        exit(-1);
    }

    unsigned int nbFeature = csv.size()-1;
    elementDouble.resize( nbFeature );
    elementString.resize( nbFeature );

    while(csv.next()) {
        line++;
        vector<string> tmp = csv.get();

        if(tmp.size() == 0)
            continue;   // Empty line

        if(tmp.size() != nbFeature + 1) {
            std::cerr << "CSV format error: " << (nbFeature+1) << " columns inspected in "<<link<<" line " << line << "." << std::endl;
            exit(-1);
        }

        for(unsigned int i=0; i<nbFeature; i++) {

            if( isDouble(tmp[i]) ) {
                istringstream istr( tmp[i] );
                double d;
                istr >> d;
                elementDouble[i].insert( d );
            } else {
                elementString[i].insert( tmp[i] );
            }
        }
        nbElements++;

        if( classes.count(tmp.back()) == 0 ) {
            classes[tmp.back()] = static_cast<unsigned int>(classes.size());
        }
    }

    return nbElements;
}

/**
 * @brief convertTypeToBin
 * @param elementString
 * @param elementDouble
 * @return mappings to binary features
 */
std::tuple< vector< map<string, vector<bool> >  >, vector< map<double, vector<bool> > > > convertTypeToBin(const vector< set<string> > &elementString, const vector< set<double> > &elementDouble) {

    vector< map<string, vector<bool> > > string2bin;
    vector< map<double, vector<bool> > > double2bin;

    unsigned int nbFeature = static_cast<unsigned int>(elementString.size());

    assert(elementDouble.size() == elementString.size());

    string2bin.resize(nbFeature);
    double2bin.resize(nbFeature);

    for(unsigned int i=0; i<nbFeature; i++) {
        if(elementString[i].size()) {
            if( elementDouble[i].size() != 0) {
                std::cerr << "CSV format error: feature " << (i+1) << " has mixed types." << std::endl;
                exit(-1);
            }

            //nbBinaryFeature += elementString[i].size();
            unsigned int num=0;
            for(auto e: elementString[i]) {
                vector<bool> tmp;
                tmp.resize(elementString[i].size(), false);
                tmp[num] = true;
                string2bin[i][e] = tmp;
                num++;
            }

        } else {
            assert(elementDouble[i].size());

            //nbBinaryFeature += elementDouble[i].size();
            vector<bool> tmp;
            tmp.resize(elementDouble[i].size(), false);
            unsigned int num=0;
            for(auto e: elementDouble[i]) {
                tmp[num] = true;
                double2bin[i][e] = tmp;
                num++;
            }
        }
    }


    return make_tuple(string2bin, double2bin);
}

/**
 * @brief LoadAsBinaryFeature : Read the CSV file and convert features to binary features
 * @param link : Link to a CSV file
 * @return data with binary features
 */
vector< vector< vector<bool> > > LoadAsBinaryFeature(string link) {

    unsigned int nombreElementTotal = 0;
    unsigned int nbBinaryFeature=0;



    vector< map<string, vector<bool> >  > string2bin;
    vector< map<double, vector<bool> >  > double2bin;


    map<string, unsigned int> classes;
    {
        vector< set<double> > elementDouble;
        vector< set<string> > elementString;
        nombreElementTotal = updateSetOfType(link, elementString, elementDouble, classes);

        std::tie(string2bin, double2bin) = convertTypeToBin(elementString, elementDouble);
    }

    vector< vector< vector<bool> > > result = getData(link, string2bin, double2bin, classes);

    assert(string2bin.size() == double2bin.size());
    for(unsigned int i=0; i<string2bin.size(); i++) {
        if(string2bin[i].size()) {
            assert(double2bin[i].size() == 0);
            nbBinaryFeature += (string2bin[i].begin())->second.size();
        } else {
            assert(double2bin[i].size());
            nbBinaryFeature += (double2bin[i].begin())->second.size();
        }
    }

    cout << "Number of examples = " << nombreElementTotal << endl;
    cout << "Number of binary features = " << nbBinaryFeature << endl;
    cout << "Number of classes = " << result.size() << endl;

    return result;

}



/**
 * @brief FinalLoadAsBinaryFeatureCross
 * @param link
 * @param nombre
 * @return
 */
vector< vector< vector< vector<bool> > > > FinalLoadAsBinaryFeatureCross(string link, unsigned int nombre){

    //unsigned int nombreElementTotal=0;
    vector< map<string, vector<bool> >  > string2bin;
    vector< map<double, vector<bool> >  > double2bin;


    map<string, unsigned int> classes;
    {
        vector< set<double> > elementDouble;
        vector< set<string> > elementString;
        //nombreElementTotal = updateSetOfType(link, elementString, elementDouble, classes);
        updateSetOfType(link, elementString, elementDouble, classes);

        std::tie(string2bin, double2bin) = convertTypeToBin(elementString, elementDouble);
    }

    vector< vector< vector<bool> > > all = getData(link, string2bin, double2bin, classes);
    std::vector< std::tuple< unsigned int,  vector<bool> > > all2;
    for(unsigned int c=0; c<all.size(); c++) {
        for(auto features: all[c]) {
            all2.push_back( make_tuple(c, features) );
        }
    }
    std::random_shuffle(all2.begin(), all2.end());

    vector< vector< vector< vector<bool> > > > result;
    result.resize(nombre);
    for(unsigned int i=0; i<nombre; i++) {
        result[i].resize( all.size() );
    }
    for(unsigned int i=0; i<all2.size(); i++) {
        result[i%nombre][ std::get<0>(all2[i]) ].push_back( std::get<1>(all2[i]) );
    }

    return result;
}



/**
 * @brief LoadAsBinaryFeature : Read the CSV files and convert features to binary features
 * @return training set and test set with binary features
 */
std::tuple< vector< vector< vector<bool> > >, vector< vector< vector<bool> > > > LoadAsBinaryFeature(string link_train, string link_test) {

    vector< set<string> > elementString;
    vector< set<double> > elementDouble;
    map<string, unsigned int> classes;
    unsigned int nombreElementTotal = updateSetOfType(link_train, elementString, elementDouble, classes);
    updateSetOfType(link_test, elementString, elementDouble, classes);

    assert(elementDouble.size() == elementString.size());

    vector< map<string, vector<bool> >  > string2bin;
    vector< map<double, vector<bool> >  > double2bin;
    std::tie(string2bin, double2bin) = convertTypeToBin(elementString, elementDouble);

    std::tuple< vector< vector< vector<bool> > >, vector< vector< vector<bool> > > > result = {
        getData(link_train, string2bin, double2bin, classes),
        getData(link_test, string2bin, double2bin, classes)
    };

    unsigned int nbBinaryFeature=0;
    assert(string2bin.size() == double2bin.size());
    for(unsigned int i=0; i<string2bin.size(); i++) {
        if(string2bin[i].size()) {
            assert(double2bin[i].size() == 0);
            nbBinaryFeature += (string2bin[i].begin())->second.size();
        } else {
            assert(double2bin[i].size());
            nbBinaryFeature += (double2bin[i].begin())->second.size();
        }
    }

    cout << "Number of training examples = " << nombreElementTotal << endl;
    cout << "Number of binary features = " << nbBinaryFeature << endl;
    cout << "Number of classes = " << std::get<0>(result).size() << endl;

    return result;
}





double score(std::vector< vector<bool> > &DNF, vector< vector< vector<bool> > > &S) {

    int nb_ok=0;
    int nb_pasok=0;

    assert(S.size() == 2);


    for(auto &e: S[1]) {
        bool ok=false;
        for(auto &clause: DNF) {
            if(resultClause(clause, e) == 1) {
                ok=true;
                break;
            }
        }
        if(!ok) {
            nb_pasok++;
        } else {
            nb_ok++;
        }
    }

    for(auto &e: S[0]) {
        bool ok=false;
        for(auto &clause: DNF) {
            if(resultClause(clause, e) == 1) {
                ok=true;
                break;
            }
        }
        if(!ok) {
            nb_ok++;
        } else {
            nb_pasok++;
        }
    }


    return static_cast<double>(nb_ok)/(static_cast<double>(nb_ok) + static_cast<double>(nb_pasok));
}

double score(std::vector<int> &tree, vector< vector< vector<bool> > > &S) {
    int ok=0;
    int pasok=0;

    for(unsigned int classe=0; classe<S.size(); classe++) {

        for(auto &e: S[classe]) {


            unsigned int pos = 1;

            while( tree[pos] >= 0) {

                assert( tree[pos] >= 0);
                unsigned int feature = static_cast<unsigned int>( tree[pos] );

                if( e[feature] ) {
                    pos = pos * 2 + 1;
                } else {
                    pos = pos * 2;
                }

            }

            assert( -tree[pos]-1 >= 0 );
            if( static_cast<unsigned int>(-tree[pos]-1) != classe ) {
                pasok++;
            } else {
                ok++;
            }
        }

    }

    return static_cast<double>(ok)/(static_cast<double>(ok) + static_cast<double>(pasok));
}

std::vector<int> findBestDT(vector< vector< vector<bool> > >& S, bool verbose=false) {
    std::vector<int> tree;
    kFind=1;
    for(;;kFind++) {
        if(verbose)
            std::cout << "Try with k=" << kFind << std::endl;
        tree = inferTreeReduceNode(kFind, S, verbose);
        if(tree.size())
            break;
    }

    return tree;
}

vector< vector< vector<bool> > > extractTestSet (vector< vector< vector<bool> > >& S, double pourcentage) {

   vector< vector< vector<bool> > > Test;

   Test.resize(S.size());

   unsigned int nb = 0;

   for(auto &c: S) {
       nb += c.size();
   }

   nb = static_cast<unsigned int>(  (nb * pourcentage) / 100.0 );

   while(nb) {

       unsigned int classe = static_cast<unsigned int>(rand()) % S.size();

       if( S[classe].size() ) {

           unsigned int e = static_cast<unsigned int>(rand()) % S[classe].size();

           Test[classe].push_back( S[classe][e] );

           S[classe][e] = S[classe].back();
           S[classe].pop_back();
           nb--;
       }
   }

   return Test;
}





int main(int argc, char *argv[]) {

    CLI::App app("Inferring Optimal Decision Tree");


    string file;
    app.add_option("CSV_file", file, "CSV file")->check(CLI::ExistingFile)->required();

    bool minimizeOnlyDepth=false;
    app.add_flag("-d,--depth", minimizeOnlyDepth, "Minimize only the depth of the tree");


    string testset="";
    auto inferCmd = app.add_subcommand("infer", "infer an optimal decision tree (\"xdot\" required for print the result)")->callback([&file, &minimizeOnlyDepth, &testset](){

        vector< vector< vector<bool> > > S;
        vector< vector< vector<bool> > > T;
        if(testset.size()) {
            std::tie(S, T) = LoadAsBinaryFeature(file, testset);
        } else {
            S = LoadAsBinaryFeature(file);
        }


        std::vector<int> tree;
        {
            MaLib::Chrono C("Inferring time");
            if(minimizeOnlyDepth) {
                for(unsigned int k=1;;k++) {
                   std::cout << "Try with k="<<k<<std::endl;
                   tree = inferTree(k, S);
                   kFind=k;
                   if(tree.size()) {
                       std::cout << "Tree Found!" << std::endl;
                       break;
                   }
               }
            } else {
                tree = findBestDT(S, true);
            }

            std::cout << endl << endl << endl << "===== RESULT =====" << endl;
            std::cout << "Depth: " << kFind << std::endl;
            if(!minimizeOnlyDepth)
                std::cout << "Number of nodes: " << nbNodes << std::endl;
            std::cout << "Number of examples considered: " << nombreElement << std::endl;
            if(testset.size()) {
                std::cout << "Accuracy: " << 100.0*score(tree, T) << " %" << std::endl;
            }

        }

        simplifyTree(tree);
        xdot::show( tree2dot(tree) );
        assert(verifier(tree, S));  // Vérifier que l'arbre est consistant avec S

        exit(0);
    });

    inferCmd->add_option("-t", testset, "Set a test dataset")->check(CLI::ExistingFile);


    unsigned int nbfold=10;
    auto benchCmd = app.add_subcommand("bench", "Benchmark with cross-validation")->callback([&file, &nbfold, &minimizeOnlyDepth](){

        Moyenne M_times("Inferting Time");
        Moyenne M_score("Accuracy");
        Moyenne M_nombreElement("Examples Considered");
        Moyenne M_kFind("Depth");
        Moyenne M_nbNodes("Nodes");

        Chrono TotalTime("Total Time");

        for(int i=0;;i++) {

            auto Cross = FinalLoadAsBinaryFeatureCross(file, nbfold);

            for(unsigned int sp=0; sp<nbfold; sp++) {

               vector< vector< vector<bool> > > S;
               S.resize( Cross[0].size() );
               vector< vector< vector<bool> > > T = Cross[sp];

               for(unsigned int x=0; x<nbfold; x++) {
                   if(x==sp)
                       continue;

                   for(unsigned int c=0; c<Cross[x].size(); c++) {
                       S[c].insert( S[c].end(), Cross[x][c].begin(), Cross[x][c].end() );
                   }
               }

               std::vector<int> tree;
               auto debut = TotalTime.tac();

               if(minimizeOnlyDepth) {
                   for(unsigned int k=1;;k++) {
                      tree = inferTree(k, S);
                      kFind=k;
                      if(tree.size())
                          break;
                  }
               } else {
                   tree = findBestDT(S);
               }

               M_times.add( (TotalTime.tac()-debut)/1000.0 );

               assert(verifier(tree, S));  // Vérifier que l'arbre est consistant avec S

               M_score.add(score(tree, T)*100.0);



               M_nombreElement.add(nombreElement);
               M_kFind.add(kFind);
               M_nbNodes.add(nbNodes);
            }


            std::cout << endl << endl << "===== RESULT AFTER "<<(i+1)<<" "<< nbfold << "-fold cross-validation  =====" << endl;

            M_score.print("%");
            M_nbNodes.print();
            M_kFind.print();
            M_nombreElement.print();
            M_times.print("ms");
        }

        assert(false);
    });

    benchCmd->add_option("-f,--fold", nbfold, "Number of fold for the cross-validation (default = 10)");


    CLI11_PARSE(app, argc, argv);


    std::cout << "Select an SUBCOMMAND (infer or bench)." << std::endl;
    std::cout << "Example : " << argv[0] << " data/mouse.csv infer" << std::endl;



    return 0;
}







