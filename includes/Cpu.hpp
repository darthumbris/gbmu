class Cpu
{
private:
    /*
     * registers (
        AF Accumaltor & Flag register,
        BC general purpose register,
        DE general purpose register,
        HL general purpose register and instructions for iterative code
        SP stack pointer
        PC program counter
        )
     *
     * flag register (z zero flag, h half carry flag, n subtraction flag, c carry flag)
     *
     */
public:
    Cpu(/* args */);
    ~Cpu();
};