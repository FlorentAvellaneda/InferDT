#ifndef CSV__H
#define CSV__H 



#include <boost/tokenizer.hpp> 
#include <string>
#include <tuple>
#include <iostream>
#include <type_traits>
#include <fstream>      // std::filebuf

#include "utile.h"



class CSV {

	std::filebuf * fb=nullptr;

	std::istream * _is=nullptr;
	boost::escaped_list_separator<char> _els;

	std::vector<std::string> vLine;

	bool end=false;


	public:
		CSV(std::istream& is, char sep=',', char quote='\"', char escape='\\') 
			: _is(&is), _els(escape, sep, quote) {

			//next();
		}

		CSV(std::string fileName, char sep=',', char quote='\"', char escape='\\')
			: _els(escape, sep, quote) {

			  fb = new std::filebuf();
			  if (fb->open (fileName,std::ios::in))
			  {
			    _is = new std::istream(fb);// std::istream is(fb);
			    //_els = boost::escaped_list_separator<char>(escape, sep, quote);

		    	//this(is);
		    	
			    
			  } else {
			  	std::cerr << "Lecture du fichier " << fileName << " imposible." << std::endl;
			  }
		}

		~CSV() {
			delete _is;
			fb->close();
			delete fb;
		}

        unsigned int size() {
            return vLine.size();
        }

		bool isEnd() {
			return end;
		}

		bool next() {
			std::string line;

			if(!std::getline(*_is, line)) {
				end = true;
				return false;
			}

            if(line.back() == 13) {
                line.pop_back();
            }

            vLine = parseLine(line);

			end = false;
			return true;
		}

		// Accesseur sans type sans position
		public:

		std::vector<std::string> & get() {
			return vLine;
		}

		std::vector< std::vector<std::string> > getAll() {
			std::vector< std::vector<std::string> > result;

			if(isEnd())
				return result;

			do {
				result.push_back( get() );
			} while( next() );

			return result;
		}

		template< class F >
		std::vector < std::vector<std::string> > getUntil(F f) {
			std::vector< std::vector<std::string> > result;

			if(isEnd())
				return result;

			do {
				if(!f(get()))
					return result;
				result.push_back( get() ); 
			} while( next() );

			return result;
		}

		// Accesseur sans type avec position
		public:

		std::vector<std::string> get(const std::vector<unsigned int> &pos) {
			std::vector<std::string> result;
			result.reserve( pos.size() );

			for(auto i: pos) {
				result.push_back( vLine[i] );
			}
			return result;
		}

		std::vector< std::vector<std::string> > getAll(const std::vector<unsigned int> &pos) {
			std::vector < std::vector<std::string> > result;

			if(isEnd())
				return result;

			do {
				result.push_back( get(pos) ); 
			} while( next() );

			return result;
		}

		template< class F >
		std::vector < std::vector<std::string> > getUntil(F f, const std::vector<unsigned int> &pos) {
			std::vector < std::vector<std::string> > result;

			if(isEnd())
				return result;

			do {
				if(!f(get(), get(pos)))
					return result;
				result.push_back( get(pos) ); 
			} while( next() );

			return result;
		}

		// Accesseur avec type sans position
		public:

        template<class T>
        std::vector<T> getSameType() {
            return vString2vAll<T>(vLine);
        }

		template<class... ARGS>
		std::tuple<ARGS...>
		get() {
			return vString2tuple<ARGS...>(vLine);
		}

		template<class... ARGS>
		std::vector< std::tuple<ARGS...> > 
		getAll() {
			std::vector< std::tuple<ARGS...> > result;

			if(isEnd())
				return result;

			do {
				result.push_back( get<ARGS...>() );
			} while( next() );

			return result;
		}

		template<class T, class... ARGS, class F>
		std::vector < std::tuple<T, ARGS...> > getUntil(F f) {
			std::vector< std::tuple<T, ARGS...> > result;

			if(isEnd())
				return result;

			do {
				if(!f(get(), get<T, ARGS...>()))
					return result;
				result.push_back( get<T, ARGS...>() ); 
			} while( next() );

			return result;
		}

		// Accesseur avec type et position
		public:

		template<class T>
		std::vector<T> get(const std::vector<unsigned int> &pos) {
            return vString2vAll<T>(vLine, pos);
		}

		template<class T>
		std::vector< std::vector<T> > 
		getAll(const std::vector<unsigned int> &pos) {
			std::vector< std::vector<T> > result;

			if(isEnd())
				return result;

			do {
				result.push_back( get<T>(pos) );
			} while( next() );

			return result;
		}


        template<class T>
        std::vector< std::vector<T> >
        getAllSameType() {
            std::vector< std::vector<T> > result;

            if(isEnd())
                return result;

            do {
                result.push_back( getSameType<T>() );
            } while( next() );

            return result;
        }

		template<class T, class F>
		std::vector < std::vector<T> > getUntil( F f, const std::vector<unsigned int> &pos ) {
			std::vector< std::vector<T> > result;

			if(isEnd())
				return result;

			do {
				if(!f(get(), get<T>(pos)))
					return result;
				result.push_back( get<T>(pos) ); 
			} while( next() );

			return result;
		}


	private:
		std::vector<std::string> parseLine(std::string &s) {
			std::vector<std::string> result;

		    for(unsigned int i=1; i<s.size(); ) {
		        if(s[i] == '"') {
		            if( s[i-1] == '"') {
		                s[i-1] = '\\';
		                i+=2;
		            } else {
		                i+=1;
		            }
		        } else {
		            i+=2;
		        }
		    }

			boost::tokenizer<boost::escaped_list_separator<char>> tok(s, _els);

			for(auto c: tok)
				result.push_back(c);

			return result;
		}


};




#endif
