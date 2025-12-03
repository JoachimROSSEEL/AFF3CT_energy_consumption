// Main to generate a noisy codewords set on the Dalek cluster
// Modules parameters (such as the CRC and polar code dimensions) are passed thanks to AFF3CT factories.


#include <aff3ct.hpp>
#include <filesystem>
#include <streampu.hpp>
#include "LoggerModule.hpp"

#include <fstream>

// Power measurement
#include <cppJoules.h>

using namespace aff3ct;


int main(int argc, char** argv, char** env) {

    std::cout << "Start !" << std::endl;

    EnergyTracker tracker;
    tracker.start();
    
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
    cp.print_warnings();

    ///////////////////////////////////
    // Create modules from factory

    // CRC from factory CRC 
    aff3ct::module::CRC<> CRC = *CRC_factory.build();

    // Channel from factory channel
    //aff3ct::module::Channel<> channel = *channel_factory.build();

    // Encoder from factory codec
    aff3ct::tools::Codec_polar<>* codec_polar = codec_polar_factory.build(); 
    auto encoder = codec_polar->get_encoder();

    // Decoder from factory codec
    auto decoder = codec_polar->get_decoder_siho();

    // Monitor from factory
    aff3ct::module::Monitor_BFER<> monitor = *monitor_factory.build();

    // Number of information bits for polar encoder
    int K = encoder.get_K();
    // Codeword size 
    int N = encoder.get_N(); 
    
    // CRC size
    int CRC_size =  CRC.get_size();
    //std::cout << CRC_size << "crc size\n";

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
        esn0.push_back(val + 10 * std::log10(static_cast<double>(K-CRC_size) / N));
    }

    std::vector<double> sigma_vals; // Vector to hold noise standard deviation values

    for (const auto& val : esn0) { // Convert Es/N0 to noise standard deviation
        sigma_vals.push_back(1 / (std::sqrt(2) * std::pow(10, val / 20)));
    }
    

    ///////////////////////////////////
    // Modules not created from factory

    // Binary source of information bits
    //spu::module::MySource_binary     src  (K - CRC_size);
    spu::module::Source_random<> src(K - CRC_size, 12);

    // BPSK Modem
    aff3ct::module::Modem_BPSK  <> mon_modem (N);

    // AWGN Channel with LLR output
    aff3ct::module::Channel_AWGN_LLR<> channel(N);

    // Logger
    //int max_data_size = 100;
    std::string filepath = "noisy_cw_polar_" + std::to_string(N) + "_" + std::to_string(encoder.get_K()) + "_CRC_" + CRC_factory.type;
    std::string filename = "/" + std::to_string(N) + "_" + std::to_string(encoder.get_K()) + "_CRC_" + CRC_factory.type + "_" 
    + std::to_string(ebn0_min) + "dB.txt";
    std::filesystem::create_directory(filepath);
    filename = filepath + filename;
    std::ofstream filestream;
    filestream.open(filename);
    filestream.close();
    spu::module::LoggerModule<float> logger(filename, N);


    // Monitor for Bit and Frame Error Rates

    ///////////////////////////////////
    // Task binding

    CRC[aff3ct::module::crc::sck::build::U_K1]                = src[spu::module::src::sck::generate::out_data];
    encoder[aff3ct::module::enc::sck::encode::U_K]            = CRC[aff3ct::module::crc::sck::build::U_K2];
    mon_modem[aff3ct::module::mdm::sck::modulate::X_N1]       = encoder[aff3ct::module::enc::sck::encode::X_N];
    channel[aff3ct::module::chn::sck::add_noise::X_N]         = mon_modem[aff3ct::module::mdm::sck::modulate::X_N2]; 
    logger["log::vals"]              = channel[aff3ct::module::chn::sck::add_noise::Y_N];

    monitor[aff3ct::module::mnt::sck::check_errors::V]        = src[spu::module::src::sck::generate::out_data];
    monitor[aff3ct::module::mnt::sck::check_errors::U]        = src[spu::module::src::sck::generate::out_data];

    std::vector<float> sigma(1);
    sigma[0] = 0.01f; // Set the noise level 
    channel[aff3ct::module::chn::sck::add_noise::CP].bind(sigma);
    mon_modem[aff3ct::module::mdm::sck::demodulate::CP].bind(sigma);

    ///////////////////////////////////
    // Sequence creation

    std::vector<spu::runtime::Task*> first = {&src("generate")};
    spu::runtime::Sequence seq(first);

    std::ofstream file("graph_tx_gen.dot");

    seq.export_dot(file);

    for (auto lt : seq.get_tasks_per_types())
        for (auto t : lt)
        {
            t->set_stats(true);
            //t->set_debug(true);
        }  
    
    ///////////////////////////////////
    // Simulation execution
    int nb_cw = 10000;
    for (size_t i = 0; i < sigma_vals.size(); i++){
        // Setting noise values and frozen bits
        sigma[0] = sigma_vals[0];
        //std::cout << sigma[0] << " sigma\n";
        // Sequence execution
        //std::cout << seq.get_n_frames() <<" nframes\n";
        seq.exec();
        /*
        src("generate").exec();
        CRC("build").exec();
        encoder("encode").exec();
        mon_modem("modulate").exec();
        channel("add_noise").exec();
        */
        //std::cout << seq.get_n_frames() <<" nframes\n";
    } 
    spu::tools::Statistics::show(seq.get_tasks_per_types());
    //seq.show_stats();
    tracker.stop();
    tracker.calculate_energy();
    tracker.print_energy();
    //tracker.save_csv("results_polar.csv");
    std::cout << "End !" << std::endl;
    //stats_output_f.close();

}
