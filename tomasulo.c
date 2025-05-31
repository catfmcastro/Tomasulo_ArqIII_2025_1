#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define num_instr 10
#define rs_size 4
#define rob_size 4
#define reg_size 8

// operações
typedef enum { ADD, SUB, MUL, DIV, LI, HALT } Opcode;

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
  if (strcmp(mnemonic, "li") == 0)
    return LI;
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

// encontra a estação de reserva disponível
  int res_disponivel() {
    for (int i = 0; i < rs_size; i++) {
      if (!res[i].busy)
        return i;
    }
    return -1;
  }

  int latencia(Opcode op) {
  switch(op) {
    case ADD:
    case SUB:
      return 2;     
    case MUL:
      return 10;    
    case DIV:
      return 40;    
    case LI:
      return 1;     
    default:
      return 1;
  }
}

  void issue(){
    Instruction instruction = program[control.pc];
    
    if (instruction.op == HALT) {
      return;
    }
  
    int reservation_index = res_disponivel();
    
    if (is_rob_full()){
      printf("ROB cheio.\n");
      control.pc++;
      return;
    }
  
    int rob_idx = control.rob_fim;
    rob[rob_idx].op = instruction.op;
    rob[rob_idx].dest = instruction.rd;
    rob[rob_idx].ready = false;
    rob[rob_idx].busy = true;

    control.rob_fim = (control.rob_fim + 1) % rob_size;
    control.rob_ocup++;
  
    // 4. Aloca nova entrada na RS
    res[reservation_index].busy = true;
    res[reservation_index].op = instruction.op;
    res[reservation_index].dest = rob_idx;

    if (instruction.op == LI) {
      // LI: valor imediato já está disponível
      res[reservation_index].qj = -1;
      res[reservation_index].vj = instruction.rs2; // rs2 armazena o imediato
      res[reservation_index].qk = -1;
      res[reservation_index].vk = 0;
    } else {
      // 5. Determina vj/qj
      res[reservation_index].qj = -1;
      for (int i = 0; i < rob_size; i++) {
        if (rob[i].busy && rob[i].dest == instruction.rs1 && !rob[i].ready) {
          res[reservation_index].qj = i;
          break;
        }
      }
      if (res[reservation_index].qj == -1)
        res[reservation_index].vj = regs.values[instruction.rs1];

      // 6. Determina vk/qk
      res[reservation_index].qk = -1;
      for (int i = 0; i < rob_size; i++) {
        if (rob[i].busy && rob[i].dest == instruction.rs2 && !rob[i].ready) {
          res[reservation_index].qk = i;
          break;
        }
      }
      if (res[reservation_index].qk == -1)
        res[reservation_index].vk = regs.values[instruction.rs2];
    }

    // 7. Debug
    printf("[Issue] PC=%d -> RS[%d], ROB[%d], r%d = r%d op r%d\n",
           control.pc, reservation_index, rob_idx, instruction.rd, instruction.rs1, instruction.rs2);
  
    // 8. Avança PC
    control.pc++;
  }

void execute() {
    for (int i = 0; i < rs_size; i++) {
        ReservationStation *r = &res[i];
        if (r->busy && r->qj == -1 && r->qk == -1) { // operandos prontos
            int result = 0;
            switch (r->op) {
                case ADD: result = r->vj + r->vk; break;
                case SUB: result = r->vj - r->vk; break;
                case MUL: result = r->vj * r->vk; break;
                case DIV: result = (r->vk ? r->vj / r->vk : 0); break;
                case LI: result = r->vj; break;
                default: break;
            }
            rob[r->dest].valor = result;
            rob[r->dest].ready = true;
            printf("[Execute] RS[%d] op=%d resultado=%d para ROB[%d]\n", i, r->op, result, r->dest);
            r->busy = false; // libera estação de reserva
        }
    }
}

void write_result() {
  for (int i = 0; i < rob_size; i++) {
    if (rob[i].busy && rob[i].ready) {
      // Propaga o valor para as estações de reserva
      for (int j = 0; j < rs_size; j++) {
        if (res[j].busy) {
          if (res[j].qj == i) {
            res[j].vj = rob[i].valor;
            res[j].qj = -1;
          }
          if (res[j].qk == i) {
            res[j].vk = rob[i].valor;
            res[j].qk = -1;
          }
        }
      }
    }
  }

  // Agora tenta commit no head do ROB
  int head = control.rob_ini;
  if (rob[head].busy && rob[head].ready) {
    int reg_dest = rob[head].dest;
    int val = rob[head].valor;

    regs.values[reg_dest] = val;
    printf("[Commit] r%d = %d (ROB[%d])\n", reg_dest, val, head);

    // Limpa entrada no ROB
    rob[head].busy = false;
    rob[head].ready = false;

    control.rob_ini = (control.rob_ini + 1) % rob_size;
    control.rob_ocup--;
  }
}

int main() {
  FILE *fp = fopen("programa2.txt", "r"); // abertura de arquivo
  if (fp == NULL) {
    perror("Erro ao abrir o arquivo");
    return 1;
  }

  char line[100];
  int count = 0;

  // le instruções do arquivo
  while (fgets(line, sizeof(line), fp) && count < num_instr) {
    char mnemonic[10];
    int rd = 0, rs = 0, rt = 0, imm = 0;

    if (sscanf(line, "%s r%d, r%d, r%d", mnemonic, &rd, &rs, &rt) >= 1) {
      Instruction instr;
      instr.op = get_opcode(mnemonic);
      if (instr.op == LI) {
        // LI tem formato: li rD, rS, IMM
        if (sscanf(line, "%*s r%d, r%d, %d", &rd, &rs, &imm) == 3) {
          instr.rd = rd;
          instr.rs1 = rs;
          instr.rs2 = imm; // rs2 armazena o imediato
        } else {
          fprintf(stderr, "Erro ao ler LI: %s\n", line);
          continue;
        }
      } else if (instr.op != HALT) {
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
    int prev_pc = control.pc;
    issue();
    execute();
    write_result();
    control.clock++;
  }

  return 0;
}
