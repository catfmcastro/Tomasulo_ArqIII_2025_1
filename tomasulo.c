#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define num_instr 10
#define rs_size 4
#define rob_size 4
#define reg_size 8

// operações
typedef enum { ADD, SUB, MUL, DIV, HALT } Opcode;

// instrução
typedef struct {
  int op;  // operation code
  int rs1; // source register 1
  int rs2; // source register 2
  int rd;  // destination register
} Instruction;
Instruction program[num_instr];

// estação de reserva
typedef struct {
  Opcode op;
  int qj, qk; // estações de reserva que produzem os operandos de origem
  int vj, vk; // valor dos operandos de origem
  int dest;   // destino da instrução
  bool busy;  // instr de reserva ocupada
} ReservationStation;
ReservationStation res[rs_size];

// buffer de reordenação
typedef struct {
  Opcode op;  // operação
  int dest;   // registrador de destino
  int valor;  // valor resultante
  bool ready; // valor está pronto
  bool busy;  // buffer ocupado
} Rob;
Rob rob[rob_size];

// registradores
typedef struct {
  int values[reg_size]; // valores dos registradores
} Regs;
Regs regs = {{0}};

// controle
typedef struct {
  int pc;       // program counter
  int rob_ini;  // ptr para início do rob
  int rob_fim;  // ptr para fim do rob
  int rob_ocup; // num de entradas ocupadas no rob
  int clock;    // ciclo de clock
} Control;
Control control = {0, 0, 0, 0, 0};

// verifica se o buffer de reordenação está cheio
bool is_rob_full() {
    return control.rob_ocup >= rob_size;
}

// verifica se todas as estações de reserva estão ocupadas
bool is_res_full() {
    for (int i = 0; i < rs_size; i++) {
        if (!res[i].busy) {
            return false;
        }
    }
    return true;
}

// adiciona entrada no rob
int add_to_rob(Opcode op, int dst) {
    int index = control.rob_fim;

    rob[index].op = op;
    rob[index].dest = dst;
    rob[index].ready = false;
    rob[index].busy = true;

    control.rob_fim = (control.rob_fim + 1) % rob_size; // move para o próximo índice
    control.rob_ocup++;
    return index; 
}




// conversão de mnemônicos do file
Opcode get_opcode(const char *mnemonic) {
  if (strcmp(mnemonic, "add") == 0)
    return ADD;
  if (strcmp(mnemonic, "mul") == 0)
    return MUL;
  if (strcmp(mnemonic, "sub") == 0)
    return SUB;
  if (strcmp(mnemonic, "div") == 0)
    return DIV;
  if (strcmp(mnemonic, "halt") == 0)
    return HALT;
  fprintf(stderr, "Unknown instruction: %s\n", mnemonic);
  return HALT; // retorna HALT se a instrução não for reconhecida
}

// imprime programa
void print_program(Instruction *program, int n) {
  printf("Instr program[N_INSTR] = {\n\n");

  for (int i = 0; i < n; i++) {
    Instruction instr = program[i];
    const char *op_str[] = {"ADD", "MUL", "SUB", "DIV", "HALT"};

    printf("  { %s, %d, %d, %d }", op_str[instr.op], instr.rd, instr.rs1,
           instr.rs2);

    if (instr.op != HALT) {
      printf(",   // r%d = r%d ", instr.rd, instr.rs1);
      if (instr.op == ADD)
        printf("+ ");
      else if (instr.op == MUL)
        printf("* ");
      else if (instr.op == SUB)
        printf("- ");
      else if (instr.op == DIV)
        printf("/ ");
      printf("r%d", instr.rs2);
    }
    printf("\n");
  }
  printf("};\n\n");
}

int main() {
  FILE *fp = fopen("programa.txt", "r"); // abertura de arquivo
  if (fp == NULL) {
    perror("Erro ao abrir o arquivo");
    return 1;
  }

  char line[100];
  int count = 0;

  // le instruções do arquivo
  while (fgets(line, sizeof(line), fp) && count < num_instr) {
    char mnemonic[10];
    int rd = 0, rs = 0, rt = 0;

    if (sscanf(line, "%s r%d, r%d, r%d", mnemonic, &rd, &rs, &rt) >= 1) {
      Instruction instr;
      instr.op = get_opcode(mnemonic);
      if (instr.op != HALT) {
        instr.rd = rd;
        instr.rs1 = rs;
        instr.rs2 = rt;
      } else {
        instr.rd = instr.rs1 = instr.rs2 = 0;
      }
      program[count++] = instr;

      // interrompe execução do programa
      if (instr.op == HALT) {
        break;
      }
    }
  }

  fclose(fp);

  // imprime instruções lidas
  printf("--------- Programa lido:\n");
  print_program(program, count);

  // execução de tomasulo
  while (program[control.pc].op != HALT) {
    printf("----------- Clock Cycle: %d ----------- \n", control.clock);

    control.clock++;
    control.pc++;
  }

  return 0;
}