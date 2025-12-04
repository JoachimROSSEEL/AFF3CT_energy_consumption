// Main to test writing in a Dalek node.
// It writes number from 0 to 999 in a file.

#include <../aff3ct/include/aff3ct.hpp>
#include <filesystem>
#include <streampu.hpp>
#include "LoggerModule.hpp"
#include <iostream>
//#include <fstream>

using namespace aff3ct;


int main(int argc, char** argv, char** env) {


    //std::filesystem::path filepath = "/testes/test";
    std::string filepath = "./scratch/rosseelj/test";
    std::string filename = "/numbers.txt";
    std::filesystem::create_directories(filepath);
    filename = filepath + filename;
    std::ofstream filestream;
    filestream.open(filename);

    for(int i = 0; i < 1000; i++)
    {
        filestream << i; 
        filestream << " \n";
    }
    filestream.close();

}
