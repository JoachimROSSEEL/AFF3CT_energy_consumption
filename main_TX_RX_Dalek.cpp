// Main to compute FER and BER of a polar code communication chain. 
// Modules parameters (such as the CRC, polar code dimensions and the decoder) are passed thanks to AFF3CT factories.

#include "aff3ct/include/aff3ct.hpp"
#include <streampu.hpp>

#include <fstream>


using namespace aff3ct;

struct utils1
    {

        std::unique_ptr<aff3ct::tools::Sigma<>>               noise;  // a sigma noise type

        std::vector<std::unique_ptr<spu::tools::Reporter>> reporters; // list of reporters dispayed in the terminal

        std::unique_ptr<spu::tools::Terminal_std>          terminal;  // manage the output text in the terminal
    };

    void init_utils1(aff3ct::module::Monitor_BFER<> &monitor, utils1 &u)
    {
        // create a sigma noise type

        u.noise = std::unique_ptr<aff3ct::tools::Sigma<>>(new aff3ct::tools::Sigma<>());

        // report the noise values (Es/N0 and Eb/N0)

        u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(new aff3ct::tools::Reporter_noise<>(*u.noise)));

        // report the bit/frame error rates

        u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(new aff3ct::tools::Reporter_BFER<>(monitor)));

        // report the simulation throughputs

        u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(new aff3ct::tools::Reporter_throughput<>(monitor)));

        // create a terminal that will display the collected data from the reporters

        u.terminal = std::unique_ptr<spu::tools::Terminal_std>(new spu::tools::Terminal_std(u.reporters));
    }


