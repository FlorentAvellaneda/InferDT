#ifndef TREE_KF39FSF22_H
#define TREE_KF39FSF22_H

#include <vector>
#include <deque>
#include <string>
#include <tuple>

#include "tabular_data.h"
#include "xdot.h"

void replace(std::vector<int> &tree, unsigned long a, unsigned long b) {
    std::deque<std::tuple<unsigned long, unsigned long> > toReplace = {std::make_tuple(a, b)};

    while (toReplace.size()) {
        std::tie(a, b) = toReplace.front();
        toReplace.pop_front();

        if (b < tree.size()) {
            tree[a] = tree[b];
            toReplace.push_back(std::make_tuple(a * 2, b * 2));
            toReplace.push_back(std::make_tuple(a * 2 + 1, b * 2 + 1));
        } else {
            tree[a] = std::numeric_limits<int>::min() + 1;
        }
    }
}

void simplifyTree(std::vector<int> &tree) {
    for (unsigned long i = tree.size() - 1; i > 0; i--) {
        if (tree[i] == std::numeric_limits<int>::min()) {
            unsigned long brother;
            if (i % 2) {
                brother = i - 1;
            } else {
                brother = i + 1;
            }
            replace(tree, i / 2, brother);
        }
    }
}

std::string tree2COR(std::vector<int> tree, const BinarizedTabularData &binData, int pos=1, int k=0) {
    std::ostringstream oss;

    if(tree[pos] < 0) { // Cas ou l'arbre n'est qu'une feuille
        if(tree[pos] != std::numeric_limits<int>::min()+1) {
            oss << (-tree[pos]-1) << std::endl;
        } else {
            oss << "null";
        }
        return oss.str();
    }

    oss << "(" << binData.feature2string(tree[pos]) << "?";

    if( tree[pos*2+1] < 0 ) {
        if(tree[pos*2+1] != std::numeric_limits<int>::min()+1)
            oss << binData.getClassName(-tree[pos*2+1]-1);
        else
            oss << "0";
    } else {
        oss << tree2COR(tree, binData, pos*2+1, k+1);
    }

    oss << ":";

    if( tree[pos*2] < 0 ) {
        if(tree[pos*2] != std::numeric_limits<int>::min()+1)
            oss << binData.getClassName(-tree[pos*2]-1);
        else
            oss << "0";
    } else {
        oss << tree2COR(tree, binData, pos*2, k+1);
    }

    oss << ")";

    return oss.str();
}


std::string tree2txt(std::vector<int> tree, const BinarizedTabularData &binData, int pos=1, int k=0) {
    std::ostringstream oss;

    if(tree[pos] < 0) { // Cas ou l'arbre n'est qu'une feuille
        if(tree[pos] != std::numeric_limits<int>::min()+1) {
            oss << (-tree[pos]-1) << std::endl;
        } else {
            oss << "null" << std::endl;
        }
        return oss.str();
    }

    for(int i=0; i<k; i++) {
        oss << "| ";
    }
    oss << binData.feature2string(tree[pos]);

    if( tree[pos*2+1] < 0 ) {
        if(tree[pos*2+1] != std::numeric_limits<int>::min()+1)
            oss << ": " << binData.getClassName(-tree[pos*2+1]-1) << std::endl;
        else
            oss << ": ?" << std::endl;
    } else {
        oss << std::endl;
        oss << tree2txt(tree, binData, pos*2+1, k+1);
    }

    for(int i=0; i<k; i++) {
        oss << "| ";
    }
    oss << "!" << binData.feature2string(tree[pos]);// << " = F";

    if( tree[pos*2] < 0 ) {
        if(tree[pos*2] != std::numeric_limits<int>::min()+1)
            oss << ": " << binData.getClassName(-tree[pos*2]-1) << std::endl;
        else
            oss << ": ?" << std::endl;
    } else {
        oss << "\n";
        oss << tree2txt(tree, binData, pos*2, k+1);
    }

    return oss.str();
}

std::string tree2dot(const std::vector<int> &tree, const BinarizedTabularData &binData) {
    std::ostringstream oss;

    oss << "digraph mon_graphe {" << std::endl;

    if(tree.size() != 0) {
        for(unsigned int i=1; i<tree.size(); i++) {
            if(tree[i] < 0) {
                if(tree[i] != std::numeric_limits<int>::min()+1)
                    oss << "Q" << i << " [shape=\"box\", label=\""<< binData.getClassName((-tree[i]-1)) <<"\"];" << std::endl;
            } else {
                oss << "Q" << i << " [label=\""<< binData.feature2string(tree[i]) <<" ?\"];" << std::endl;

                oss << "Q" << i << " -> Q" << (i*2) << " [label=\"no\"];" << std::endl;
                oss << "Q" << i << " -> Q" << (i*2+1) << " [label=\"yes\"];" << std::endl;
            }
        }
    }

    oss << "}" << std::endl;

    return oss.str();
}


unsigned int numberNodes(const std::vector<int> &tree) {
    unsigned int nbLeaves=0;
    for(int i=1; i<tree.size(); i++) {
        if((tree[i] < 0) && (tree[i] > (std::numeric_limits<int>::min()+1))) {
            nbLeaves++;
        }
    }
    return nbLeaves*2-1;
}

#endif // TREE_KF39FSF22_H
