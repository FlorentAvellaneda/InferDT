#include <iostream>
#include <random>
#include <vector>
#include <set>
#include <map>
#include <tuple>
#include <cassert>
#include <deque>

#include "xdot.h"
#include "Chrono.h"
#include "CSV.h"
#include "Moy.h"
#include "CLI11.hpp"

#include "tabular_data.h"
#include "tree.h"
#include "infering.h"
#include "incrementallearner.h"

using namespace MaLib;
using namespace std;

bool verify(std::vector<int> &tree, const vector<vector<vector<bool> > > &S) {
    for (unsigned int idClass = 0; idClass < S.size(); idClass++) {
        for (auto &e: S[idClass]) {
            unsigned int pos = 1;
            while (tree[pos] >= 0) {
                assert(tree[pos] >= 0);
                unsigned int feature = static_cast<unsigned int>( tree[pos] );

                if (e[feature]) {
                    pos = pos * 2 + 1;
                } else {
                    pos = pos * 2;
                }
            }

            assert(-tree[pos] - 1 >= 0);
            if (static_cast<unsigned int>( -tree[pos] - 1 ) != idClass) {
                return false;
            }
        }
    }

    return true;
}

double score(const std::vector<int> &tree, const vector<vector<vector<bool> > > &S) {
    int ok = 0;
    int pasok = 0;

    for (unsigned int classe = 0; classe < S.size(); classe++) {
        for (auto &e: S[classe]) {
            unsigned int pos = 1;
            while (tree[pos] >= 0) {
                assert(tree[pos] >= 0);
                unsigned int feature = static_cast<unsigned int>( tree[pos] );
                if (e[feature]) {
                    pos = pos * 2 + 1;
                } else {
                    pos = pos * 2;
                }
            }

            assert(-tree[pos] - 1 >= 0);
            if (static_cast<unsigned int>(-tree[pos] - 1) != classe) {
                pasok++;
            } else {
                ok++;
            }
        }
    }
    return static_cast<double>(ok) / (static_cast<double>(ok) + static_cast<double>(pasok));
}


int main(int argc, char *argv[]) {
    CLI::App app("Inferring Optimal Decision Tree");

    string trainingData;
    app.add_option("CSV_file", trainingData, "CSV file")->check(CLI::ExistingFile)->required();

    bool minimizeOnlyDepth = false;
    app.add_flag("-d,--depth", minimizeOnlyDepth, "Minimize only the depth of the tree");

    unsigned int minNumberForK=1;
    app.add_option("-k", minNumberForK, "The minimum depth of the tree to infer");

    bool addIDAsFeatures=false;
    app.add_flag("--id", addIDAsFeatures, "Add ID as features");

    bool useText=false;
    app.add_flag("-x", useText, "Using text instead of xdot");

    bool useCOR=false;
    app.add_flag("--COR", useCOR, "Using Conditional Operator Representation (COR) instead of xdot");

    bool verbose=false;
    app.add_flag("-v", verbose, "verbose");

    bool useEquality=false;
    app.add_flag("--eq", useEquality, "Using equality rather than inequality");
    

    std::optional<string> testingData;
    auto inferCmd = app.add_subcommand("infer", "infer an optimal decision tree (\"xdot\" required for print the result)")->callback( [&]() {

        TabularData data(trainingData, testingData);
        auto binData = data.getBinarizedTraining(useEquality);
        if(addIDAsFeatures)
            binData.addIDAsFeatures();
        IncrementalLearner<Infering> IL( &binData );

        auto tree = IL.findOptimalTree(minNumberForK, !minimizeOnlyDepth, verbose);

        if(verbose) {
            std::cout << endl << endl << endl << "===== RESULT =====" << endl;
            std::cout << "Depth: " << log2(tree.size())-1 << std::endl;
            std::cout << "Number of nodes: " << numberNodes(tree) << std::endl;
        }

        if(testingData) {
            auto testData = data.getBinarizedTesting(useEquality);
            std::cout << "Accuracy: " << 100.0 * score(tree, testData.getData()) << " %" << std::endl;
        }

        simplifyTree(tree);
        if(useText) {
            std::cout << tree2txt(tree, binData) << std::endl;
        } else if(useCOR){
            std::cout << tree2COR(tree, binData) << std::endl;
        } else {
            xdot::show( tree2dot(tree, binData) );
        }
        assert(verify(tree, binData.getData()));  // Vérifier que l'arbre est consistant avec binData

        exit(0);
    });

    inferCmd->add_option("-t", testingData, "Set a test dataset")->check(CLI::ExistingFile);

    unsigned int nbfold = 10;

    auto benchCmd = app.add_subcommand("bench", "Benchmark with cross-validation")->callback( [&]() {

        Moyenne M_times("Inferting Time");
        Moyenne M_score("Accuracy");
        Moyenne M_nombreElement("Examples Considered");
        Moyenne M_kFind("Depth");
        Moyenne M_nbNodes("Nodes");

        Chrono TotalTime("Total Time");

        TabularData data(trainingData);
        auto binData = data.getBinarizedTraining(useEquality);
        if(addIDAsFeatures)
            binData.addIDAsFeatures();

        for (int i = 0;; i++) {
            auto partitionOfBinData = binData.partition(nbfold, i);

            for (unsigned int sp = 0; sp < nbfold; sp++) {
                if(verbose) std::cout << "fold " << (sp+1) << std::endl;

                unsigned int first=0;
                if(sp==0)
                    first=1;
                auto subBinData = partitionOfBinData[first];

                vector<vector<vector<bool> > > T = partitionOfBinData[sp].getData();

                for (unsigned int x = first+1; x < nbfold; x++) {
                    if (x == sp)
                        continue;

                    subBinData.add(partitionOfBinData[x]);
                }

                auto debut = TotalTime.tac();
                IncrementalLearner<Infering> IL( &subBinData );
                auto tree = IL.findOptimalTree(minNumberForK, !minimizeOnlyDepth, verbose);

                M_times.add((TotalTime.tac() - debut) / 1000.0);

                assert(verify(tree, subBinData.getData()));

                M_score.add(score(tree, T) * 100.0);

                M_kFind.add(log2(tree.size())-1);
                M_nbNodes.add(numberNodes(tree));
            }

            std::cout << endl << endl << "===== RESULT AFTER " << (i + 1) << " " << nbfold << "-fold cross-validation  =====" << endl;

            M_score.print("%");
            M_nbNodes.print();
            M_kFind.print();
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




