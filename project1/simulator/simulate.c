/* LC-2K Instruction-level simulator */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8       /* number of machine registers */
#define MAXLINELENGTH 1000
#define HALT 25165824

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);

int convertNum(int num) {
    /* convert a 16-bit number into a 32-bit Linux integer */
    if (num & (1 << 15)) {
        num -= (1 << 16);
    }
    return (num);
}

int isValidAddress(int address, stateType *state) {
    return address >= 0 && state->numMemory > address;
}

int getOpcode(int num) { return (num >> 22) & 7; }

int getRegA(int num) { return (num >> 19) & 7; }

int getRegB(int num) { return (num >> 16) & 7; }

int getDestReg(int num) { return num & 7; }

int getOffsetField(int num) { return convertNum(num & ((1 << 16) - 1)); }

int main(int argc, char *argv[]) {
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;
    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }
    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }
    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
         state.numMemory++) {
        if (sscanf(line, "%d", state.mem + state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }
    state.pc = 0;
    memset(state.reg, 0, sizeof(state.reg));
    int executed, inst, regA, regB, destReg, offsetField;
    for (executed = 1;; executed++) {
        if (!isValidAddress(state.pc, &state)) exit(1);

        printState(&state);
        inst = state.mem[state.pc];
        int opcode = getOpcode(inst);
        switch (opcode) {
            case 0:  // add
                regA = getRegA(inst);
                regB = getRegB(inst);
                destReg = getDestReg(inst);
                state.reg[destReg] = state.reg[regA] + state.reg[regB];
                break;
            case 1:  // nor
                regA = getRegA(inst);
                regB = getRegB(inst);
                destReg = getDestReg(inst);
                state.reg[destReg] = ~(state.reg[regA] | state.reg[regB]);
                break;
            case 2:  // lw
                regA = getRegA(inst);
                regB = getRegB(inst);
                offsetField = getOffsetField(inst);
                if (isValidAddress(state.reg[regA] + offsetField, &state))
                    state.reg[regB] = state.mem[state.reg[regA] + offsetField];
                else
                    exit(1);
                break;
            case 3:  // sw
                regA = getRegA(inst);
                regB = getRegB(inst);
                offsetField = getOffsetField(inst);
                if (isValidAddress(state.reg[regA] + offsetField, &state))
                    state.mem[state.reg[regA] + offsetField] = state.reg[regB];
                else
                    exit(1);
                break;
            case 4:  // beq
                regA = getRegA(inst);
                regB = getRegB(inst);
                offsetField = getOffsetField(inst);
                if (state.reg[regA] == state.reg[regB]) state.pc += offsetField;
                break;
            case 5:  // jalr
                regA = getRegA(inst);
                regB = getRegB(inst);
                state.reg[regB] = state.pc + 1;
                state.pc = state.reg[regA] - 1;
            default:  // halt, noop
                break;
        }
        state.pc++;
        if (inst == HALT) {
            printf("machine halted\n");
            printf("total of %d instructions executed\n", executed);
            printf("final state of machine:\n");
            break;
        }
    }
    printState(&state);
    return (0);
}

void printState(stateType *statePtr) {
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i = 0; i < statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}