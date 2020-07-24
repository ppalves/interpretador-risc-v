/*
 *
 * Nome: Pedro Pupo Alves 
 * RA: 204729
 * 
 * 
*/



#include "montador.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


void declaraErroLexico(int linha){  //printa erros lexicos na stderr
    fprintf(stderr, "ERRO LEXICO: palavra inválida na linha %d!\n", linha);
}

void declaraErroGramatical(int linha){  //printa erros gramaticais na stderr
    fprintf(stderr, "ERRO GRAMATICAL: palavra na linha %d!\n", linha);
}

int verificaRangeDecimal(char* palavra){  //verifica se um numero decimal esta dentro do intervalo proposto (0:1023)
    long int numero;
    numero = strtol(palavra, NULL, 10);
    if(numero > 1023 || numero < 0)
        return 0;
    return 1;
}

int verificaRangeHex(char* palavra){   //verifica se um numero hexadecimal esta dentro do intervalo proposto (0:1023)
    long int numero;
    numero = strtol(palavra, NULL, 16);
    if(numero > 1023 || numero < 0)
        return 0;
    return 1;
}

int verificaHexadecimal(char* palavra){  //verifica se todos os digitos de um numero hexadecimal sao digitos hexadecimais validos
    for (int i = 2; i < strlen(palavra); i++){
        if(!isxdigit(palavra[i])) 
            return 0;
    }
    return 1;
}

int verificaDiretiva(char* palavra){  //verifica se uma possivel diretiva é uma diretiva conhecida e valida
    for (int i = 0; i < strlen(palavra); i++)
        palavra[i] = tolower(palavra[i]);
    if(!(strcmp(palavra, ".set") && strcmp(palavra, ".org") && strcmp(palavra, ".align") && strcmp(palavra, ".wfill") && strcmp(palavra, ".word")))
        return 1;
    return 0;     
}

int verificaInstrucao(char* palavra){  //verifica se uma possivel instrução é um mnemônico valido
    for (int i = 0; i < strlen(palavra); i++)
        palavra[i] = toupper(palavra[i]);
    if(!(strcmp(palavra, "LD") && strcmp(palavra, "LDINV") && strcmp(palavra, "LDABS") && strcmp(palavra, "LDMQ") && strcmp(palavra, "LDMQMX") && strcmp(palavra, "STORE") && strcmp(palavra, "JUMP") && strcmp(palavra, "JGE") && strcmp(palavra, "ADD") && strcmp(palavra, "ADDABS") && strcmp(palavra, "SUB") && strcmp(palavra, "SUBABS") && strcmp(palavra, "MULT") && strcmp(palavra, "DIV") && strcmp(palavra, "LSH") && strcmp(palavra, "RSH") && strcmp(palavra, "STOREND")))
        return 1;
    return 0;     
}

int verificaDecimal(char* palavra){  //verifica se todos os digitos de um numero decimal sao validos
    for (int i = 0; i < strlen(palavra); i++){
        if(!isdigit(palavra[i]))
            return 0;
    }
    return 1;
}

int verificaDefRotulo(char* palavra){  //verifica se todos caracteres associados ao nome de um rotulo sao validos
    for (int i = 0; i < strlen(palavra)-1; i++){
        if(!(isalnum(palavra[i]) || palavra[i]=='_'))
            return 0;
    }
    return 1;
}

int verificaNome(char* palavra){  //verifica se todos caracteres associados a um nome sao validos
    for (int i = 0; i < strlen(palavra); i++){
        if(!(isalnum(palavra[i]) || palavra[i]=='_'))
            return 0;
    }
    return 1;
}


int determinaTipoToken(char* palavra, int linha){
    //Analisamos a estrutura da palavra para determinar seu tipo e procuramos por erros lexicos 

    //Diretivas
    if(palavra[0] == '.'){
        if(verificaDiretiva(palavra))
            return 1001;
        else{
            declaraErroLexico(linha);
            return 0;
        }
    }

    //hexadecimal
    else if (palavra[0] == '0' && (palavra[1] == 'x' || palavra[1] == 'X')){
        if(verificaHexadecimal(palavra))
            return 1003;
        else{
            declaraErroLexico(linha);
            return 0;
        }
    }

    //decimal
    else if (isdigit(palavra[0])){
        if(verificaDecimal(palavra))
            return 1004;
    }
        
    //defRotulo
    else if (palavra[strlen(palavra)-1] == ':' && !isdigit(palavra[0]) ){
        if(verificaDefRotulo(palavra))
            return 1002;
        else{
           declaraErroLexico(linha);
            return 0;
        }
    }

    //instrução
    else if (verificaInstrucao(palavra))
        return 1000;
    else if (verificaNome(palavra))
        return 1005;
    else{
        declaraErroLexico(linha);
        return 0;
    }
    declaraErroLexico(linha);
    return 0;
}

