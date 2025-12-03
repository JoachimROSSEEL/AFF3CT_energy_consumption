// Generate codewords and write them in a file
// même graîne: même mot de code 
// faut même bruit pour test les décodeurs. 
// Si batch pas trop bruité sur un et un autre bien bruité ça sera pas pareil (notamment déc itératif)
// faut même condition
// faudrait bruit ligne commande
#include <filesystem>
#include <fstream>

// Power measurement
#include <cppJoules.h>

#include <aff3ct.hpp>
#include <streampu.hpp>
#include "LoggerModule.hpp"




using namespace aff3ct;


int main(int argc, char** argv, char** env) {

    //std::cout << "Start !" << std::endl;

    EnergyTracker tracker;
    
    
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
    //std::cout << poly_key  << " crc type facto\n";

    // CRC size
    int CRC_size =  CRC.get_size();
    //std::cout << CRC_size  << " CRC size\n";

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
    // std::cout << ebn0_min << " = ebno_min\n";
    double ebn0_max = noise_factory.range.back(); // Maximum SNR value in dB
    // std::cout << ebn0_max << " = ebno_max\n";
    double ebn0_step = 1; // SNR step in dB
    // std::cout << ebn0_step << " = ebno_step\n";

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
    std::string filepath = "noisy_cw_polar_" + std::to_string(N) + "_" + std::to_string(encoder.get_K()) + "_CRC_" + CRC_factory.type;
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
    // channel[aff3ct::module::chn::sck::add_noise::CP].bind(sigma);
    // mon_modem[aff3ct::module::mdm::sck::demodulate::CP].bind(sigma);
    
    ///////////////////////////////////
    // Sequence creation

    std::vector<spu::runtime::Task*> first = {&logger("read")};
    spu::runtime::Sequence seq(first);

    std::ofstream file("graph_rx_gen.dot");

    seq.export_dot(file);

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

    //std::vector<double> fer(ebn0.size(), 0.0);
    //std::vector<double> ber(ebn0.size(), 0.0);
    
    // tracker.start();
    // int nb_repeat = 3; 
    // for (int k = 0; k < nb_repeat; k++)
    // {
    //     for (size_t i = 0; i < sigma_vals.size(); ++i)
    //     {
    //         // Setting noise values
    //         sigma[0] = sigma_vals[0];
    //         // Sequence execution
    //         seq.exec();

    //         monitor.reset();
    //     }
    //     std::cout << "one repeat\n"; 
    // }

    // tracker.stop();
    
    tracker.start();
    int nb_repeat = 2; 
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
        //std::cout << "one repeat\n";
    }
    tracker.stop();
    //spu::tools::Statistics::show(seq.get_tasks_per_types());
    //seq.show_stats();
    
    tracker.calculate_energy();
    //tracker.print_energy();

    // Saving energy consumption
    std::string filepath_energy = "energy/energy_polar_2_" + std::to_string(N) + "_" + std::to_string(encoder.get_K()) + "_CRC_" + CRC_factory.type + "_" 
    + decoder.get_name();
    //std::filesystem::current_path(std::filesystem::temp_directory_path());
    std::filesystem::create_directory(filepath_energy);
    std::string filename_energy =  filepath_energy + "/" + dec_full_name + ".csv";
    tracker.save_csv(filename_energy);
    //std::cout << "End !" << std::endl;
    //stats_output_f.close();

}
