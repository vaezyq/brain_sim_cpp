//
// Created by 22126 on 2022/12/6.
//

# include <iostream>
# include <random>
# include "../include/XoshiroCpp.hpp"

int main() {
    using namespace XoshiroCpp;

    const std::uint64_t seed = 12345;

    Xoshiro256PlusPlus rng(seed);

    std::uniform_int_distribution<int> dist(1, 6);

    for (int i = 0; i < 5; ++i) {
        std::cout << dist(rng) << '\n';
    }
}