int verificaGramaticaDiretiva(Token* listaLinha, int posicao, int elementosNaFrente){
    //Analisamos possiveis erros gramaticais associados aos diferentes tipos de diretivas
    //Apesar do nome elemetentos na frente, a variavel conta o elemento atual tambem, logo para .word 0x100 temos elementosNaFrente = 2
    if(!(strcmp(listaLinha[posicao].palavra,".set") && strcmp(listaLinha[posicao].palavra,".SET"))){
        if(elementosNaFrente-1 != 2)
            return 0;
        if(listaLinha[posicao + 1].tipo != 1005 || (listaLinha[posicao + 2].tipo != 1003 && listaLinha[posicao + 2].tipo != 1004))
            return 0;
    }
    else if(!(strcmp(listaLinha[posicao].palavra,".org") && strcmp(listaLinha[posicao].palavra,".ORG"))){
        if(elementosNaFrente-1 != 1)
            return 0;
        if(listaLinha[posicao + 1].tipo != 1003 && listaLinha[posicao + 1].tipo != 1004)
            return 0; 
    }
    else if(!(strcmp(listaLinha[posicao].palavra,".align") && strcmp(listaLinha[posicao].palavra,".ALIGN"))){
        if(elementosNaFrente-1 != 1)
            return 0;
        if(listaLinha[posicao + 1].tipo != 1004)
            return 0; 
    }
    else if(!(strcmp(listaLinha[posicao].palavra,".wfill") && strcmp(listaLinha[posicao].palavra,".WFILL"))){
        if(elementosNaFrente-1 != 2)
            return 0;
        if(listaLinha[posicao + 1].tipo != 1004 || (listaLinha[posicao + 2].tipo != 1003 && listaLinha[posicao + 2].tipo != 1004 && listaLinha[posicao + 2].tipo != 1005))
            return 0;
    }
    else if(!(strcmp(listaLinha[posicao].palavra,".word") && strcmp(listaLinha[posicao].palavra,".WORD"))){
        if(elementosNaFrente-1 != 1)
            return 0;
        if(listaLinha[posicao + 1].tipo != 1003 && listaLinha[posicao + 1].tipo != 1004 && listaLinha[posicao + 1].tipo != 1005)
            return 0; 
    }
    return 1;
}

int verificaGramaticaInstrucao(Token* listaLinha, int posicao, int elementosNaFrente){
    //Analisamos possiveis erros gramaticais associados aos diferentes tipos de instruções
    //Apesar do nome elemetentos na frente, a variavel conta o elemento atual tambem, logo para .word 0x100 temos elementosNaFrente = 2
    if(!(strcmp(listaLinha[posicao].palavra,"ldmq") && strcmp(listaLinha[posicao].palavra,"LDMQ") && strcmp(listaLinha[posicao].palavra,"rsh") && strcmp(listaLinha[posicao].palavra,"RSH") && strcmp(listaLinha[posicao].palavra,"lsh") && strcmp(listaLinha[posicao].palavra,"LSH"))){
        if(elementosNaFrente != 1)
            return 0;
    }
    else{
        if(elementosNaFrente != 2)
            return 0;
        if(listaLinha[posicao + 1].tipo != 1003 && listaLinha[posicao + 1].tipo != 1004 && listaLinha[posicao + 1].tipo != 1005)
            return 0;
    }
    return 1;
}


