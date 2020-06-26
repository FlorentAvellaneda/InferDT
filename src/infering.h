#ifndef INFERING_10QPORR9US_H
#define INFERING_10QPORR9US_H

#include <minisat/core/Solver.h>
#include <vector>
#include "utile.h"


using namespace Minisat;

class Infering {
    Minisat::Solver *solver;

    class VarSAT {
        static Minisat::Solver *solver;
    public:
        int id;

        VarSAT() {
            id = solver->newVar();
        }

        static void init(Minisat::Solver *solver) {
            VarSAT::solver = solver;
        }

        static int newVar() {
            return solver->newVar();
        }

        friend class Infering;
    };

    class VarAux {
        static Minisat::Solver *solver;
    public:
        int id;

        VarAux() {
            id = solver->newVar(l_Undef, false);
        }

        static void init(Minisat::Solver *solver) {
            VarAux::solver = solver;
        }

        static int newVar() {
            return solver->newVar(l_Undef, false);
        }

        friend class Infering;
    };

    bool toUpdate = true;
    bool lastResult = false;
    unsigned int k;
    unsigned int nbFeature;
    unsigned int nbClasse;
    unsigned int nombreElement=0;

    std::vector<std::vector<VarSAT>> X;
    int getVarX(unsigned int e, unsigned int j) {
        if (e >= X.size())
            X.resize(e + 1);

        if (j >= X[e].size())
            X[e].resize(j + 1);

        return X[e][j].id;
    }

    std::vector<std::vector<VarSAT>> F;
    int getVarF(unsigned int q, unsigned int j) {
        if (q >= F.size())
            F.resize(q + 1);

        if (j >= F[q].size())
            F[q].resize(j + 1);

        return F[q][j].id;
    }

    std::vector<std::vector<VarSAT>> CL;
    int getVarCL(unsigned int q, unsigned int cl) {
        if (q >= CL.size())
            CL.resize(q + 1);

        if (cl >= CL[q].size())
            CL[q].resize(cl + 1);

        return CL[q][cl].id;
    }


    std::vector<VarAux> USED;
    int getVarUsed(unsigned int feuille) {
        if (feuille >= USED.size()) {
            USED.resize(feuille + 1);
        }

        return USED[feuille].id;
    }

    std::vector<std::vector<VarSAT> > COMPTER;
    int getVarCompter(unsigned int i, unsigned int j) {
        if (i >= COMPTER.size())
            COMPTER.resize(i + 1);

        if (j >= COMPTER[i].size())
            COMPTER[i].resize(j + 1);

        return COMPTER[i][j].id;
    }


public:

    Infering(const unsigned int k, unsigned int nbFeature, unsigned int nbClasse)
            : solver(new Minisat::Solver()), k(k), nbFeature(nbFeature), nbClasse(nbClasse) {

        VarSAT::solver = solver;
        VarAux::solver = solver;

        for (unsigned int i = 1; i < pow2(k); i++) {
            for (unsigned int f1 = 0; f1 < nbFeature; f1++) {
                for (unsigned int f2 = f1 + 1; f2 < nbFeature; f2++) {
                    solver->addClause(
                            Minisat::mkLit(getVarF(i, f1), true),
                            Minisat::mkLit(getVarF(i, f2), true)
                    );
                }
            }
        }

        for (unsigned int i = 1; i < pow2(k); i++) {
            Minisat::vec<Minisat::Lit> clause;
            for (unsigned int f1 = 0; f1 < nbFeature; f1++) {
                clause.push(Minisat::mkLit(getVarF(i, f1), false));
            }
            solver->addClause(clause);
        }

        for (unsigned int q = 0; q < pow2(k + 1); q++)
            for (unsigned int c1 = 0; c1 < nbClasse; c1++)
                getVarCL(q, c1);

        // TIME OUT DE 10 MIN
        //solver->set_timeout_all_calls(10);
    }

    void setToUpdate() {
        toUpdate = true;
    }

    void addConstraints_MaxLeaves() {
        for (unsigned int q = 0; q < pow2(k + 1); q++) {
            for (unsigned int c = 0; c < nbClasse; c++) {
                solver->addClause(
                        Minisat::mkLit(getVarCL(q, c), true),
                        Minisat::mkLit(getVarUsed(q), false)
                );
            }
        }

        solver->addClause(Minisat::mkLit(getVarCompter(0, 0), false));

        for (unsigned int q = 0; q < pow2(k + 1); q++) {
            for (unsigned int j = 0; j < q + 1; j++) {
                solver->addClause(
                        Minisat::mkLit(getVarUsed(q), true),
                        Minisat::mkLit(getVarCompter(q, j), true),
                        Minisat::mkLit(getVarCompter(q + 1, j + 1), false)
                );

                solver->addClause(
                        Minisat::mkLit(getVarCompter(q, j), true),
                        Minisat::mkLit(getVarCompter(q + 1, j), false)
                );
            }
        }
    }

