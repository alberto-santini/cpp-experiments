#include <algorithm>
#include <chrono>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <random>

#include "pcg_random.hpp" // Random number generator

using uivec = std::vector<unsigned int>;
using uivec_size = uivec::size_type;

auto make_random_vector(uivec_size length) -> uivec {    
    pcg_extras::seed_seq_from<std::random_device> seed_source;

    auto v = uivec(length);
    auto random_engine = pcg32(seed_source);
    auto distribution = std::uniform_int_distribution<unsigned int>(0u, 2u);
    
    for(auto i = uivec_size(); i < length; ++i) {
        v[i] = distribution(random_engine);
    }
    
    return v;
 }

auto sequential_acc(uivec& v) -> long unsigned int {
    return std::accumulate(v.begin(), v.end(), 0l);
}

auto parallel_acc(uivec& v, unsigned int n) -> long unsigned int {
    auto acc = [&v] (auto from, auto to) -> long unsigned int {
        return std::accumulate(&v[from], &v[to], 0l);
    };

    auto l = v.size();
    auto f = std::vector<std::future<long unsigned int>>(n);

    for(auto i = 0u; i < n; ++i) {
        f[i] = std::async(std::launch::async, acc, l * i / n, l * (i + 1) / n);
    }

    return std::accumulate(f.begin(), f.end(), 0l, [] (auto sum, auto& fut) { return sum + fut.get(); });
}

int main(int argc, char** argv) {
    auto lengths = std::vector<uivec_size>{
        100'000u, 1'000'000u, 10'000'000u,
        100'000'000u, 110'000'000u, 120'000'000u, 130'000'000u, 140'000'000u,
        150'000'000u, 160'000'000u, 170'000'000u, 180'000'000u, 190'000'000u,
        200'000'000u, 210'000'000u, 220'000'000u, 230'000'000u, 240'000'000u,
        250'000'000u, 260'000'000u, 270'000'000u, 280'000'000u, 290'000'000u,
        300'000'000u, 310'000'000u, 320'000'000u, 330'000'000u, 340'000'000u,
        350'000'000u, 360'000'000u, 370'000'000u, 380'000'000u, 390'000'000u,
        400'000'000u, 410'000'000u, 420'000'000u, 430'000'000u, 440'000'000u,
        450'000'000u, 460'000'000u, 470'000'000u, 480'000'000u, 490'000'000u,
        500'000'000u, 510'000'000u, 520'000'000u, 530'000'000u, 540'000'000u,
        550'000'000u, 560'000'000u, 570'000'000u, 580'000'000u, 590'000'000u,
        600'000'000u, 610'000'000u, 620'000'000u, 630'000'000u, 640'000'000u,
        650'000'000u, 660'000'000u, 670'000'000u, 680'000'000u, 690'000'000u,
        700'000'000u, 710'000'000u, 720'000'000u, 730'000'000u, 740'000'000u,
        750'000'000u, 760'000'000u, 770'000'000u, 780'000'000u, 790'000'000u,
        800'000'000u, 810'000'000u, 820'000'000u, 830'000'000u, 840'000'000u,
        850'000'000u, 860'000'000u, 870'000'000u, 880'000'000u, 890'000'000u,
     };

    for(auto length : lengths) {
        auto v = make_random_vector(length);
    
        std::cout << (length / 100'000u) << "\t" << std::flush;
    
        auto start_seq = std::chrono::steady_clock::now();
        auto res_seq = sequential_acc(v);
        auto end_seq = std::chrono::steady_clock::now();

        std::cout << std::chrono::duration<double, std::milli>(end_seq - start_seq).count() << "\t" << std::flush;

        auto start_par = std::chrono::steady_clock::now();
        auto res_par = parallel_acc(v, 2u);
        auto end_par = std::chrono::steady_clock::now();

        std::cout << std::chrono::duration<double, std::milli>(end_par - start_par).count() << std::endl;
        
        assert(res_seq == res_par);
    }

    return 0;
}