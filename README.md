# Simulador de Tomasulo em C

> Autores:
> André Resende @AndreMenezes03
> Catarina F. M. Castro @catfmcastro
> Diego Pereira Maia @sn0ttz
> Rafael Peixoto @rafaelpeixoto7559

---

## Introdução

Este projeto implementa um simulador, em C, do Algoritmo de Tomasulo. Dependêndias RAW, WAR e WAW são solucionadas por
meio de

* **Renomeação de Registradores**, para eliminação das dependências falsas (WAR e WAW);
* **Buffer de Reordenação**, para a manutenção da ordem de execução correta do programa;
* **Execução Fora de Ordem**, que permite que as intruções sejam executadas assim que seus operandos estejam prontos,
  aumentando o paralelismo em nível de instrução.
* **Barramento de Dados Comum**, para a passagem rápida de dados.

## Estrutura de Arquivos

| Arquivo         | Função                              |
|-----------------|-------------------------------------|
| `tomasulo.c`    | Código fonte monolítico             |
| `programaX.txt` | Código a ser executado (X = número) |
| `README.md`     | Explicação do simulador             |

## Como Executar

### 1. Preparar o arquivo de entrada

Crie um arquivo ```programaX.txt``` com as instruções no formato:

```
li r1, r0, 5
li r2, r0, 3
add r3, r1, r2
mul r4, r1, r2
sub r5, r3, r4
halt
```

### 2. Compilar e executar

```bash
gcc -o tomasulo tomasulo.c
./tomasulo
```

### 3. Formato das instruções

- **Operações aritméticas**: ```op rD, rS1, rS2```
    - Exemplo: ```add r3, r1, r2``` → r3 = r1 + r2
- **Load immediate**: ```li rD, rS, IMM```
    - Exemplo: ```li r1, r0, 5``` → r1 = r0 + 5
- **Parada**: ```halt```

## Exemplo de Execução

```
----------- Clock Cycle: 0 -----------
Registers: r0=0 r1=0 r2=0 r3=0 r4=0 r5=0 r6=0 r7=0
[Issue] PC=0 -> RS[0], ROB[0], r1 = r0 op r5
[Execute] RS[0] op=4 resultado=5 para ROB[0]
[Commit] r1 = 5 (ROB[0])
----------- Clock Cycle: 1 -----------
Registers: r0=0 r1=5 r2=0 r3=0 r4=0 r5=0 r6=0 r7=0
[Issue] PC=1 -> RS[0], ROB[1], r2 = r0 op r3
[Execute] RS[0] op=4 resultado=3 para ROB[1]
[Commit] r2 = 3 (ROB[1])
```

## Arquitetura do Simulador

### Constantes de Configuração

```c
#define num_instr 10    // Número máximo de instruções
#define rs_size 4       // Número de estações de reserva
#define rob_size 4      // Tamanho do buffer de reordenação
#define reg_size 8      // Número de registradores
```

### Operações Suportadas

- **ADD**: Adição (latência: 2 ciclos)
- **SUB**: Subtração (latência: 2 ciclos)
- **MUL**: Multiplicação (latência: 10 ciclos)
- **DIV**: Divisão (latência: 40 ciclos)
- **LI**: Load immediate (latência: 1 ciclo)
- **HALT**: Parada do programa

## Estruturas de Dados

### Estação de Reserva (```ReservationStation```)

```c
typedef struct {
    Opcode op;          // Operação a ser executada
    int qj, qk;         // IDs das estações que produzem os operandos
    int vj, vk;         // Valores dos operandos (quando disponíveis)
    int dest;           // Entrada do ROB de destino
    bool busy;          // Indica se a estação está ocupada
} ReservationStation;
```

### Buffer de Reordenação (```Rob```)

```c
typedef struct {
    Opcode op;          // Operação
    int dest;           // Registrador de destino
    int valor;          // Resultado da operação
    bool ready;         // Resultado está pronto
    bool busy;          // Entrada está ocupada
} Rob;
```

### Registradores (```Regs```)

```c
typedef struct {
    int values[reg_size];   // Valores dos registradores r0-r7
} Regs;
```

### Controle (```Control```)

```c
typedef struct {
    int pc;             // Program counter
    int rob_ini;        // Ponteiro para início do ROB (head)
    int rob_fim;        // Ponteiro para fim do ROB (tail)
    int rob_ocup;       // Número de entradas ocupadas no ROB
    int clock;          // Ciclo de clock atual
} Control;
```

## Algoritmo de Execução

O simulador executa em **três estágios principais** a cada ciclo de clock:

### 1. Issue (Emissão)

- Busca a próxima instrução do programa
- Verifica disponibilidade de estação de reserva e entrada no ROB
- **Renomeação de registradores**: identifica dependências e mapeia operandos
- Aloca recursos e avança o PC

### 2. Execute (Execução)

- Verifica estações de reserva com operandos prontos (```qj == -1 && qk == -1```)
- Executa a operação aritmética correspondente
- Marca o resultado como pronto no ROB
- Libera a estação de reserva

### 3. Write Result/Commit (Escrita/Confirmação)

- **Write Result**: Propaga resultados prontos para estações dependentes
- **Commit**: Confirma instruções na ordem original (head do ROB)
- Atualiza registradores arquiteturais
- Remove entradas do ROB

## Limitações

1. **Tamanho fixo das estruturas** (estações de reserva, ROB, registradores)
2. **Unidades funcionais infinitas** (não modela contenção de recursos)
3. **Sem hierarquia de memória** (não implementa cache/memória)
