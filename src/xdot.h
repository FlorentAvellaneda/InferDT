
#ifndef __MALIB__XDOT__H__
#define __MALIB__XDOT__H__ 

#include <string>
#include <fstream>
#include <sstream>

class xdot
{
public:
    xdot(){

    }
    ~xdot(){

    }

    static void show(std::string dot, bool pause=true) {
        static int num=100;

        std::ostringstream oss;
        oss << "/tmp/xdotFile" << num++ << ".dot";
        std::string fileName = oss.str();
        {
            std::string cuState;
            std::ofstream file( fileName );

            file << dot << std::endl;
        }

        std::string cmd = "xdot ";
        cmd += fileName;
        cmd += " > /dev/null 2> /dev/null";
        if(!pause)
            cmd += " &";
        if( std::system (cmd.c_str()) ) {
        }
    }

    static void save(std::string dot, std::string nameSG="") {
        static int num=100;

        std::ostringstream oss;
        if(nameSG.size()==0) {
            oss << "/tmp/xdotFile" << num++ << ".dot";
        } else {
            oss << "/tmp/" << nameSG << ".dot";
        }
        std::string fileName = oss.str();
        {
            std::string cuState;
            std::ofstream file( fileName );

            file << dot << std::endl;
        }
    }

    static void showImg(std::string dot, bool pause=true) {
        static int num=100;
        num++;

        // Ecrire le dot
        std::string fileNameDot;
        {
            std::ostringstream oss;
            oss << "/tmp/xxdotFile" << num << ".dot";
            fileNameDot = oss.str();
            std::ofstream file( fileNameDot );
            file << dot << std::endl;
        }

        // Generer le jpg
        std::string fileNameJPG;
        {
            std::ostringstream oss;
            oss << "/tmp/xxdotFile" << num << ".jpg";
            fileNameJPG = oss.str();
            if(std::system (("dot -Tjpeg "+fileNameDot+" -o " + fileNameJPG).c_str())) {
            }
        }

        if(!pause) {
            if( std::system (("eog " + fileNameJPG + " > /dev/null 2> /dev/null &").c_str()) ) {
            }
        } else {
            if( std::system (("eog " + fileNameJPG + " > /dev/null 2> /dev/null").c_str()) ) {
            }
        }
    }
};



#endif

