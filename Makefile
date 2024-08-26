NAME = gbmu

CXX := clang++
CXXFLAGS := -std=c++20 -O3 -flto

LINKERFLAGS := -lSDL2 -flto -L /home/$(USER)/.capt/root/usr/lib/x86_64-linux-gnu/ -lSDL2_mixer

SRC_DIR = src
OBJ_DIR = obj
LOG_DIR = test
INC_DIR = includes

SRC =	main.cpp \
        events.cpp \
        input/input.cpp \
        rom/Rom.cpp \
        rom/RomHeader.cpp \
        rom/RomOnly.cpp \
        rom/MCB1.cpp \
        rom/MCB2.cpp \
        rom/MCB3.cpp \
        rom/MCB5.cpp \
        cpu/Decoder.cpp \
        cpu/Operand.cpp \
        cpu/Instruction.cpp \
        cpu/Flags.cpp \
        cpu/Cpu.cpp \
        cpu/MemoryMap.cpp \
        cpu/PixelProcessingUnit/PixelProcessingUnit.cpp \
        cpu/PixelProcessingUnit/ppu_dma.cpp \
        cpu/PixelProcessingUnit/ppu_memory.cpp \
        cpu/PixelProcessingUnit/ppu_sdl.cpp \
        cpu/instruction_table.cpp \
        cpu/Serialization.cpp \
        cpu/Interruptor.cpp \
        cpu/ops/add.cpp \
        cpu/ops/alu.cpp \
        cpu/ops/call.cpp \
        cpu/ops/jumps.cpp \
        cpu/ops/loads.cpp \
        cpu/ops/misc.cpp \
        cpu/ops/ret.cpp \
        cpu/ops/rotate_shift.cpp \
        cpu/ops/sub.cpp \
        cpu/AudioProcessingUnit/AudioProcessingUnit.cpp \
        cpu/AudioProcessingUnit/Blip_Buffer.cpp \
        cpu/AudioProcessingUnit/Effects_Buffer.cpp \
        cpu/AudioProcessingUnit/Gb_Apu.cpp \
        cpu/AudioProcessingUnit/Gb_Oscs.cpp \
        cpu/AudioProcessingUnit/Multi_Buffer.cpp \
        


SRC_EXT = cpp

OBJ := $(addprefix $(OBJ_DIR)/, $(SRC:%.$(SRC_EXT)=%.o))
SRC :=  $(addprefix $(SRC_DIR)/, $(SRC))

INC := -I $(INC_DIR) -I "/home/$(USER)/.capt/root/usr/include/SDL2" -I vendors/gbapu/include

COM_COLOR   = \033[0;33m
OBJ_COLOR   = \033[0;36m
OK_COLOR    = \033[0;32m
ERROR_COLOR = \033[0;31m
WARN_COLOR  = \033[0;33m
NO_COLOR    = \033[m
PRG_COLOR	= \033[0;35m

OK_STRING    = "[OK]"
ERROR_STRING = "[ERROR]"
WARN_STRING  = "[WARNING]"
COM_STRING   = "Compiling"

ifeq ($(DEBUG),1)
	CXXFLAGS += -g3 -D DEBUG_MODE=1
    COM_STRING = "Compiling[DEBUG]"
endif
ifeq ($(DEBUG),2)
	CXXFLAGS += -g3 -fsanitize=address -D DEBUG_MODE=1
    COM_STRING = "Compiling[LEAKS]"
endif

all: $(NAME)

$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(LINKERFLAGS) $(OBJ) -o $(NAME) 2> $@.log; \
        RESULT=$$?; \
        if [ $$RESULT -ne 0 ]; then \
            printf "%-60b%b" "$(COM_COLOR)$(COM_STRING)$(PRG_COLOR) $@" "$(ERROR_COLOR)$(ERROR_STRING)$(NO_COLOR)\n"; \
        elif [ -s $@.log ]; then \
            printf "%-60b%b" "$(COM_COLOR)$(COM_STRING)$(PRG_COLOR) $@" "$(WARN_COLOR)$(WARN_STRING)$(NO_COLOR)\n"; \
        else  \
            printf "%-60b%b" "$(COM_COLOR)$(COM_STRING)$(PRG_COLOR) $(@F)" "$(OK_COLOR)$(OK_STRING)$(NO_COLOR)\n"; \
        fi; \
        cat $@.log; \
        rm -f $@.log; \
        exit $$RESULT

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.$(SRC_EXT) | $(OBJ_DIR) Makefile
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INC) -c -o $@ $< 2> $@.log; \
        RESULT=$$?; \
        if [ $$RESULT -ne 0 ]; then \
            printf "%-60b%b" "$(COM_COLOR)$(COM_STRING)$(OBJ_COLOR) $@" "$(ERROR_COLOR)$(ERROR_STRING)$(NO_COLOR)\n"; \
        elif [ -s $@.log ]; then \
            printf "%-60b%b" "$(COM_COLOR)$(COM_STRING)$(OBJ_COLOR) $@" "$(WARN_COLOR)$(WARN_STRING)$(NO_COLOR)\n"; \
        else  \
            printf "%-60b%b" "$(COM_COLOR)$(COM_STRING)$(OBJ_COLOR) $(@F)" "$(OK_COLOR)$(OK_STRING)$(NO_COLOR)\n"; \
        fi; \
        cat $@.log; \
        rm -f $@.log; \
        exit $$RESULT

clean:
	@printf "%b" "$(ERROR_COLOR)Removing $(OBJ_COLOR)object files\n"
	@rm -rf $(OBJ_DIR) $(LOG_DIR)

fclean:	clean
	@printf "%b" "$(ERROR_COLOR)Removing $(PRG_COLOR)$(NAME)\n"
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re