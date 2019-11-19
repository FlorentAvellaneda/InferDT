

#ifndef UTILE__H
#define UTILE__H 

#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <ostream>


namespace {
	template<class T>
	std::vector< std::vector<T> > combinaisonImpl(const std::vector<T> & elements, unsigned int nombre, unsigned int first, std::vector<T> prefix) {
		//assert(nombre <= (elements.size() - first));

		if( nombre==0 )
			return { prefix };

		std::vector< std::vector<T> > result1;


		if(nombre < (elements.size() - first) )
			result1 = combinaisonImpl( elements, nombre, first+1, prefix );

		prefix.push_back(elements[first]);
		std::vector< std::vector<T> > result2 = combinaisonImpl( elements, nombre-1, first+1, prefix );
	

		result1.insert(result1.end(), result2.begin(), result2.end());


		return result1;
	}
}


template<class T>
std::vector< std::vector<T> > combinaison(const std::vector<T> & elements, unsigned int nombre) {
	if( nombre > elements.size() )
		return std::vector< std::vector<T> >();

	return combinaisonImpl(elements, nombre, 0, std::vector<T>());
}


// Vérifier si le motif correspond au string
bool validate_card_format(const std::string source, const std::string motif);

// Découpe la chaine selon les séparateurs donnés
std::vector<std::string> split( const std::string & source, const std::string & separateurs );

// Convertir un string en T
template<class T>
inline T s2all( const std::string & source )
{
    T result;
    std::istringstream iss( source );
    iss >> result;

    return result;
}
template<>
inline std::string s2all<std::string>( const std::string & source )
{
    return source;
}

// Convertir un tableau de string en tableau de T
template <typename T>
std::vector<T> vString2vAll (const std::vector<std::string>& v)
{
    std::vector<T> result;
    result.reserve(v.size());

    for(auto i: v) {
        result.push_back( s2all<T>( i ) );
    }

    return result;
}

// Convertir un tableau de string en tableau de T ( pour le sous tableau line(vPos[i]) )
template <typename T>
std::vector<T> vString2vAll (const std::vector<std::string>& line, const std::vector<unsigned int> &vPos)
{
    std::vector<T> result;
    result.reserve(vPos.size());

    for(auto i: vPos) {
        result.push_back( s2all<T>( line[i] ) );
    }

    return result;
}

// SUPPRIMER ?
template <typename T>
std::tuple<T> vString2tuple (const std::vector<std::string>& line, const unsigned int beginPos=0)
{
    return std::tuple<T>( s2all<T>(line[beginPos]) );
}

// Convertir un vector de string en tuple de <T1, ... >
template <
    typename T1,
    typename T2,
    typename... Args>
std::tuple<T1, T2, Args...> vString2tuple (const std::vector<std::string>& line, const unsigned int beginPos=0)
{
    return std::tuple_cat(
        std::tuple<T1>( s2all<T1>(line[beginPos]) ),
        vString2tuple<T2, Args...>(line, beginPos+1)
    );
}

// TypeTuple<T, n>::type correspond a un tuple de <T, T, ... T> (n fois)
template <class T, int DEGREE>
struct TypeTuple {
    typedef decltype(
        std::tuple_cat(
            std::declval<std::tuple<T>>(),
            std::declval<typename TypeTuple<T, DEGREE-1>::type >()
        )
    ) type;
};
template <class T>
struct TypeTuple<T, 0> {
    typedef std::tuple<> type;
};



namespace {
    template <int ID, class F, class... ARGS>
    struct for_each_tuple_rec {
        void operator() (std::tuple<ARGS...> &t, F f) {
            for_each_tuple_rec<ID-1, F, ARGS...>()(t, f);
            f( std::get<ID>(t), ID );
        }
    };

    template <class F, class... ARGS>
    struct for_each_tuple_rec<0, F, ARGS...> {
        void operator() (std::tuple<ARGS...> &t, F f) {
            f( std::get<0>(t), 0 );
        }
    };
}

// Appliquer la fonction f(T Obj, int pos) pour chaque élément du tuple
template <class... ARGS, class F>
void for_each_tuple(std::tuple<ARGS...> &t, F f) {
    for_each_tuple_rec<
        std::tuple_size< std::tuple<ARGS...> >::value-1,
        F,
        ARGS...>()(t, f);
}

// garder le sous tableau v[IDargs[*]]
template <class T>
std::vector<T> subVector(std::vector<T> v, std::vector<unsigned int> IDargs) {
    std::vector<T> result;
    result.reserve(IDargs.size());
    for(auto i: IDargs)
        result.push_back(v[i]);
    return result;
}

// garder le sous tableau v - v[IDargs[*]]
template <class T, class C>
std::vector<T> complSubVector(std::vector<T> v, C IDargs) {
    std::vector<T> result;
    result.reserve(v.size()-IDargs.size());
    for(unsigned int i=0; i<v.size(); i++) {
        if( std::find(IDargs.begin(), IDargs.end(), i) == IDargs.end() ) {
            result.push_back( v[i] );
        }
    }

    return result;
}



#endif