int verificaGramatica(Token* listaLinha, int tamanho1){
    //passamos pelos elementos da linha, vemos seu tipo e analisamos possiveis erros gramaticais associados a esse tipo
    for (int j = 0; j < tamanho1; j++){
        if(listaLinha[j].tipo == 1001){
            if(!verificaGramaticaDiretiva(listaLinha, j, tamanho1 - j))
                return 0;
        }
        else if(listaLinha[j].tipo == 1003){
            if(j==0)
                return 0;
            if(!verificaRangeHex(listaLinha[j].palavra))
                return 0;
        }
        else if(listaLinha[j].tipo == 1004){
            if(j==0)
                return 0;
            if(!verificaRangeDecimal(listaLinha[j].palavra))
                return 0;
        }
        else if(listaLinha[j].tipo == 1002){
            if(j != 0)
                return 0;
            if(tamanho1 != 1){
                if(listaLinha[j+1].tipo == 1002 || listaLinha[j+1].tipo == 1003 || listaLinha[j+1].tipo == 1004 || listaLinha[j+1].tipo == 1005)
                    return 0;
            }  
        }
        else if(listaLinha[j].tipo == 1005){
            if(j == 0)
                return 0;
        }
        else if(listaLinha[j].tipo == 1000){
            if(!verificaGramaticaInstrucao(listaLinha, j, tamanho1 - j))
                return 0;
        }
    }
    return 1;
}



/*
    Essa função é chamada pela main em main.c
    Entrada da função: arquivo de texto lido e seu tamanho
    Retorna:
        * 1 caso haja erro na montagem; (imprima o erro em stderr)
        * 0 caso não haja erro.         (Caso não haja erro, na parte 1, ao retornar desta função, a lista de Tokens (adicionados utilizando a função adicionarToken()) é impressa)
*/
int processarEntrada(char* entrada, unsigned tamanho){ 

    //Primeiro tratamos a entrada, eliminando comentarios e classificando o tipo do Token enquanto checamos por erros lexicos
    int linha = 1;
    unsigned int anterior = 0;
    for (unsigned i = 0; i < tamanho; i++){

        //Se tiver um comentario substituimos tudo depois do # para \0 e partimos para a proxima linha
        if(entrada[i] == '#'){
            while(entrada[i] != '\n'){
                entrada[i] = '\0';
                i++;
            }
            anterior = i + 1;
            linha += 1;
        }

        //Se tiver um espaço em branco substituimos o espaço por \0 e analisamos a string a partir do ponto anterior (proximo caracter apos o espaço em branco anterior)
        else if(entrada[i]==' '){
            if (!(i==anterior)){
                entrada[i] = '\0';
                int tipo = determinaTipoToken(&entrada[anterior], linha);
                if(tipo)
                    adicionarToken(tipo, &entrada[anterior], linha);
                else 
                    return 1;
            }
            anterior= i + 1; //setamos o novo ponto anterior como o proximo caracter, ja que o caracter em questao é um espaco em branco
        }

        //Se tiver uma quebra de linha aplicamos a mesma logica de se tiver espaço, porem aumentamos em 1 o numero da linha
        else if(entrada[i]=='\n'){
            if (!(i==anterior)){
                entrada[i] = '\0';
                int tipo = determinaTipoToken(&entrada[anterior], linha);
                if(tipo)
                    adicionarToken(tipo, &entrada[anterior], linha);
                else 
                    return 1;
            }    
            anterior = i + 1;
            linha += 1;
        }
    }

    //Depois de gerar os tokens vamos verificar possiveis erros gramaticais
    unsigned numeroDeTokens = getNumberOfTokens();
    Token listaLinha[10];   //Lista para armazenar os tokens de uma linha
    unsigned linhaAtual = 1;
    int tamanhoLista = 0;

    for (unsigned i = 0; i < numeroDeTokens; i++){
        Token *tokenAtual = recuperaToken(i);
        if(tokenAtual->linha == linhaAtual){  //caso o token ainda esteja na mesma linha o adicionamos na lista
            listaLinha[tamanhoLista] = *tokenAtual;
            tamanhoLista++;
        }
        
        else{  //caso ele nao esteja na mesma linha chamamos a verificação gramatical na linha q se completou
            if(!verificaGramatica(listaLinha, tamanhoLista)){
                declaraErroGramatical(linhaAtual);
                return 1;
            }
            i--; //diminui em 1 i para analisarmos novamente o token que acabamos de analisar e determinamos q nao pertence a mesma linha
            linhaAtual++; //aumenta a linha atual
            tamanhoLista = 0; //zera o tamanho
        }

        if(i == numeroDeTokens - 1) //caso seja o ultimo token chamamos a analise gramatical na linha em questao
            if(!verificaGramatica(listaLinha, tamanhoLista)){
                declaraErroGramatical(linhaAtual);
                return 1;
            }
    }

    return 0;
}