int main(int argc, char** argv, char** env) {

    std::cout << "Start !" << std::endl;
    
    // Creating factories for CRC, polar codec, channel and noise level
    factory::CRC CRC_factory;
    factory::Codec_polar codec_polar_factory;
    factory::Channel channel_factory;
    factory::Noise noise_factory;
    std::vector<factory::Factory*> params_list = {&codec_polar_factory, &CRC_factory, &noise_factory};

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
    cp.print_warnings();

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

    // Noise level
    double ebn0_min = noise_factory.range[0]; // Minimum SNR value in dB
    // std::cout << ebn0_min << " = ebno_min\n";
    double ebn0_max = noise_factory.range.back(); // Maximum SNR value in dB
    // std::cout << ebn0_max << " = ebno_max\n";
    double ebn0_step = 0.25; // SNR step in dB
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

    std::vector<bool> frozen_bits(N);

    // AWGN Channel with LLR output
    aff3ct::module::Channel_AWGN_LLR<> channel(N);

    // Monitor for Bit and Frame Error Rates
    aff3ct::module::Monitor_BFER<> monitor(K, 100);


    ///////////////////////////////////
    // Factory and polar codes
    
    // Polynomial CRC
    aff3ct::module::CRC_polynomial<> CRC_poly(K, poly_key, CRC_size);
        
    // Codec polar from factory
    aff3ct::tools::Codec_polar<>* codec_polar = codec_polar_factory.build(&CRC_poly);
    
    // Encoder from factory codec
    auto encoder = codec_polar->get_encoder();

    // Decoder from factory codec
    auto decoder = codec_polar->get_decoder_siho();

    ///////////////////////////////////
    // Task binding

    CRC_poly[aff3ct::module::crc::sck::build::U_K1]           = src[spu::module::src::sck::generate::out_data];
    encoder[aff3ct::module::enc::sck::encode::U_K]            = CRC_poly[aff3ct::module::crc::sck::build::U_K2];
    mon_modem[aff3ct::module::mdm::sck::modulate::X_N1]       = encoder[aff3ct::module::enc::sck::encode::X_N];
    channel[aff3ct::module::chn::sck::add_noise::X_N]         = mon_modem[aff3ct::module::mdm::sck::modulate::X_N2]; 

    decoder[aff3ct::module::dec::sck::decode_siho::Y_N]       = channel[aff3ct::module::chn::sck::add_noise::Y_N];
    CRC_poly[aff3ct::module::crc::sck::extract::V_K1]         = decoder[aff3ct::module::dec::sck::decode_siho::V_K];
     
    //monitor     [aff3ct::module::mnt::sck::check_errors::V] = Decoder_polar[aff3ct::module::dec::sck::decode_siho::V_K];
    monitor[aff3ct::module::mnt::sck::check_errors::V]        = CRC_poly[aff3ct::module::crc::sck::extract::V_K2];
    //monitor[aff3ct::module::mnt::sck::check_errors::V]        = decoder[aff3ct::module::dec::sck::decode_siho::V_K];
    monitor[aff3ct::module::mnt::sck::check_errors::U]        = src[spu::module::src::sck::generate::out_data];

    std::vector<float> sigma(1);
    sigma[0] = 0.01f; // Set the noise level 
    channel[aff3ct::module::chn::sck::add_noise::CP].bind(sigma);
    mon_modem[aff3ct::module::mdm::sck::demodulate::CP].bind(sigma);

    ///////////////////////////////////
    // Sequence creation

    std::vector<spu::runtime::Task*> first = {&src("generate")};
    spu::runtime::Sequence seq(first);

    for (auto lt : seq.get_tasks_per_types())
        for (auto t : lt)
        {
            t->set_stats(true);
            //t->set_debug(true);
        }
    
    ///////////////////////////////////
    // Utils 
    // Terminal display
    utils1 u;   init_utils1  (monitor, u); //
    u.terminal->legend();    
    
    // FER and BER arrays and files
    std::vector<double> fer(ebn0.size(), 0.0);
    std::vector<double> ber(ebn0.size(), 0.0);

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
        dec_full_name = dec_full_name + "_nflips_" + arg1[found + 12];
    }  

    // Get polar nodes config
    if(arg1.find("--dec-polar-nodes ") != std::string::npos)
    {
        found = arg1.find("--dec-polar-nodes ");
        dec_full_name = dec_full_name + "_nodes_" + arg1.substr(found + 18, arg1.length() - (found + 19));
    }  

    // // FER file according to the decoder 
    // std::string filename_FER = "FER/FER_polar_" + std::to_string(N) + "_" + std::to_string(encoder.get_K()) + "_CRC_" + CRC_factory.type + "_" 
    // + dec_full_name + ".txt";
    // std::ofstream filestream_FER;
    // filestream_FER.open(filename_FER);

    // // BER file according to the decoder 
    // std::string filename_BER = "BER/BER_polar_" + std::to_string(N) + "_" + std::to_string(encoder.get_K()) + "_CRC_" + CRC_factory.type + "_" 
    // + dec_full_name + ".txt";
    // std::ofstream filestream_BER;
    // filestream_BER.open(filename_BER);

    
    ///////////////////////////////////
    // Simulation execution

    for (size_t i = 0; i < sigma_vals.size(); i++){
        // Setting noise values and frozen bits
        sigma[0] = sigma_vals[i];
    
        // Display the performance (BER and FER) in real 
        u.noise->set_values(sigma_vals[i], ebn0[i], esn0[i]);
        u.terminal->start_temp_report();

        // Sequence execution
        seq.exec();

        // Get BER/FER 
        ber[i] = monitor.get_ber();
        fer[i] = monitor.get_fer();
  
        // Display the performance (BER and FER) in the terminal
        u.terminal->final_report();
        //stats_output_f << "Frames: " << monitor.get_n_analyzed_fra() << " Ebn0: " << ebn0[i] << " Sigma: " << sigma[0] << " BER: " << ber[i] << " FER: " << fer[i] << std::endl; 
        
        // Reset the monitor and the terminal for the next SNR
        //u.terminal->reset();
        monitor.reset();
    } 
    spu::tools::Statistics::show(seq.get_tasks_per_types());
    //seq.show_stats();
    //tracker.save_csv("results_polar.csv");
    std::cout << "End !" << std::endl;
    //stats_output_f.close();
    
    // int j = 0;
    // for(j = 0; j < sigma_vals.size(); j++)
    // {
    //     filestream_FER << std::to_string(ebn0[j]) + " " + std::to_string(fer[j]) + "\n";
    //     filestream_BER << std::to_string(ebn0[j]) + " " + std::to_string(ber[j]) + "\n";
    // }
    // filestream_FER.close();
    // filestream_BER.close();

}
