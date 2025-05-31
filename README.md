# Simulador de Tomasulo em C

> Autores:
> André Resende @AndreMenezes03
> Catarina F. M. Castro @catfmcastro
> Diego Pereira Maia @sn0ttz
> Rafael Peixoto @rafaelpeixoto7559

---

## Introdução
Este projeto implementa um simulador, em C, do Algoritmo de Tomasulo. Dependêndias RAW, WAR e WAW são solucionadas por meio de

* **Renomeação de Registradores**, para eliminação das dependências falsas (WAR e WAW);
* **Buffer de Reordenação**, para a manutenção da ordem de execução correta do programa;
* **Execução Fora de Ordem**, que permite que as intruções sejam executadas assim que seus operandos estejam prontos, aumentando o paralelismo em nível de instrução.
* **Barramento de Dados Comum**, para a passagem rápida de dados.

O simulador recebe uma entrada de um arquivo .txt e executa 

## Como executar
Para o Prompt de Compando do Windows:
1. No diretório raiz, rode o comando abaixo para compilar o projeto.
    ```bash
        gcc tomasulo.c -o tomasulo
    ```
2. Uma vez compilado, execute o .exe com o seguinde comando:
    ```bash
        tomasulo.exe
    ```

Para fazer um programa diferente, configure o programa.txt ou altere o código para o nome do .txt.