

#ifndef LOGGER_MODULE_HPP_
#define LOGGER_MODULE_HPP_

#include <iostream>
#include <fstream>
#include <iomanip>

#include "aff3ct/lib/streampu/include/Module/Stateful/Stateful.hpp"


namespace spu
{
namespace module
{

template <typename T> class LoggerModule : public Stateful {

private:
    std::string filename;           // file to store channel data 
    std::fstream filestream;       // file stream 
    const int N;                    // frame size
    bool is_first;                  // first execution
    int total_count;                // counter of execution number

public:
    LoggerModule(std::string filename, const int N);
    virtual ~LoggerModule();
    // Reset reading/writing at start of file
    void reset(); 

protected:
    virtual void _log(const T *vals, const int frame_id);
    virtual void _read(T* vals, const int frame_id);

//private:
    //static void dumpBuffer(std::vector<std::vector<T>> &buffer, std::vector<std::ofstream> &file_streams, bool encode=false);
    //void dumpBufferThread(bool wait=false);
};
}
}

#endif