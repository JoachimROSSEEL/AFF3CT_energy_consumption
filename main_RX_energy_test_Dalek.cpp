
#include <filesystem>
#include <fstream>

#include <chrono>
#include <ctime>

#include "aff3ct/include/aff3ct.hpp"
#include <streampu.hpp>
#include "LoggerModule.hpp"


using namespace aff3ct;


int main(int argc, char** argv, char** env) {

    std::cout << "Start !" << std::endl;
    auto start = std::chrono::system_clock::now();
    
    // Creating factories for CRC, polar codec, channel and noise level
    factory::CRC CRC_factory;
    factory::Codec_polar codec_polar_factory;
    factory::Channel channel_factory;
    factory::Noise noise_factory;
    factory::Monitor_BFER monitor_factory;
    std::vector<factory::Factory*> params_list = {&codec_polar_factory, &CRC_factory, &noise_factory, &monitor_factory};

    tools::Command_parser cp(argc, argv, params_list, true);
    if (cp.parsing_failed())
    {
        cp.print_help    ();
        cp.print_warnings();
        cp.print_errors  ();
        std::exit(1);
    }
    std::cout << "# Simulation parameters: " << std::endl;
    tools::Header::print_parameters(params_list); // display the headers (= print the AFF3CT parameters on the screen)
    std::cout << "#" << std::endl;
    //cp.print_warnings();

    ///////////////////////////////////
    // Create modules from factory

    // CRC from factory CRC 
    auto CRC = *CRC_factory.build();
    const std::string poly_key = CRC_factory.type;

    // CRC size
    int CRC_size =  CRC.get_size();

    // Number of information bits for polar encoder
    int K = CRC.get_K();

    // Codeword size 
    int N = codec_polar_factory.N;
    
    // Polynomial CRC
    aff3ct::module::CRC_polynomial<> CRC_poly(K, poly_key, CRC_size);
        
    // Codec polar from factory
    aff3ct::tools::Codec_polar<>* codec_polar = codec_polar_factory.build(&CRC_poly);
    
    // Encoder from factory codec
    auto encoder = codec_polar->get_encoder();

    // Decoder from factory codec
    auto decoder = codec_polar->get_decoder_siho();

    // Monitor from factory
    aff3ct::module::Monitor_BFER<> monitor = *monitor_factory.build();

    // Noise level
    double ebn0_min = noise_factory.range[0]; // Minimum SNR value in dB

    double ebn0_max = noise_factory.range.back(); // Maximum SNR value in dB

    double ebn0_step = 1; // SNR step in dB


    std::vector<double> ebn0; // Vector to hold Eb/N0 values
    for (double val = ebn0_min; val < ebn0_max; val += ebn0_step) {
        ebn0.push_back(val);
    }
    
    std::vector<double> esn0; // Vector to hold Es/N0 values
    for (const auto& val : ebn0) {
        esn0.push_back(val + 10 * std::log10(static_cast<double>(K) / N));
    }

    std::vector<double> sigma_vals; // Vector to hold noise standard deviation values

    for (const auto& val : esn0) { // Convert Es/N0 to noise standard deviation
        sigma_vals.push_back(1 / (std::sqrt(2) * std::pow(10, val / 20)));
    }
    

    ///////////////////////////////////
    // Modules not created from factory

    // Binary source of information bits
    //spu::module::MySource_binary     src  (K - CRC_size);
    spu::module::Source_random<> src(K, 12);

    // BPSK Modem
    aff3ct::module::Modem_BPSK  <> mon_modem (N);

    // AWGN Channel with LLR output
    aff3ct::module::Channel_AWGN_LLR<> channel(N);

    // Logger
    //int max_data_size = 100;
    std::string filepath = "/scratch/rosseelj/noisy_cw_polar_" + std::to_string(N) + "_" + std::to_string(encoder.get_K()) + "_CRC_" + CRC_factory.type;
    std::string filename = "/" + std::to_string(N) + "_" + std::to_string(encoder.get_K()) + "_CRC_" + CRC_factory.type + "_" 
    + std::to_string(ebn0_min) + "dB.txt";
    filename = filepath + filename;
    spu::module::LoggerModule<float> logger(filename, N);


    // Monitor for Bit and Frame Error Rates

    ///////////////////////////////////
    // Task binding
    decoder[aff3ct::module::dec::sck::decode_siho::Y_N]       = logger["read::vals"];
    CRC_poly[aff3ct::module::crc::sck::extract::V_K1]         = decoder[aff3ct::module::dec::sck::decode_siho::V_K];
     
    //monitor     [aff3ct::module::mnt::sck::check_errors::V] = Decoder_polar[aff3ct::module::dec::sck::decode_siho::V_K];
    monitor[aff3ct::module::mnt::sck::check_errors::V]        = CRC_poly[aff3ct::module::crc::sck::extract::V_K2];
    //monitor[aff3ct::module::mnt::sck::check_errors::V]        = decoder[aff3ct::module::dec::sck::decode_siho::V_K];
    monitor[aff3ct::module::mnt::sck::check_errors::U]        = CRC_poly[aff3ct::module::crc::sck::extract::V_K2];
    std::vector<float> sigma(1);
    sigma[0] = 0.01f; // Set the noise level 
    
    ///////////////////////////////////
    // Sequence creation

    std::vector<spu::runtime::Task*> first = {&logger("read")};
    spu::runtime::Sequence seq(first);

    for (auto lt : seq.get_tasks_per_types())
        for (auto t : lt)
        {
            t->set_stats(true);
            //t->set_debug(true);
        }  
    
    std::string dec_full_name = decoder.get_name();

    // Convert argv into a string 
    std::string arg1 = "";
    for(int i = 0; i< argc; i++)
        arg1 = arg1 + argv[i] + " ";

    // Find decoders parameters in command line 
    std::size_t found = 0;

    // List decoders
    if(arg1.find("-L ") != std::string::npos)
    {
        found = arg1.find("-L ");
        dec_full_name = dec_full_name + "_L_" + arg1[found + 3];
    }

    // Iterations for SCAN decoder
    if(arg1.find("-i ") != std::string::npos)
    {
        found = arg1.find("-i ");
        dec_full_name = dec_full_name + "_ite_" + arg1[found + 3];
    }

    // Flip for SCF
    if(arg1.find("--dec-flips ") != std::string::npos)
    {
        found = arg1.find("--dec-flips ");
        dec_full_name = dec_full_name + "_nflips_" + arg1[found + 13];
    }  
    // tracker.save_csv(filename_energy);
    // Get SIMD Strategy
    if(arg1.find("--dec-simd ") != std::string::npos)
    {
        found = arg1.find("--dec-simd ");
        dec_full_name = dec_full_name + "_simd_" + arg1.substr(found + 11, 5);
    }

      // Get polar nodes config
    if(arg1.find("--dec-polar-nodes ") != std::string::npos)
    {
        found = arg1.find("--dec-polar-nodes ");
        dec_full_name = dec_full_name + "_nodes_" + arg1.substr(found + 18, arg1.length() - (found + 19)); 
    }
    ///////////////////////////////////
    // Simulation execution
    
    int nb_repeat = 20; 
    for (int k = 0; k < nb_repeat; k++)
    {
        for (size_t i = 0; i < monitor_factory.max_frame; i++){
            // Setting noise values
            
            sigma[0] = sigma_vals[0];
            // Sequence execution
            logger("read").exec();
        
            decoder("decode_siho").exec();

            CRC_poly("extract").exec();
            monitor("check_errors").exec();
            //seq.exec();
        } 
        monitor.reset();
        logger.reset();
    }

    // Time measurement
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

    std::cout << "finished computation at " << std::ctime(&end_time)
              << "elapsed time: " << elapsed_seconds.count() << "s"
              << std::endl;
    std::cout << "End !" << std::endl;
    
    // File to write execution time
    std::string filepath_runtime = "/scratch/rosseelj/runtime/runtime_RX_" + std::to_string(N) + "_" + std::to_string(encoder.get_K()) + "_CRC_" + CRC_factory.type;
    // argc-3 : Decoder and argc-1 : node configuation
    std::string filename_runtime = "/Decoder_polar_" + std::to_string(*argv[argc-3]) + "_nodes_" + std::to_string(*argv[argc-1]) + ".txt";
    std::filesystem::create_directory(filepath_runtime);
    filename = filepath_runtime + filename_runtime;
    std::ofstream filestream_runtime;
    filestream_runtime.open(filename_runtime);
    filestream_runtime << "Node_configuration Run_Time(s) \n";
    filestream_runtime << std::to_string(*argv[argc-1]) + " " + std::to_string(elapsed_seconds.count()) + "\n";
    filestream_runtime.close();
    std::cout << "End !" << std::endl;

}