    void addConstraints_MaxLeaves(int numberMaxOfLeaves) {
        toUpdate = true;
        if (numberMaxOfLeaves != -1) {
            for (unsigned int q = 0; q < pow2(k + 1); q++) {
                for (unsigned int c = 0; c < nbClasse; c++) {
                    solver->addClause(
                            Minisat::mkLit(getVarCL(q, c), true),
                            Minisat::mkLit(getVarUsed(q), false)
                    );
                }
            }

            solver->addClause(Minisat::mkLit(getVarCompter(0, 0), false));

            for (unsigned int q = 0; q < pow2(k + 1); q++) {
                for (unsigned int j = 0; j < q + 1; j++) {
                    solver->addClause(
                            Minisat::mkLit(getVarUsed(q), true),
                            Minisat::mkLit(getVarCompter(q, j), true),
                            Minisat::mkLit(getVarCompter(q + 1, j + 1), false)
                    );

                    solver->addClause(
                            Minisat::mkLit(getVarCompter(q, j), true),
                            Minisat::mkLit(getVarCompter(q + 1, j), false)
                    );
                }
            }

            solver->addClause(Minisat::mkLit(getVarCompter(pow2(k + 1), static_cast<unsigned int>(numberMaxOfLeaves + 1)), true));
        }

    }

    ~Infering() {
        delete solver;
    }

    void addConstraints_Feature(const std::vector<bool> &newElement, unsigned int nb, Minisat::vec<Minisat::Lit> &clause, unsigned int q, unsigned int lvl) {

        if (nb == 0) {
            return;
        }

        unsigned int idNewElement = nombreElement;

        clause.push(Minisat::mkLit(getVarX(idNewElement, lvl), false));

        for (unsigned int feature = 0; feature < newElement.size(); feature++) {
            if (newElement[feature]) {
                clause.push(Minisat::mkLit(getVarF(q, feature), true));
                solver->addClause(clause);

                clause.pop();
            }
        }

        addConstraints_Feature(newElement, nb - 1, clause, q * 2, lvl + 1);
        clause.pop();

        clause.push(Minisat::mkLit(getVarX(idNewElement, lvl), true));

        for (unsigned int feature = 0; feature < newElement.size(); feature++) {
            if (!newElement[feature]) {
                clause.push(Minisat::mkLit(getVarF(q, feature), true));
                solver->addClause(clause);

                clause.pop();
            }
        }

        addConstraints_Feature(newElement, nb - 1, clause, q * 2 + 1, lvl + 1);
        clause.pop();
    }

    void addConstraints_Class(Minisat::vec<Minisat::Lit> &clause, unsigned int q, unsigned int lvl, unsigned int cl) {

        if (lvl == k) {
            clause.push(Minisat::mkLit(getVarCL(q, cl), false));
            solver->addClause(clause);
            clause.pop();
            for (unsigned int c = 0; c < nbClasse; c++) {
                if (c != cl) {
                    clause.push(Minisat::mkLit(getVarCL(q, c), true));
                    solver->addClause(clause);
                    clause.pop();
                }
            }
            return;
        }

        unsigned int idNewElement = nombreElement;

        clause.push(Minisat::mkLit(getVarX(idNewElement, lvl), false));
        addConstraints_Class(clause, q * 2, lvl + 1, cl);
        clause.pop();

        clause.push(Minisat::mkLit(getVarX(idNewElement, lvl), true));
        addConstraints_Class(clause, q * 2 + 1, lvl + 1, cl);
        clause.pop();
    }

    void add(const std::vector<bool> &newElement, unsigned int label) {
        assert(nbFeature == newElement.size());
        assert(label > 0);

        Minisat::vec<Minisat::Lit> clause;
        addConstraints_Feature(newElement, k, clause, 1, 0);

        assert(clause.size() == 0);

        addConstraints_Class(clause, 0, 0, label - 1);

        nombreElement++;

        toUpdate = true;
    }


    bool inferModel(unsigned int maxNumberOfLeaves = 0) {

        if (!toUpdate)
            return lastResult;
        toUpdate = false;

        //solver->simplify();

        if (maxNumberOfLeaves > 0) {
            //int nombre
            lastResult = solver->solve(
                    Minisat::mkLit(getVarCompter(pow2(k + 1), static_cast<unsigned int>(maxNumberOfLeaves + 1)), true));
        } else {
            lastResult = solver->solve();
        }

        return lastResult;
    }

    std::vector<int> getModel() {
        inferModel();

        std::vector<int> tree;

        tree.resize(pow2(k + 1), std::numeric_limits<int>::min());

        for (unsigned int q = 1; q < pow2(k); q++) {
            for (unsigned int f = 0; f < nbFeature; f++) {
                if (solver->model[getVarF(q, f)] == l_True)
                    tree[q] = static_cast<int>(f);
            }
        }

        for (unsigned int q = 0; q < pow2(k); q++) {
            for (unsigned int cl = 0; cl < nbClasse; cl++) {
                if (solver->model[getVarCL(q, cl)] == l_True) {
                    tree[pow2(k) + q] = -static_cast<int>(cl + 1);
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

    unsigned int getDepth() const {
        return k;
    }
};

Minisat::Solver *Infering::VarSAT::solver = nullptr;
Minisat::Solver *Infering::VarAux::solver = nullptr;



#endif // INFERING_10QPORR9US_H
