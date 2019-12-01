#ifndef MALIB_MOYENNE_H
#define MALIB_MOYENNE_H


#include <iostream>
#include <vector>
#include <cmath>


// Welford's Online Algorithm
class Moyenne {
    unsigned int nb = 0;
    double moy = 0.0;
    double M2 = 0.0;
    double val_t = 3.0; // Pour le % de certitude (1 => 68; 2 => 95% ; 3 => 99,7)
    std::string title;
public:

    Moyenne(std::string title = "") : title(title) {
    }

    // "t" implique le % de certitude (1 => 68%; 2 => 95% ; 3 => 99.7%)
    void setT(double t) {
        val_t = t;
    }

    //
    void add(double val) {
        ++nb;
        double delta = val - moy;
        moy += delta / (double) nb;
        M2 += delta * (val - moy);
    }

    // Valeur minimal de la moyenne avec une certitude qui dépend de t
    // Attention, si nb est trop petit la valeur peut étre erroné
    double getMoyMin() const {
        return getMoy() - val_t * (getEcartType() / sqrt(nb));
    }

    // Valeur maximal de la moyenne avec une certitude qui dépend de t
    // Attention, si nb est trop petit la valeur peut étre erroné
    double getMoyMax() const {
        return getMoy() + val_t * (getEcartType() / sqrt(nb));
    }

    double getMoy() const {
        return moy;
    }

    double getVariance() const {
        return M2 / ((double) nb - 1.0);
    }

    double getEcartType() const {
        return sqrt(getVariance());
    }

    double size() {
        return nb;
    }

    void print(std::string unite = "") {
        std::cout << "Average " << title << ": " << getMoy() << unite << " +- " << (getMoyMax() - getMoy()) << unite << std::endl;
    }

    ~Moyenne() {
    }
};


#endif


