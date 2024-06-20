#include <SDL2/SDL.h>
#include <stdio.h>
#include <fstream>
#include "Rom.hpp"
#include "Decoder.hpp"
#include "Cpu.hpp"
#include <cstddef>
#include <iostream>


int main(int argc, char *argv[])
{
    std::string path = argv[1];
    std::string p = "Opcodes.json";
    Dict::Decoder decode = Dict::Decoder(p);

    decode.set_data(path);

    Cpu cpu = Cpu(decode, path);

    while (!cpu.status())
    {
        cpu.tick();
    }
    cpu.close();

    return 0;
}