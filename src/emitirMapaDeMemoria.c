/* 
 *
 * Nome: Pedro Pupo Alves
 * RA: 204729
 *  
 */



#include "montador.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


typedef struct Endereco{       //struct para armazenar o endereço de uma instrução ou diretiva
    int endereco;
    int lado; // 0 = esq; 1 = direita
} Endereco;

typedef struct DefRotulos{     //struct para armazenar as definições de rotulos e seus endereços
    char* palavra;
    Endereco endereco;
} DefRotulos;

typedef struct sets{           //struct para armazenar os valores associados aos sym do .set
    char* sym;
    char* valor;
} valoresSets;


void transformaMinusculo(char* string){     //função para passar uma string para lowercase
    int tamanho = strlen(string);
    for (int i = 0; i < tamanho; i++){
        string[i] = tolower(string[i]);
    }
}

void atualizaEnderecoInstrucao(Endereco *endereco){     //função para atualizar o endereço de uma instrução
    if(endereco->lado == 0)
        endereco->lado = 1;
    else{
        endereco->lado = 0;
        endereco->endereco += 1;
    }
}
  
void contaRotulosESets(unsigned *numeroRotulos, unsigned *numeroSets, unsigned numeroTokens){    //função para contar o numero de def rotulos e sets q temos no programa
    for (unsigned i = 0; i < numeroTokens; i++){
        Token *token = recuperaToken(i);
        if(token->tipo == 1002)
            (*numeroRotulos)++;
        else if(token->tipo == 1001 && !(strcmp(token->palavra, ".set") && strcmp(token->palavra, ".SET")))
            (*numeroSets)++;   
    }
}


int comparaRotulo(char *palavra, DefRotulos listaDefRotulo[], int numeroRotulos){    //função para comparar se um rotulo esta na lista de rotulos
    for (int j = 0; j < numeroRotulos; j++){
        listaDefRotulo[j].palavra[strlen(listaDefRotulo[j].palavra)-1] = '\0';
        transformaMinusculo(listaDefRotulo[j].palavra);
        if(!strcmp(listaDefRotulo[j].palavra, palavra)){
            listaDefRotulo[j].palavra[strlen(listaDefRotulo[j].palavra)] = ':';
            return 1;
        }
        listaDefRotulo[j].palavra[strlen(listaDefRotulo[j].palavra)] = ':';
    }
    return 0;
}

int comparaSets(char *palavra, valoresSets listaDefSets[], int numeroSets){   //função para comparar se um sym ja foi definido por um .set
    for (int k = 0; k < numeroSets; k++){
        transformaMinusculo(listaDefSets[k].sym);
        if(!strcmp(listaDefSets[k].sym, palavra)){
            return 1;
        }
    }
    return 0;
}

Endereco* substituiRotulo(char *palavra, DefRotulos listaDefRotulo[], int numeroRotulos){     //função para substituir um rotulo por seu endereço
    for (int j = 0; j < numeroRotulos; j++){
        listaDefRotulo[j].palavra[strlen(listaDefRotulo[j].palavra)-1] = '\0';
        transformaMinusculo(listaDefRotulo[j].palavra);
        if(!strcmp(listaDefRotulo[j].palavra, palavra)){
            listaDefRotulo[j].palavra[strlen(listaDefRotulo[j].palavra)] = ':';
            return &(listaDefRotulo[j].endereco);
        }
        listaDefRotulo[j].palavra[strlen(listaDefRotulo[j].palavra)] = ':';
    }
    return NULL;
}

char* substituiSets(char *palavra, valoresSets listaDefSets[], int numeroSets){      //função para substituir um sym por seu valor
    for (int k = 0; k < numeroSets; k++){
        transformaMinusculo(listaDefSets[k].sym);
        if(!strcmp(listaDefSets[k].sym, palavra)){
            return listaDefSets[k].valor;
        }
    }
    return NULL;
}

