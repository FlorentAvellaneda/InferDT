#ifndef INCREMENTALLEARNER_S0158D0FE2M_H
#define INCREMENTALLEARNER_S0158D0FE2M_H

#include <vector>
#include <optional>
#include "tabular_data.h"

template<class INFER_ALGO>
class IncrementalLearner {
    BinarizedTabularData *_data;
public:

    IncrementalLearner(BinarizedTabularData *data)
        : _data(data) {

    }

    std::vector<int> findOptimalTree(unsigned int minNumberForK, bool doNodeMinimization = false, bool verbose = false) {
        std::optional<std::vector<int>> tree;
        for (;; minNumberForK++) {
            if (verbose)
                std::cout << "Try with k=" << minNumberForK << std::endl;
            tree = inferTree(minNumberForK, doNodeMinimization, verbose);
            if(tree)
                break;
        }

        return tree.value();
    }

    std::vector<int> minimizeNode(INFER_ALGO& infer, bool verbose=false) {
        auto saveTree = infer.getModel();
        infer.addConstraints_MaxLeaves();

        unsigned int FMin = 1;
        unsigned int FMax = pow2( infer.getDepth() );
        unsigned int last_nombreMaxFeuille = 0;
        while (FMin != FMax) {
            unsigned int nombreMaxFeuille = (FMax + FMin) / 2;
            if (verbose) {
                if (last_nombreMaxFeuille != nombreMaxFeuille) {
                    std::cout << "Try with MaxNodes=" << (2 * (nombreMaxFeuille) - 1) << std::endl;
                }
            }
            last_nombreMaxFeuille = nombreMaxFeuille;

            infer.setToUpdate();

            if (infer.inferModel(nombreMaxFeuille)) {
                auto tree = infer.getModel();
                auto [element, label] = distinguish(tree);
                if (element.size() == 0) {
                    FMax = nombreMaxFeuille;
                    saveTree = tree;
                    if (verbose) std::cout << "Solution" << std::endl;
                } else {
                    infer.add(element, label);
                }
            } else {
                FMin = nombreMaxFeuille + 1;
                if (verbose) std::cout << "No solution" << std::endl;
            }
        }

        return saveTree;
    }


    std::optional<std::vector<int>> inferTree(unsigned int k, bool doNodeMinimization = false, bool verbose = false) {
        unsigned int nbFeature = _data->numFeatures();
        assert(nbFeature > 0);

        INFER_ALGO infer(k, nbFeature, _data->numClasses());

        while (infer.inferModel()) {
            auto tree = infer.getModel();
            auto [element, label] = distinguish(tree);

            if (element.size() == 0) {
                assert(label == 0);

                if(doNodeMinimization)
                    tree = minimizeNode(infer);

                return std::make_optional(tree);
            }

            infer.add(element, label);
        }

        return {};
    }


private:

    std::tuple<std::vector<bool>, unsigned int> distinguish(const std::vector<int> &tree) {
        for (unsigned int i = 0; i < _data->numClasses(); i++) {
            for (auto &e: _data->getData()[i]) {
                unsigned int pos = 1;
                for (;;) {
                    assert(pos < tree.size());
                    int feature = tree[pos];
                    if (feature < 0) {

                        if ((-feature - 1) != static_cast<int>(i))
                            return std::make_tuple(e, (i + 1));
                        break;
                    }

                    assert(feature >= 0);
                    if (e[static_cast<unsigned int>(feature)])
                        pos = pos * 2 + 1;
                    else
                        pos = pos * 2;
                }
            }
        }

        return std::make_tuple(std::vector<bool>(), 0);
    }

};



#endif // INCREMENTALLEARNER_S0158D0FE2M_H
