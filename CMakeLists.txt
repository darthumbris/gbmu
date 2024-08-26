cmake_minimum_required(VERSION 3.5)

project(gbmu LANGUAGES CXX)

set(GLEW_INCLUDE_DIR "/home/$(USER)/.capt/root/usr/include/GL")

option(GBMU_VENDORED "Use vendored libraries" OFF)


if(GBMU_VENDORED)
    add_subdirectory(vendored/sdl EXCLUDE_FROM_ALL)
else()
    find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
	find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
	#find_package(GLEW REQUIRED)
endif()

if(MSVC)
	add_compile_options(/W4)
else()
	add_compile_options(-Wall -Wextra -Wold-style-cast)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

add_executable(gbmu
	src/main.cpp
	src/events.cpp
	src/input/input.cpp
	src/rom/Rom.cpp
	src/rom/RomHeader.cpp
	src/rom/RomOnly.cpp
	src/rom/MCB1.cpp
	src/rom/MCB2.cpp
	src/rom/MCB3.cpp
	src/rom/MCB5.cpp
	src/cpu/Decoder.cpp
	src/cpu/Operand.cpp
	src/cpu/Instruction.cpp
	src/cpu/Flags.cpp
	src/cpu/Cpu.cpp
	src/cpu/MemoryMap.cpp
	src/cpu/PixelProcessingUnit/PixelProcessingUnit.cpp
	src/cpu/PixelProcessingUnit/ppu_dma.cpp
	src/cpu/PixelProcessingUnit/ppu_memory.cpp
	src/cpu/PixelProcessingUnit/ppu_sdl.cpp
	src/cpu/instruction_table.cpp
	src/cpu/Serialization.cpp
	src/cpu/Interruptor.cpp
	src/cpu/ops/add.cpp
	src/cpu/ops/alu.cpp
	src/cpu/ops/call.cpp
	src/cpu/ops/jumps.cpp
	src/cpu/ops/loads.cpp
	src/cpu/ops/misc.cpp
	src/cpu/ops/ret.cpp
	src/cpu/ops/rotate_shift.cpp
	src/cpu/ops/sub.cpp
	src/cpu/AudioProcessingUnit/AudioProcessingUnit.cpp
	src/cpu/AudioProcessingUnit/Blip_Buffer.cpp
	src/cpu/AudioProcessingUnit/Effects_Buffer.cpp
	src/cpu/AudioProcessingUnit/Gb_Apu.cpp
	src/cpu/AudioProcessingUnit/Gb_Oscs.cpp
	src/cpu/AudioProcessingUnit/Multi_Buffer.cpp
)

target_include_directories(gbmu PRIVATE includes)

if(TARGET SDL2::SDL2main)
	target_link_libraries(gbmu PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(gbmu PRIVATE SDL2::SDL2)