int emitirMapaDeMemoria(){

/* Função principal, retorna 1 caso encontre erro, 0 caso contrario
 * A função faz basicamente 3 buscas  - Primeira para buscar definições de rotulos e gravar cada rotulo com seu endereço associado em um vetor
 *                                    - Segunda para verificar se todos os rotulos usados são declarados, enquando grava em um vetor cada sym do .set e seu valor 
 *                                      associado bem como verifica se sym usados foram setados previamente. Caso encontre algum erro declara o erro e para a execução
 *                                    - Terceira analisa cada instrução e/ou diretiva e printa o mapa de memoria 
 */

    unsigned numeroRotulos = 0;
    unsigned numeroSets = 0;
    unsigned numeroTokens = getNumberOfTokens();

    contaRotulosESets(&numeroRotulos, &numeroSets, numeroTokens);

    Endereco enderecoAtual;         //inicializa o endereço em 0x000
    enderecoAtual.endereco = 0;
    enderecoAtual.lado = 0;


    DefRotulos listaDefRotulo[numeroRotulos];
    valoresSets listaDefSets[numeroSets];



    numeroRotulos = 0;
    numeroSets = 0;



    /*--------------------------------------------------------PRIMEIRA BUSCA---------------------------------------------------------------*/


    for (unsigned i = 0; i < numeroTokens; i++){ 
        /*Primeira busca, procuramos por definições de rotulos e seu endereço associado*/
        Token *tokenAtual = recuperaToken(i);

        if(tokenAtual->tipo == 1002){   //caso o token atual seja a definição de um rotulo, armazenamos essa definição no array "listaDefRotulo"
            DefRotulos rotulo;
            rotulo.endereco = enderecoAtual;
            rotulo.palavra = tokenAtual->palavra;
            listaDefRotulo[numeroRotulos] = rotulo;
            numeroRotulos++;    
        }

        //Atualiza o endereço para armazenar corretamente o rotulo
        else if(tokenAtual->tipo == 1001){
            if(!(strcmp(tokenAtual->palavra, ".org") && strcmp(tokenAtual->palavra, ".ORG"))){
                Token *proximoToken1 = recuperaToken(i+1);
                int numero;
                if(proximoToken1->tipo == 1003)    
                    numero = strtol(proximoToken1->palavra, NULL, 16);
                
                else
                    numero = strtol(proximoToken1->palavra, NULL, 10);
            
                enderecoAtual.endereco = numero;
                enderecoAtual.lado = 0;
                i++;
            }

            else if(!(strcmp(tokenAtual->palavra, ".align") && strcmp(tokenAtual->palavra, ".ALIGN"))){
                Token *proximoToken1 = recuperaToken(i+1);
                int numero;
                numero = strtol(proximoToken1->palavra, NULL, 10);

                for (int j = 0; j <= numero; j++){
                    if((enderecoAtual.endereco % numero == 0) && enderecoAtual.lado == 0)
                        break;                   
                    enderecoAtual.lado = 0;
                    enderecoAtual.endereco += 1;
                }
                i++;
            }

            else if(!(strcmp(tokenAtual->palavra, ".word") && strcmp(tokenAtual->palavra, ".WORD"))){
                enderecoAtual.lado = 0;
                enderecoAtual.endereco += 1;
                i++;
            }

            else if(!(strcmp(tokenAtual->palavra, ".wfill") && strcmp(tokenAtual->palavra, ".WFILL"))){
                Token *proximoToken1 = recuperaToken(i+1);
                int numero;
                numero = strtol(proximoToken1->palavra, NULL, 10);
                enderecoAtual.lado = 0;
                enderecoAtual.endereco = numero;
                i += 2;
            }
        }

        else if(tokenAtual->tipo == 1000){
            if(enderecoAtual.lado == 1){
                enderecoAtual.endereco += 1;
                enderecoAtual.lado = 0;
            }
            else 
                enderecoAtual.lado = 1;
        }

    }

   
    


    /*--------------------------------------------------------SEGUNDA BUSCA---------------------------------------------------------------*/



    enderecoAtual.endereco = 0;  //zeramos a posição do endereço
    enderecoAtual.lado = 0;




    for (unsigned i = 0; i < numeroTokens; i++){
        Token *tokenAtual = recuperaToken(i);

        //caso o token seja um .set, armazenamos seu sym e valor em um vetor
        //vale observar que o .set deve ser declarado antes de seu uso, diferente da diretiva
        //por isso que temos q procurar por .sets na segunda busca, na qual ja buscamos por erros
        if(tokenAtual->tipo == 1001 && !(strcmp(tokenAtual->palavra, ".set") && strcmp(tokenAtual->palavra, ".SET"))){
            valoresSets novoSet;
            Token *proximoToken1 = recuperaToken(i+1);
            Token *proximoToken2 = recuperaToken(i+2);
            novoSet.sym = proximoToken1->palavra;
            novoSet.valor = proximoToken2->palavra;
            listaDefSets[numeroSets] = novoSet;
            numeroSets++;
            i = i + 2;
        }

        
        
        

        else if(tokenAtual->tipo == 1005){
            //caso o token seja um nome vemos se esse nome ja foi declarado anteriormente, caso n tenha sido, indicamos o erro e paramos a execução do programa        
            transformaMinusculo(tokenAtual->palavra);
            if(!(comparaRotulo(tokenAtual->palavra, listaDefRotulo, numeroRotulos) || comparaSets(tokenAtual->palavra, listaDefSets, numeroSets))){
                fprintf(stderr, "ERRO: Rótulo ou símbolo usado mas não definido: %s\n", tokenAtual->palavra);
                return 1;
            }
            
        }

    }



    /*--------------------------------------------------------TERCEIRA BUSCA---------------------------------------------------------------*/


    for (unsigned i = 0; i < numeroTokens; i++){
    
    //Na terceira busca analisamos token a token, e ja printamos seu equivalente enquando buscamos o token
    //Caso encontremos um nome buscamos seu valor ou endereço correspondente nos arrays listaDefRotulos e listaDefSets

        Token *tokenAtual = recuperaToken(i);
        transformaMinusculo(tokenAtual->palavra);


/*-----------------------------------------------------------------INSTRUÇÕES-----------------------------------------------------------------------*/

        if(tokenAtual->tipo == 1000){
            if(!strcmp(tokenAtual->palavra, "ld")){

                if(enderecoAtual.lado == 0)
                    printf("%03X ", enderecoAtual.endereco);

                printf("01 ");
                Token *proximoToken1 = recuperaToken(i + 1);
                transformaMinusculo(proximoToken1->palavra);
                if(proximoToken1->tipo == 1005){
                    Endereco *ptrEndereco = substituiRotulo(proximoToken1->palavra, listaDefRotulo, numeroRotulos);
                    
                    if(!(ptrEndereco == NULL)){
                        Endereco enderecoRotulo; 
                        enderecoRotulo = *ptrEndereco;
                        
                        if(enderecoAtual.lado == 0)
                            printf("%03X ", enderecoRotulo.endereco);
                        else
                            printf("%03X\n", enderecoRotulo.endereco);;
                    }
                    else{
                        char *valor = substituiSets(proximoToken1->palavra, listaDefSets, numeroSets);
                        if(valor != NULL){
                            if (valor[0] == '0' && (valor[1] == 'x' || valor[1] == 'X')){
                                long int numero = strtol(valor, NULL, 16);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                            else{
                                long int numero = strtol(valor, NULL, 10);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                        } 
                    }
                }

                else if(proximoToken1->tipo == 1003){
                    long int numero = strtol(proximoToken1->palavra, NULL, 16);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                else if(proximoToken1->tipo == 1004){
                    long int numero = strtol(proximoToken1->palavra, NULL, 10);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                atualizaEnderecoInstrucao(&enderecoAtual);
                i++;
            }

            else if(!strcmp(tokenAtual->palavra, "ldinv")){
                if(enderecoAtual.lado == 0)
                    printf("%03X ", enderecoAtual.endereco);
                    
                printf("02 ");
                Token *proximoToken1 = recuperaToken(i + 1);
                transformaMinusculo(proximoToken1->palavra);
                if(proximoToken1->tipo == 1005){
                    Endereco *ptrEndereco = substituiRotulo(proximoToken1->palavra, listaDefRotulo, numeroRotulos);
                    
                    if(!(ptrEndereco == NULL)){
                        Endereco enderecoRotulo; 
                        enderecoRotulo = *ptrEndereco;
                        if(enderecoAtual.lado == 0)
                            printf("%03X ", enderecoRotulo.endereco);
                        else
                            printf("%03X\n", enderecoRotulo.endereco);;
                    }
                    else{
                        char *valor = substituiSets(proximoToken1->palavra, listaDefSets, numeroSets);
                        if(valor != NULL){
                            if (valor[0] == '0' && (valor[1] == 'x' || valor[1] == 'X')){
                                long int numero = strtol(valor, NULL, 16);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                            else{
                                long int numero = strtol(valor, NULL, 10);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                        } 
                    }
                }

                else if(proximoToken1->tipo == 1003){
                    long int numero = strtol(proximoToken1->palavra, NULL, 16);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                else if(proximoToken1->tipo == 1004){
                    long int numero = strtol(proximoToken1->palavra, NULL, 10);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                atualizaEnderecoInstrucao(&enderecoAtual);
                i++;
            }

            else if(!strcmp(tokenAtual->palavra, "ldabs")){

                if(enderecoAtual.lado == 0)
                    printf("%03X ", enderecoAtual.endereco);
                    
                printf("03 ");
                Token *proximoToken1 = recuperaToken(i + 1);
                transformaMinusculo(proximoToken1->palavra);
                if(proximoToken1->tipo == 1005){
                    Endereco *ptrEndereco = substituiRotulo(proximoToken1->palavra, listaDefRotulo, numeroRotulos);
                    
                    if(!(ptrEndereco == NULL)){
                        Endereco enderecoRotulo; 
                        enderecoRotulo = *ptrEndereco;
                        if(enderecoAtual.lado == 0)
                            printf("%03X ", enderecoRotulo.endereco);
                        else
                            printf("%03X\n", enderecoRotulo.endereco);;
                    }
                    else{
                        char *valor = substituiSets(proximoToken1->palavra, listaDefSets, numeroSets);
                        if(valor != NULL){
                            if (valor[0] == '0' && (valor[1] == 'x' || valor[1] == 'X')){
                                long int numero = strtol(valor, NULL, 16);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                            else{
                                long int numero = strtol(valor, NULL, 10);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                        } 
                    }
                }

                else if(proximoToken1->tipo == 1003){
                    long int numero = strtol(proximoToken1->palavra, NULL, 16);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                else if(proximoToken1->tipo == 1004){
                    long int numero = strtol(proximoToken1->palavra, NULL, 10);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                atualizaEnderecoInstrucao(&enderecoAtual);
                i++;

            }

            else if(!strcmp(tokenAtual->palavra, "ldmq")){

                if(enderecoAtual.lado == 0){
                    printf("%03X ", enderecoAtual.endereco);
                    printf("0A 000 ");
                }
                    
                printf("0A 000\n");  

                atualizaEnderecoInstrucao(&enderecoAtual);   
            }

            else if(!strcmp(tokenAtual->palavra, "ldmqmx")){

                if(enderecoAtual.lado == 0)
                    printf("%03X ", enderecoAtual.endereco);
                    
                printf("09 ");
                Token *proximoToken1 = recuperaToken(i + 1);
                transformaMinusculo(proximoToken1->palavra);
                if(proximoToken1->tipo == 1005){
                    Endereco *ptrEndereco = substituiRotulo(proximoToken1->palavra, listaDefRotulo, numeroRotulos);
                    if(!(ptrEndereco == NULL)){
                        Endereco enderecoRotulo; 
                        enderecoRotulo = *ptrEndereco;
                        if(enderecoAtual.lado == 0)
                            printf("%03X ", enderecoRotulo.endereco);
                        else
                            printf("%03X\n", enderecoRotulo.endereco);;
                    }
                    else{
                        char *valor = substituiSets(proximoToken1->palavra, listaDefSets, numeroSets);
                        if(valor != NULL){
                            if (valor[0] == '0' && (valor[1] == 'x' || valor[1] == 'X')){
                                long int numero = strtol(valor, NULL, 16);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                            else{
                                long int numero = strtol(valor, NULL, 10);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                        } 
                    }
                }

                else if(proximoToken1->tipo == 1003){
                    long int numero = strtol(proximoToken1->palavra, NULL, 16);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                else if(proximoToken1->tipo == 1004){
                    long int numero = strtol(proximoToken1->palavra, NULL, 10);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                atualizaEnderecoInstrucao(&enderecoAtual);
                i++;
            }

            else if(!strcmp(tokenAtual->palavra, "store")){

                if(enderecoAtual.lado == 0)
                    printf("%03X ", enderecoAtual.endereco);
                    
                printf("21 ");
                Token *proximoToken1 = recuperaToken(i + 1);
                transformaMinusculo(proximoToken1->palavra);
                if(proximoToken1->tipo == 1005){
                    Endereco *ptrEndereco = substituiRotulo(proximoToken1->palavra, listaDefRotulo, numeroRotulos);
                    if(!(ptrEndereco == NULL)){
                        Endereco enderecoRotulo; 
                        enderecoRotulo = *ptrEndereco;
                        if(enderecoAtual.lado == 0)
                            printf("%03X ", enderecoRotulo.endereco);
                        else
                            printf("%03X\n", enderecoRotulo.endereco);;
                    }
                    else{
                        char *valor = substituiSets(proximoToken1->palavra, listaDefSets, numeroSets);
                        if(valor != NULL){
                            if (valor[0] == '0' && (valor[1] == 'x' || valor[1] == 'X')){
                                long int numero = strtol(valor, NULL, 16);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                            else{
                                long int numero = strtol(valor, NULL, 10);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                        } 
                    }
                }

                else if(proximoToken1->tipo == 1003){
                    long int numero = strtol(proximoToken1->palavra, NULL, 16);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                else if(proximoToken1->tipo == 1004){
                    long int numero = strtol(proximoToken1->palavra, NULL, 10);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                atualizaEnderecoInstrucao(&enderecoAtual);
                i++;
            }

            else if(!strcmp(tokenAtual->palavra, "jump")){

                if(enderecoAtual.lado == 0)
                    printf("%03X ", enderecoAtual.endereco);
                    
                Token *proximoToken1 = recuperaToken(i + 1);
                transformaMinusculo(proximoToken1->palavra);
                if(proximoToken1->tipo == 1005){
                    Endereco *ptrEndereco = substituiRotulo(proximoToken1->palavra, listaDefRotulo, numeroRotulos);
                    if(!(ptrEndereco == NULL)){
                        Endereco enderecoRotulo; 
                        enderecoRotulo = *ptrEndereco;
                        if(enderecoRotulo.lado == 0)
                            printf("0D ");
                        else if(enderecoRotulo.lado == 1)
                            printf("0E ");
                        if(enderecoAtual.lado == 0)
                            printf("%03X ", enderecoRotulo.endereco);
                        else
                            printf("%03X\n", enderecoRotulo.endereco);;
                    }
                    else{
                        char *valor = substituiSets(proximoToken1->palavra, listaDefSets, numeroSets);
                        if(valor != NULL){
                            if (valor[0] == '0' && (valor[1] == 'x' || valor[1] == 'X')){
                                long int numero = strtol(valor, NULL, 16);
                                printf("0D ");
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                            else{
                                long int numero = strtol(valor, NULL, 10);
                                printf("0D ");
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                        } 
                    }
                }

                else if(proximoToken1->tipo == 1003){
                    printf("0D ");
                    long int numero = strtol(proximoToken1->palavra, NULL, 16);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                else if(proximoToken1->tipo == 1004){
                    printf("0D ");
                    long int numero = strtol(proximoToken1->palavra, NULL, 10);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                atualizaEnderecoInstrucao(&enderecoAtual);
                i++;
            }

            else if(!strcmp(tokenAtual->palavra, "jge")){

                if(enderecoAtual.lado == 0)
                    printf("%03X ", enderecoAtual.endereco);
                    
                Token *proximoToken1 = recuperaToken(i + 1);
                transformaMinusculo(proximoToken1->palavra);
                if(proximoToken1->tipo == 1005){
                    Endereco *ptrEndereco = substituiRotulo(proximoToken1->palavra, listaDefRotulo, numeroRotulos);
                    
                    if(!(ptrEndereco == NULL)){
                        Endereco enderecoRotulo; 
                        enderecoRotulo = *ptrEndereco;
                        if(enderecoRotulo.lado == 0)
                            printf("0F ");
                        else if(enderecoRotulo.lado == 1)
                            printf("10 ");
                        if(enderecoAtual.lado == 0)
                            printf("%03X ", enderecoRotulo.endereco);
                        else
                            printf("%03X\n", enderecoRotulo.endereco);;
                    }
                    else{
                        char *valor = substituiSets(proximoToken1->palavra, listaDefSets, numeroSets);
                        if(valor != NULL){
                            if (valor[0] == '0' && (valor[1] == 'x' || valor[1] == 'X')){
                                long int numero = strtol(valor, NULL, 16);
                                printf("0F ");
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                            else{
                                long int numero = strtol(valor, NULL, 10);
                                printf("0F ");
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                        } 
                    }
                }

                else if(proximoToken1->tipo == 1003){
                    printf("0F ");
                    long int numero = strtol(proximoToken1->palavra, NULL, 16);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                else if(proximoToken1->tipo == 1004){
                    printf("0F ");
                    long int numero = strtol(proximoToken1->palavra, NULL, 10);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                atualizaEnderecoInstrucao(&enderecoAtual);
                i++;
            }


            else if(!strcmp(tokenAtual->palavra, "add")){

                if(enderecoAtual.lado == 0)
                    printf("%03X ", enderecoAtual.endereco);
                    
                printf("05 ");
                Token *proximoToken1 = recuperaToken(i + 1);
                transformaMinusculo(proximoToken1->palavra);
                if(proximoToken1->tipo == 1005){
                    Endereco *ptrEndereco = substituiRotulo(proximoToken1->palavra, listaDefRotulo, numeroRotulos);
                    
                    if(!(ptrEndereco == NULL)){
                        Endereco enderecoRotulo; 
                        enderecoRotulo = *ptrEndereco;
                        if(enderecoAtual.lado == 0)
                            printf("%03X ", enderecoRotulo.endereco);
                        else
                            printf("%03X\n", enderecoRotulo.endereco);;
                    }
                    else{
                        char *valor = substituiSets(proximoToken1->palavra, listaDefSets, numeroSets);
                        if(valor != NULL){
                            if (valor[0] == '0' && (valor[1] == 'x' || valor[1] == 'X')){
                                long int numero = strtol(valor, NULL, 16);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                            else{
                                long int numero = strtol(valor, NULL, 10);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                        } 
                    }
                }

                else if(proximoToken1->tipo == 1003){
                    long int numero = strtol(proximoToken1->palavra, NULL, 16);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                else if(proximoToken1->tipo == 1004){
                    long int numero = strtol(proximoToken1->palavra, NULL, 10);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                atualizaEnderecoInstrucao(&enderecoAtual);
                i++;
            }

            else if(!strcmp(tokenAtual->palavra, "addabs")){

                if(enderecoAtual.lado == 0)
                    printf("%03X ", enderecoAtual.endereco);
                    
                printf("07 ");
                Token *proximoToken1 = recuperaToken(i + 1);
                transformaMinusculo(proximoToken1->palavra);
                if(proximoToken1->tipo == 1005){
                    Endereco *ptrEndereco = substituiRotulo(proximoToken1->palavra, listaDefRotulo, numeroRotulos);
                    
                    if(!(ptrEndereco == NULL)){
                        Endereco enderecoRotulo; 
                        enderecoRotulo = *ptrEndereco;
                        if(enderecoAtual.lado == 0)
                            printf("%03X ", enderecoRotulo.endereco);
                        else
                            printf("%03X\n", enderecoRotulo.endereco);;
                    }
                    else{
                        char *valor = substituiSets(proximoToken1->palavra, listaDefSets, numeroSets);
                        if(valor != NULL){
                            if (valor[0] == '0' && (valor[1] == 'x' || valor[1] == 'X')){
                                long int numero = strtol(valor, NULL, 16);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                            else{
                                long int numero = strtol(valor, NULL, 10);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                        } 
                    }
                }

                else if(proximoToken1->tipo == 1003){
                    long int numero = strtol(proximoToken1->palavra, NULL, 16);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                else if(proximoToken1->tipo == 1004){
                    long int numero = strtol(proximoToken1->palavra, NULL, 10);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                atualizaEnderecoInstrucao(&enderecoAtual);
                i++;
            }

            else if(!strcmp(tokenAtual->palavra, "sub")){

                if(enderecoAtual.lado == 0)
                    printf("%03X ", enderecoAtual.endereco);
                    
                printf("06 ");
                Token *proximoToken1 = recuperaToken(i + 1);
                transformaMinusculo(proximoToken1->palavra);
                if(proximoToken1->tipo == 1005){
                    Endereco *ptrEndereco = substituiRotulo(proximoToken1->palavra, listaDefRotulo, numeroRotulos);
                    
                    if(!(ptrEndereco == NULL)){
                        Endereco enderecoRotulo; 
                        enderecoRotulo = *ptrEndereco;
                        if(enderecoAtual.lado == 0)
                            printf("%03X ", enderecoRotulo.endereco);
                        else
                            printf("%03X\n", enderecoRotulo.endereco);;
                    }
                    else{
                        char *valor = substituiSets(proximoToken1->palavra, listaDefSets, numeroSets);
                        if(valor != NULL){
                            if (valor[0] == '0' && (valor[1] == 'x' || valor[1] == 'X')){
                                long int numero = strtol(valor, NULL, 16);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                            else{
                                long int numero = strtol(valor, NULL, 10);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                        } 
                    }
                }

                else if(proximoToken1->tipo == 1003){
                    long int numero = strtol(proximoToken1->palavra, NULL, 16);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                else if(proximoToken1->tipo == 1004){
                    long int numero = strtol(proximoToken1->palavra, NULL, 10);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                atualizaEnderecoInstrucao(&enderecoAtual);
                i++;
            }

            else if(!strcmp(tokenAtual->palavra, "subabs")){

                if(enderecoAtual.lado == 0)
                    printf("%03X ", enderecoAtual.endereco);
                    
                printf("08 ");
                Token *proximoToken1 = recuperaToken(i + 1);
                transformaMinusculo(proximoToken1->palavra);
                if(proximoToken1->tipo == 1005){
                    Endereco *ptrEndereco = substituiRotulo(proximoToken1->palavra, listaDefRotulo, numeroRotulos);
                    
                    if(!(ptrEndereco == NULL)){
                        Endereco enderecoRotulo; 
                        enderecoRotulo = *ptrEndereco;
                        if(enderecoAtual.lado == 0)
                            printf("%03X ", enderecoRotulo.endereco);
                        else
                            printf("%03X\n", enderecoRotulo.endereco);;
                    }
                    else{
                        char *valor = substituiSets(proximoToken1->palavra, listaDefSets, numeroSets);
                        if(valor != NULL){
                            if (valor[0] == '0' && (valor[1] == 'x' || valor[1] == 'X')){
                                long int numero = strtol(valor, NULL, 16);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                            else{
                                long int numero = strtol(valor, NULL, 10);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                        } 
                    }
                }

                else if(proximoToken1->tipo == 1003){
                    long int numero = strtol(proximoToken1->palavra, NULL, 16);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                else if(proximoToken1->tipo == 1004){
                    long int numero = strtol(proximoToken1->palavra, NULL, 10);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                atualizaEnderecoInstrucao(&enderecoAtual);
                i++;
            }

            else if(!strcmp(tokenAtual->palavra, "mult")){

                if(enderecoAtual.lado == 0)
                    printf("%03X ", enderecoAtual.endereco);
                    
                printf("0B ");
                Token *proximoToken1 = recuperaToken(i + 1);
                transformaMinusculo(proximoToken1->palavra);
                if(proximoToken1->tipo == 1005){
                    Endereco *ptrEndereco = substituiRotulo(proximoToken1->palavra, listaDefRotulo, numeroRotulos);
                    
                    if(!(ptrEndereco == NULL)){
                        Endereco enderecoRotulo; 
                        enderecoRotulo = *ptrEndereco;
                        if(enderecoAtual.lado == 0)
                            printf("%03X ", enderecoRotulo.endereco);
                        else
                            printf("%03X\n", enderecoRotulo.endereco);;
                    }
                    else{
                        char *valor = substituiSets(proximoToken1->palavra, listaDefSets, numeroSets);
                        if(valor != NULL){
                            if (valor[0] == '0' && (valor[1] == 'x' || valor[1] == 'X')){
                                long int numero = strtol(valor, NULL, 16);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                            else{
                                long int numero = strtol(valor, NULL, 10);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                        } 
                    }
                }

                else if(proximoToken1->tipo == 1003){
                    long int numero = strtol(proximoToken1->palavra, NULL, 16);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                else if(proximoToken1->tipo == 1004){
                    long int numero = strtol(proximoToken1->palavra, NULL, 10);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                atualizaEnderecoInstrucao(&enderecoAtual);
                i++;
            }

            else if(!strcmp(tokenAtual->palavra, "div")){

                if(enderecoAtual.lado == 0)
                    printf("%03X ", enderecoAtual.endereco);
                    
                printf("0C ");
                Token *proximoToken1 = recuperaToken(i + 1);
                transformaMinusculo(proximoToken1->palavra);
                if(proximoToken1->tipo == 1005){
                    Endereco *ptrEndereco = substituiRotulo(proximoToken1->palavra, listaDefRotulo, numeroRotulos);
                    
                    if(!(ptrEndereco == NULL)){
                        Endereco enderecoRotulo; 
                        enderecoRotulo = *ptrEndereco;
                        if(enderecoAtual.lado == 0)
                            printf("%03X ", enderecoRotulo.endereco);
                        else
                            printf("%03X\n", enderecoRotulo.endereco);;
                    }
                    else{
                        char *valor = substituiSets(proximoToken1->palavra, listaDefSets, numeroSets);
                        if(valor != NULL){
                            if (valor[0] == '0' && (valor[1] == 'x' || valor[1] == 'X')){
                                long int numero = strtol(valor, NULL, 16);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                            else{
                                long int numero = strtol(valor, NULL, 10);
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                        } 
                    }
                }

                else if(proximoToken1->tipo == 1003){
                    long int numero = strtol(proximoToken1->palavra, NULL, 16);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                else if(proximoToken1->tipo == 1004){
                    long int numero = strtol(proximoToken1->palavra, NULL, 10);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                atualizaEnderecoInstrucao(&enderecoAtual);
                i++;
            }

            else if(!strcmp(tokenAtual->palavra, "lsh")){

                if(enderecoAtual.lado == 0){
                    printf("%03X ", enderecoAtual.endereco);
                    printf("14 000 ");
                }
                else    
                    printf("14 000\n"); 

                atualizaEnderecoInstrucao(&enderecoAtual);    
            }

            else if(!strcmp(tokenAtual->palavra, "rsh")){

                if(enderecoAtual.lado == 0){
                    printf("%03X ", enderecoAtual.endereco);
                    printf("15 000 ");
                }
                else    
                    printf("15 000\n");

                atualizaEnderecoInstrucao(&enderecoAtual);     
            }

            else if(!strcmp(tokenAtual->palavra, "storend")){

                if(enderecoAtual.lado == 0)
                    printf("%03X ", enderecoAtual.endereco);
                    
                Token *proximoToken1 = recuperaToken(i + 1);
                transformaMinusculo(proximoToken1->palavra);
                if(proximoToken1->tipo == 1005){
                    Endereco *ptrEndereco = substituiRotulo(proximoToken1->palavra, listaDefRotulo, numeroRotulos);
                    
                    if(!(ptrEndereco == NULL)){
                        Endereco enderecoRotulo; 
                        enderecoRotulo = *ptrEndereco;
                        if(enderecoRotulo.lado == 0)
                            printf("12 ");
                        else if(enderecoRotulo.lado == 1)
                            printf("13 ");
                        if(enderecoAtual.lado == 0)
                            printf("%03X ", enderecoRotulo.endereco);
                        else
                            printf("%03X\n", enderecoRotulo.endereco);;
                    }
                    else{
                        char *valor = substituiSets(proximoToken1->palavra, listaDefSets, numeroSets);
                        if(valor != NULL){
                            if (valor[0] == '0' && (valor[1] == 'x' || valor[1] == 'X')){
                                long int numero = strtol(valor, NULL, 16);
                                printf("12 ");
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                            else{
                                long int numero = strtol(valor, NULL, 10);
                                printf("12 ");
                                if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                            }
                        } 
                    }
                }

                else if(proximoToken1->tipo == 1003){
                    printf("12 ");
                    long int numero = strtol(proximoToken1->palavra, NULL, 16);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                else if(proximoToken1->tipo == 1004){
                    printf("12 ");
                    long int numero = strtol(proximoToken1->palavra, NULL, 10);
                    if(enderecoAtual.lado == 0)
                                    printf("%03lX ", numero);
                                else
                                    printf("%03lX\n", numero);
                }

                atualizaEnderecoInstrucao(&enderecoAtual);
                i++;
            }

            
            

        }


/*-----------------------------------------------------------------DIRETIVAS-----------------------------------------------------------------------*/

        else if(tokenAtual->tipo == 1001){
            if(!strcmp(tokenAtual->palavra, ".word")){

                if(enderecoAtual.lado == 1){
                    printf("00 000\n");
                    enderecoAtual.lado = 0;
                    enderecoAtual.endereco += 1;
                }

                if(enderecoAtual.lado == 0)
                    printf("%03X ", enderecoAtual.endereco);
                    
                Token* proximoToken1 = recuperaToken(i + 1);


                transformaMinusculo(proximoToken1->palavra);
                if(proximoToken1->tipo == 1005){
                    Endereco *ptrEndereco = substituiRotulo(proximoToken1->palavra, listaDefRotulo, numeroRotulos);
                    
                    if(!(ptrEndereco == NULL)){
                        Endereco enderecoRotulo; 
                        enderecoRotulo = *ptrEndereco;
                        printf("00 000 00 ");
                        printf("%03X\n",enderecoRotulo.endereco);
                    }
                    else{
                        char *valor = substituiSets(proximoToken1->palavra, listaDefSets, numeroSets);
                        if(valor != NULL){
                            if (valor[0] == '0' && (valor[1] == 'x' || valor[1] == 'X')){
                                long int numero = strtol(valor, NULL, 16);
                                printf("00 000 00 ");
                                printf("%03lX\n", numero);
                            }
                            else{
                                long int numero = strtol(valor, NULL, 10);
                                printf("00 000 00 ");
                                printf("%03lX\n", numero);
                            }
                        } 
                    }
                }

                else if(proximoToken1->tipo == 1003){
                    long int numero = strtol(proximoToken1->palavra, NULL, 16);
                    printf("00 000 00 ");
                    printf("%03lX\n", numero);
                }

                else if(proximoToken1->tipo == 1004){
                    long int numero = strtol(proximoToken1->palavra, NULL, 10);
                    printf("00 000 00 ");
                    printf("%03lX\n", numero);
                }

                i++;

                enderecoAtual.endereco += 1;
                
            }


            if(!strcmp(tokenAtual->palavra, ".wfill")){

               if(enderecoAtual.lado == 1){           
                    printf("00 000\n");
                    enderecoAtual.lado = 0;
                    enderecoAtual.endereco += 1;
                }
                    
                Token* proximoToken1 = recuperaToken(i + 1);
                Token* proximoToken2 = recuperaToken(i + 2);
                long int vezes = strtol(proximoToken1->palavra, NULL, 10);

                transformaMinusculo(proximoToken2->palavra);
                if(proximoToken2->tipo == 1005){
                    Endereco *ptrEndereco = substituiRotulo(proximoToken2->palavra, listaDefRotulo, numeroRotulos);
                    
                    if(!(ptrEndereco == NULL)){
                        Endereco enderecoRotulo; 
                        enderecoRotulo = *ptrEndereco;
                        
                        for(long int y = 0; y < vezes; y++){
                            
                            printf("%03X ", enderecoAtual.endereco);
                            printf("00 000 00 ");
                            printf("%03X\n",enderecoRotulo.endereco);
                            enderecoAtual.endereco += 1;
                        }
                    }
                    else{
                        char *valor = substituiSets(proximoToken2->palavra, listaDefSets, numeroSets);
                        if(valor != NULL){
                            if (valor[0] == '0' && (valor[1] == 'x' || valor[1] == 'X')){
                                long int numero = strtol(valor, NULL, 16);
                                for(long int y = 0; y < vezes; y++){
                                    
                                    printf("%03X ", enderecoAtual.endereco);
                                    printf("00 000 00 ");
                                    printf("%03lX\n", numero);
                                    enderecoAtual.endereco += 1;
                                }
                            }
                            else{
                                long int numero = strtol(valor, NULL, 10);
                                for(long int y = 0; y < vezes; y++){
                                    
                                    printf("%03X ", enderecoAtual.endereco);
                                    printf("00 000 00 ");
                                    printf("%03lX\n", numero);
                                    enderecoAtual.endereco += 1;
                                }
                            }
                        } 
                    }
                }

                else if(proximoToken2->tipo == 1003){
                    long int numero = strtol(proximoToken2->palavra, NULL, 16);
                    for(long int y = 0; y < vezes; y++){
                        
                        printf("%03X ", enderecoAtual.endereco);
                        printf("00 000 00 ");
                        printf("%03lX\n", numero);
                        enderecoAtual.endereco += 1;
                    }
                }

                else if(proximoToken2->tipo == 1004){
                    long int numero = strtol(proximoToken2->palavra, NULL, 10);
                    for(long int y = 0; y < vezes; y++){
                        
                        printf("%03X ", enderecoAtual.endereco);
                        printf("00 000 00 ");
                        printf("%03lX\n", numero);
                        enderecoAtual.endereco += 1;
                    }
                }

                i += 2;

                
                
            }

            if(!(strcmp(tokenAtual->palavra, ".org"))){
                Token *proximoToken1 = recuperaToken(i+1);
                int numero;
                if(proximoToken1->tipo == 1003)    
                    numero = strtol(proximoToken1->palavra, NULL, 16);
                
                else
                    numero = strtol(proximoToken1->palavra, NULL, 10);
            
                enderecoAtual.endereco = numero;
                enderecoAtual.lado = 0;
                i++;
            }

            else if(!(strcmp(tokenAtual->palavra, ".align"))){
                if(enderecoAtual.lado == 1){
                    printf("00 000\n");
                    enderecoAtual.endereco += 1;
                    enderecoAtual.lado = 0;
                }
                Token *proximoToken1 = recuperaToken(i+1);
                int numero;
                numero = strtol(proximoToken1->palavra, NULL, 10);

                for (int j = 0; j <= numero; j++){
                    if((enderecoAtual.endereco % numero == 0) && enderecoAtual.lado == 0)
                        break;                   
                    enderecoAtual.lado = 0;
                    enderecoAtual.endereco += 1;
                }
                i++;
            }
        }
    }

    
    if(enderecoAtual.lado == 1){     //caso o mapa termine com uma linha incompleta completa ela com zeros
        printf("00 000\n");
    }
    
    return 0;
}
