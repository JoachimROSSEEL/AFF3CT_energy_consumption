#include "LoggerModule.hpp"

#include <thread>

#include <typeinfo>

using namespace spu;
using namespace spu::module;

template <typename T> 
LoggerModule<T>::LoggerModule(std::string filename, const int N) : Stateful(), filename(filename), N(N), is_first(true), total_count(0) 
{

    this->filestream.open(this->filename);
    this->filestream << std::setprecision(10);

    this->set_name("LoggerModule");
    this->set_short_name("LoggerModule");

    auto &t_log = create_task("log");
    auto val_in = create_socket_in<T>(t_log, "vals", this->N);

    this->create_codelet(t_log, [val_in](Module &m, runtime::Task &t, const size_t frame_id) -> int {
        static_cast<LoggerModule&>(m)._log( static_cast<T*>(t[val_in].get_dataptr()),
                                                frame_id);
        return 0;
    });

    auto &t_read = create_task("read");
    auto val_out = create_socket_out<T>(t_read, "vals", this->N);

    this->create_codelet(t_read, [val_out](Module &m, runtime::Task &t, const size_t frame_id) -> int {
        static_cast<LoggerModule&>(m)._read(static_cast<T*>(t[val_out].get_dataptr()), frame_id);
        return 0;
    });
   
}


template <typename T> 
LoggerModule<T>::~LoggerModule() 
{
    this->filestream.close();
}

template <typename T> 
void LoggerModule<T>::reset() 
{
    this->filestream.close();
    this->filestream.open(this->filename);
    this->filestream << std::setprecision(10);
}

template <typename T>
void LoggerModule<T>::_log(const T *vals, const int frame_id) 
{
    this->total_count++;
    if(!this->is_first)
    {
        this->filestream << " \n";
    }
    for(int i = 0; i < N; i++)
    {
        this->filestream << " "; 
        this->filestream << vals[i]; 
    }
    if(this->is_first) this->is_first = false;
}
// tous read d'un coup MMAP (adresse + matrice), tout stocker en RAM et on décode 
template <typename T>
void LoggerModule<T>::_read(T* vals, const int frame_id) 
{
  std::string line;
  std::getline(this->filestream,line);
  //std::vector buff; 
  std::stringstream sline(line);
  std::string word;
  this->total_count++;
  //std::cout << line << '\n';
  int i = 0;
  while(sline >> word)
  {
    vals[i] = std::stof(word);
    i++;
  }

}

template class spu::module::LoggerModule<float>;
template class spu::module::LoggerModule<int>;
