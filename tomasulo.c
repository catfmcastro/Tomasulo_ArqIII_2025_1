#include <stdio.h>
#include <stdbool.h>

#define num_instr 6
#define rs_size 4
#define rob_size 4
#define reg_size 8

// operações
typedef enum {
    ADD,
    SUB,
    MUL,
    DIV
} Operation;

// instrução
typedef struct {
    int instr_id;
    int op; // operation code
    int rs1; // source register 1
    int rs2; // source register 2
    int rd; // destination register
} Instruction;

// estação de reserva
typedef struct {
    Operation op;
    int qj, qk; // estações de reserva que produzem os operandos de origem
    int vj, vk; // valor dos operandos de origem
    int dest; // destino da instrução
    bool busy; // instr de reserva ocupada
} ReservationStation;

// buffer de reordenação
typedef struct {
    Operation op; // operação
    int dest; // registrador de destino
    int valor; // valor resultante
    bool ready; // valor está pronto
    bool busy; // buffer ocupado
} Rob;
Rob rob[rob_size];

// registradores
typedef struct {
    int values[reg_size]; // valores dos registradores
} Regs;
Regs regs = {{0}};


// controle
typedef struct {
    int pc; // program counter
    int rob_ini; // ptr para início do rob
    int rob_fim; // ptr para fim do rob
    int rob_ocup; // num de entradas ocupadas no rob
    int clock; // ciclo de clock
} Control;
Control control = {0, 0, 0, 0, 0};

// fazer unidade funcional?

