#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#define MEM_SIZE 256
#define FLAG_ZERO 0x01
#define NOPS 8
#define NUM_REGISTERS 10

uint8_t zero_flag = 0;
bool halted = false;

// Memory is 16-bit, one cell is one instruction or data
uint16_t mem[MEM_SIZE] = {0};

enum reg { R0 = 0, R1, R2, R3, R4, R5, R6, R7, PC, LR };

uint16_t registers[NUM_REGISTERS] = {0};

// Memory read and write functions
static inline uint16_t mr(uint16_t address) { return mem[address]; }
static inline void mw(uint16_t address, uint16_t val) { mem[address] = val; }

// MOV instruction: copy src register to dst register
static inline void MOV(uint16_t instruction) {
    uint8_t dst = (instruction & 0x0F00) >> 8;
    uint8_t src = (instruction & 0x00F0) >> 4;
    registers[dst] = registers[src];
}

// LOAD instruction: load from memory address into dst register
static inline void LOAD(uint16_t instruction) {
    uint8_t dst = (instruction & 0x0F00) >> 8;
    uint8_t addr = instruction & 0x00FF;
    registers[dst] = mem[addr];
}

// STORE instruction: store src register value into memory address
static inline void STORE(uint16_t instruction) {
    uint8_t src = (instruction & 0x0F00) >> 8;
    uint8_t addr = instruction & 0x00FF;
    mem[addr] = registers[src];
}

// ADD instruction: add memory value at address to dst register
static inline void ADD(uint16_t instruction) {
    uint8_t dst = (instruction & 0x0F00) >> 8;
    uint8_t addr = instruction & 0x00FF;
    registers[dst] += mem[addr];
}

// CMP instruction: compare register with memory value, set zero_flag
static inline void CMP(uint16_t instruction) {
    uint8_t reg = (instruction & 0x0F00) >> 8;
    uint8_t addr = instruction & 0x00FF;
    zero_flag = (registers[reg] == mem[addr]);
}

// JE instruction: jump to address if zero_flag is set
static inline void JE(uint16_t instruction) {
    uint8_t addr = instruction & 0x00FF;
    if (zero_flag) {
        registers[PC] = addr;
    } else {
        registers[PC]++; // If not jumping, advance PC to next instruction
    }
}

// JMP instruction: unconditional jump to address
static inline void JMP(uint16_t instruction) {
    uint8_t addr = instruction & 0x00FF;
    registers[PC] = addr;
}

// HALT instruction: stop CPU execution
static inline void HALT(uint16_t instruction) {
    halted = true;
}

typedef void (*instruction_set)(uint16_t instruction);

instruction_set set[NOPS] = {MOV, LOAD, STORE, ADD, JMP, CMP, JE, HALT};

void cpu_run() {
    while (!halted) {
        uint16_t pc = registers[PC];
        if (pc >= MEM_SIZE) {
            printf("PC out of bounds: %u\n", pc);
            halted = true;
            break;
        }

        uint16_t instruction = mem[pc];
        uint8_t opcode = (instruction & 0xF000) >> 12;

        if (opcode >= NOPS) {
            printf("Unknown opcode %u at PC=%u\n", opcode, pc);
            halted = true;
            break;
        }

        if (opcode == 6 || opcode == 4) {
            set[opcode](instruction);
        } else {
            set[opcode](instruction);
            registers[PC] = (registers[PC] + 1) % MEM_SIZE;
        }
    }
}

void init_mem() {
    for (int i = 0; i < MEM_SIZE; i++) {
        mem[i] = 0;
    }
}

int main(void) {
    init_mem();

    registers[R0] = 42;
    registers[R1] = 0;

    mem[0] = 0x0100; // MOV R1, R0

    mem[1] = 0x3103;

    // mem[2] = HALT instruction
    mem[2] = 0x7000;

    mem[3] = 10;

    cpu_run();

    printf("R1 = %d\n", registers[R1]);

    return 0;
}
