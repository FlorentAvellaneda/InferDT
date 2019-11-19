

#ifndef __MALIB__COUTUTIL____
#define __MALIB__COUTUTIL____ 


#include <iostream>
#include <vector>
#include <deque>
#include <set>
#include <tuple>
#include <map>
#include <sstream>
#include <string>


namespace MaLib
{

template <class T>
std::tuple<T, int> max(const std::vector<T> &vect) {
    if(vect.size()==0) {
        T tmp;
        return std::make_tuple(tmp, -1);
    }
    T vMax = vect[0];
    int id=0;

    for(int i=1; i<vect.size(); i++) {
        if(vMax < vect[i]) {
            vMax = vect[i];
            id = i;
        }
    }

    return std::make_tuple(vMax, id);
}

template <class T>
std::tuple<T, int> min(const std::vector<T> &vect) {
    if(vect.size()==0) {
        T tmp;
        return std::make_tuple(tmp, -1);
    }
    T vMin = vect[0];
    int id=0;

    for(int i=1; i<vect.size(); i++) {
        if(vMin > vect[i]) {
            vMin = vect[i];
            id = i;
        }
    }

    return std::make_tuple(vMin, id);
}

class LogCall {
public:
    static void call(int decalage) {
        static unsigned int nb = 0;

        if(decalage != 0) {
            nb+=decalage;
        } else {
            for(unsigned int i=0; i<nb; i++)
                std::cout << "  ";
        }
    }

    LogCall(std::string msg) {
        LogCall::call(0);
        std::cout << msg << std::endl;
        LogCall::call(1);
    }

    ~LogCall() {
        std::cout << "-1" << std::endl;
        LogCall::call(-1);
    }
};

#define LOG MaLib::LogCall::call(0); std::cout
//#define LOG if(false) std::cout

#define LOG_CALL(txt) MaLib::LogCall LOG_15768312_INT(txt);
//#define LOG_CALL(txt)



// Afficher le continue d'un vector
template <class T>
std::ostream& operator<< (std::ostream &output, const std::vector<T> &v) {
    unsigned int size=v.size();
    output << "[";
    for(unsigned int i=0; i<size; i++) {
        output << v[i];
        if( i+1 < size)
            output << ", ";
    }
    output << "]";
    return output;
}

// Afficher le continue d'un deque
template <class T>
std::ostream& operator<< (std::ostream &output, const std::deque<T> &v) {
    unsigned int size=v.size();
    output << "[";
    for(unsigned int i=0; i<size; i++) {
        output << v[i];
        if( i+1 < size)
            output << ", ";
    }
    output << "]";
    return output;
}

// Afficher le continue d'un set
template <class T>
std::ostream& operator<< (std::ostream &output, const std::set<T> &s) {
	output << "{";
	bool first=true;
	for(auto &e: s) {
		if(!first)
			output << ", ";
		else
    	first=false;
		output << e;
	}
	output << "}";
	return output;
}

namespace {
	template<std::size_t> struct int_{};

	template <class Tuple, size_t Pos>
	std::ostream& print_tuple(std::ostream& out, const Tuple& t, int_<Pos> ) {
	  out << std::get< std::tuple_size<Tuple>::value-Pos >(t) << ", ";
	  return print_tuple(out, t, int_<Pos-1>());
	}

	template <class Tuple>
	std::ostream& print_tuple(std::ostream& out, const Tuple& t, int_<1> ) {
	  return out << std::get<std::tuple_size<Tuple>::value-1>(t);
	}
}

// Afficher le continue d'un tuple
template <class... Args>
std::ostream& operator<< (std::ostream &output, const std::tuple<Args...> &t) {
    output << "<";
    print_tuple(output, t, int_<sizeof...(Args)>()); 
    output << ">";
    return output;
}

template <class T1, class T2>
std::ostream& operator<< (std::ostream &output, const std::pair<T1, T2> &p) {
    output << "(" << p.first << ", " << p.second << ")";
    return output;
}


// Afficher le continue d'un map
template <class T1, class T2>
std::ostream& operator<< (std::ostream &output, const std::map<T1, T2> &s) {
	output << "{";
	bool first=true;
	for(auto &e: s) {
		if(!first)
			output << ", ";
		else
    	first=false;
		output << e.first << ": " << e.second;
	}
	output << "}";
	return output;
}


}


#endif
