#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TAM_LEXEMA 128

/* -------------------- Tokens da Csmall -------------------- */

typedef enum
{
    TOK_INT,
    TOK_FLOAT,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_PRINT,
    TOK_IDENTIFIER,
    TOK_INT_CONST,
    TOK_FLOAT_CONST,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MULT,
    TOK_DIV,
    TOK_ASSIGN,
    TOK_LT,
    TOK_LE,
    TOK_GT,
    TOK_GE,
    TOK_EQ,
    TOK_NE,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_SEMICOLON,
    TOK_EOF,
    TOK_INVALID
} TokenType;

typedef struct
{
    TokenType tipo;
    char lexema[TAM_LEXEMA];
    int linha;
} Token;

const char *nomeToken(TokenType tipo)
{
    switch (tipo)
    {
    case TOK_INT:
        return "INT";
    case TOK_FLOAT:
        return "FLOAT";
    case TOK_IF:
        return "IF";
    case TOK_ELSE:
        return "ELSE";
    case TOK_WHILE:
        return "WHILE";
    case TOK_PRINT:
        return "PRINT";
    case TOK_IDENTIFIER:
        return "IDENTIFIER";
    case TOK_INT_CONST:
        return "INT_CONST";
    case TOK_FLOAT_CONST:
        return "FLOAT_CONST";
    case TOK_PLUS:
        return "PLUS";
    case TOK_MINUS:
        return "MINUS";
    case TOK_MULT:
        return "MULT";
    case TOK_DIV:
        return "DIV";
    case TOK_ASSIGN:
        return "ASSIGN";
    case TOK_LT:
        return "LT";
    case TOK_LE:
        return "LE";
    case TOK_GT:
        return "GT";
    case TOK_GE:
        return "GE";
    case TOK_EQ:
        return "EQ";
    case TOK_NE:
        return "NE";
    case TOK_LPAREN:
        return "LPAREN";
    case TOK_RPAREN:
        return "RPAREN";
    case TOK_LBRACE:
        return "LBRACE";
    case TOK_RBRACE:
        return "RBRACE";
    case TOK_SEMICOLON:
        return "SEMICOLON";
    case TOK_EOF:
        return "EOF";
    case TOK_INVALID:
        return "INVALID";
    default:
        return "UNKNOWN";
    }
}

void imprimirToken(Token token)
{
    printf("%-14s %-20s linha %d\n",
           nomeToken(token.tipo), token.lexema, token.linha);
}

/* -------------------- Leitura do codigo-fonte -------------------- */

typedef struct
{
    FILE *arquivo;
    int linha;
} CodigoFonte;

int abrirCodigoFonte(CodigoFonte *codigo, const char *nomeArquivo)
{
    codigo->arquivo = fopen(nomeArquivo, "r");
    codigo->linha = 1;

    if (codigo->arquivo == NULL)
    {
        fprintf(stderr, "Nao foi possivel abrir o arquivo: %s\n", nomeArquivo);
        return 0;
    }

    return 1;
}

void fecharCodigoFonte(CodigoFonte *codigo)
{
    if (codigo->arquivo != NULL)
    {
        fclose(codigo->arquivo);
        codigo->arquivo = NULL;
    }
}

/* Retornamos int porque fgetc() precisa poder retornar EOF. */
int lerCaractere(CodigoFonte *codigo)
{
    int caractere = fgetc(codigo->arquivo);

    if (caractere == '\n')
    {
        codigo->linha++;
    }

    return caractere;
}

void devolverCaractere(CodigoFonte *codigo, int caractere)
{
    if (caractere == EOF)
    {
        return;
    }

    ungetc(caractere, codigo->arquivo);

    if (caractere == '\n')
    {
        codigo->linha--;
    }
}

/* Lê um caractere para consulta e depois o devolve ao arquivo (lookahead). */
int olharCaractere(CodigoFonte *codigo)
{
    int caractere = lerCaractere(codigo);
    devolverCaractere(codigo, caractere);
    return caractere;
}

/* -------------------- Mensagens de erro -------------------- */

void erroLexico(int linha)
{
    fprintf(stderr, "Erro lexico na linha %d.\n", linha);
}

void erroSintatico(int linha, TokenType esperado, TokenType encontrado)
{
    fprintf(stderr, "Erro sintatico na linha %d: esperado %s, encontrado %s.\n",
            linha, nomeToken(esperado), nomeToken(encontrado));
}

void erroSemantico(int linha, const char *mensagem)
{
    fprintf(stderr, "Erro semantico na linha %d: %s\n", linha, mensagem);
}

/* -------------------- Identificadores -------------------- */

char VALID_SYMBOLS[] = {
    '+', '-', '*', '/', '=', '<', '>', '!', '(', ')', '{', '}', ';'};

int isSymbol(char c)
{
    int numSymbols = sizeof(VALID_SYMBOLS) / sizeof(VALID_SYMBOLS[0]);
    for (int i = 0; i < numSymbols; i++)
    {
        if (c == VALID_SYMBOLS[i])
        {
            return 1;
        }
    }
    return 0;
}

/* -------------------- Lexer ------------------------------------ */

Token scanWord(CodigoFonte *codigo)
{
    Token token = {TOK_IDENTIFIER, "", codigo->linha};
    int i = 0;
    while (i < TAM_LEXEMA - 1)
    {
        int caractere = olharCaractere(codigo);
        if (caractere == EOF || !(isalpha((unsigned char)caractere) || isdigit((unsigned char)caractere) || (unsigned char)caractere == '_'))
        {
            break;
        }
        token.lexema[i] = caractere;
        i++;
        lerCaractere(codigo);
    }
    token.lexema[i] = '\0';
    if (strcmp(token.lexema, "int") == 0) token.tipo = TOK_INT;
    else if (strcmp(token.lexema, "float") == 0) token.tipo = TOK_FLOAT;
    else if (strcmp(token.lexema, "if") == 0) token.tipo = TOK_IF;
    else if (strcmp(token.lexema, "else") == 0) token.tipo = TOK_ELSE;
    else if (strcmp(token.lexema, "while") == 0) token.tipo = TOK_WHILE;
    else if (strcmp(token.lexema, "print") == 0) token.tipo = TOK_PRINT;
    return token;
}

Token scanNumber(CodigoFonte *codigo)
{
    Token token = {TOK_INT_CONST, "", codigo->linha};

    int isFloat = 0;
    for (int i = 0; i < TAM_LEXEMA-1; i++)
    {
        char caractere = olharCaractere(codigo);
        if (isdigit(caractere))
        {
            token.lexema[i] = caractere;
        }
        else if (caractere == '.')
        {
            if (isFloat)
            {
                token.tipo = TOK_INVALID;
                token.lexema[i] = '\0';
                return token;
            }

            token.lexema[i] = caractere;
            token.tipo = TOK_FLOAT_CONST;
            isFloat = 1;
        }
        else
        {
            token.lexema[i] = '\0';
            return token;
        }
        lerCaractere(codigo);
    }

    token.tipo = TOK_INVALID;
    return token;
}

Token scanSymbol(CodigoFonte *codigo)
{
    Token token = {TOK_INVALID, "", codigo->linha};

    char caractere = lerCaractere(codigo);
    switch (caractere)
    {
    case '+':
        strcpy(token.lexema, "+");
        token.tipo = TOK_PLUS;
        return token;
    case '-':
        strcpy(token.lexema, "-");
        token.tipo = TOK_MINUS;
        return token;
    case '*':
        strcpy(token.lexema, "*");
        token.tipo = TOK_MULT;
        return token;
    case '/':
        strcpy(token.lexema, "/");
        token.tipo = TOK_DIV;
        return token;
    case '=':
        caractere = olharCaractere(codigo);
        if (caractere == '=')
        {
            lerCaractere(codigo);
            strcpy(token.lexema, "==");
            token.tipo = TOK_EQ;
        }
        else
        {
            strcpy(token.lexema, "=");
            token.tipo = TOK_ASSIGN;
        }
        return token;
    case '<':
        caractere = olharCaractere(codigo);
        if (caractere == '=')
        {
            lerCaractere(codigo);
            strcpy(token.lexema, "<=");
            token.tipo = TOK_LE;
        }
        else
        {
            strcpy(token.lexema, "<");
            token.tipo = TOK_LT;
        }
        return token;
    case '>':
        caractere = olharCaractere(codigo);
        if (caractere == '=')
        {
            lerCaractere(codigo);
            strcpy(token.lexema, ">=");
            token.tipo = TOK_GE;
        }
        else
        {
            strcpy(token.lexema, ">");
            token.tipo = TOK_GT;
        }
        return token;
    case '!':
        caractere = olharCaractere(codigo);
        if (caractere == '=')
        {
            lerCaractere(codigo);
            strcpy(token.lexema, "!=");
            token.tipo = TOK_NE;
        }
        return token;
    case '(':
        strcpy(token.lexema, "(");
        token.tipo = TOK_LPAREN;
        return token;
    case ')':
        strcpy(token.lexema, ")");
        token.tipo = TOK_RPAREN;
        return token;
    case '{':
        strcpy(token.lexema, "{");
        token.tipo = TOK_LBRACE;
        return token;
    case '}':
        strcpy(token.lexema, "}");
        token.tipo = TOK_RBRACE;
        return token;
    case ';':
        strcpy(token.lexema, ";");
        token.tipo = TOK_SEMICOLON;
        return token;
    default:
        token.lexema[0] = caractere;
        token.lexema[1] = '\0';
        return token;
    }
}

Token proximoToken(CodigoFonte *codigo)
{
    Token token = {TOK_EOF, "", codigo->linha};

    int caractere = olharCaractere(codigo);

    while(caractere != EOF && isspace((unsigned char)caractere))
    {
        lerCaractere(codigo);
        caractere = olharCaractere(codigo);
    }
    if (caractere == EOF)
    {
        token.linha = codigo->linha;
        token.tipo = TOK_EOF;
        strcpy(token.lexema, "EOF");
        return token;
    }
    if (isalpha((unsigned char)caractere) || caractere == '_')
    {
        return scanWord(codigo);
    }
    if (isdigit((unsigned char)caractere))
    {
        return scanNumber(codigo);
    }
    if (isSymbol((char)caractere))
    {
        return scanSymbol(codigo);
    }

    caractere = lerCaractere(codigo);
    token.tipo = TOK_INVALID;
    token.lexema[0] = (char)caractere;
    token.lexema[1] = '\0';
    return token;
}

/* -------------------- Tabela de simbolos: estruturas iniciais -------------------- */

typedef enum
{
    TIPO_INT,
    TIPO_FLOAT,
    TIPO_ERRO
} TipoDado;

typedef struct EntradaSimbolo
{
    char lexema[TAM_LEXEMA];
    TipoDado tipo;
    int linha;
    struct EntradaSimbolo *prox;
} EntradaSimbolo;

typedef struct TabelaSimbolos
{
    EntradaSimbolo *entradas;
    struct TabelaSimbolos *anterior;
} TabelaSimbolos;

int inserirSimbolo(TabelaSimbolos *tabela, const char *lexema, TipoDado tipo, int linha);
EntradaSimbolo *buscarSimbolo(TabelaSimbolos *tabela, const char *lexema);

TabelaSimbolos *criarEscopo(TabelaSimbolos *anterior)
{
    TabelaSimbolos *nova = (TabelaSimbolos *)malloc(sizeof(TabelaSimbolos));
    if (!nova) return NULL;
    nova->entradas = NULL;
    nova->anterior = anterior;
    return nova;
}

TabelaSimbolos *destruirEscopo(TabelaSimbolos *tabela)
{
    if (!tabela) return NULL;
    TabelaSimbolos *anterior = tabela->anterior;
    EntradaSimbolo *atual = tabela->entradas;
    while (atual != NULL)
    {
        EntradaSimbolo *temp = atual;
        atual = atual->prox;
        free(temp);
    }
    free(tabela);
    return anterior;
}

/* -------------------- Parser: estrutura inicial -------------------- */

struct Parser
{
    CodigoFonte *codigo;
    Token tokenAtual;
    TabelaSimbolos *tabela;
};

typedef struct Parser Parser;

/* -------------------- Declaracoes Antecipadas do Parser -------------------- */

void listaDecl(Parser *parser);
void declaracao(Parser *parser);
TipoDado tipo(Parser *parser);
void listaCmd(Parser *parser);
void comando(Parser *parser);
void atribuicao(Parser *parser);
void Print(Parser *parser);
void If(Parser *parser);
void opcionalElse(Parser *parser);
void While(Parser *parser);
void bloco(Parser *parser);
void condicao(Parser *parser);
void relOp(Parser *parser);
TipoDado expressao(Parser *parser);
TipoDado exprLinha(Parser *parser, TipoDado esq);
TipoDado termo(Parser *parser);
TipoDado termoLinha(Parser *parser, TipoDado esq);
TipoDado fator(Parser *parser);


void avancarEntrada(Parser *parser)
{
    parser->tokenAtual = proximoToken(parser->codigo);
    imprimirToken(parser->tokenAtual);
}

void casar(Parser *parser, TokenType esperado)
{
    if (parser->tokenAtual.tipo == esperado)
    {
        avancarEntrada(parser);
    }
    else
    {
        erroSintatico(parser->tokenAtual.linha, esperado, parser->tokenAtual.tipo);
    }
}

void programa(Parser *parser)
{
    listaDecl(parser);
    listaCmd(parser); 
}

void listaDecl(Parser *parser)
{
    if (parser->tokenAtual.tipo == TOK_INT || parser->tokenAtual.tipo == TOK_FLOAT)
    {
        declaracao(parser);
        listaDecl(parser);
    } else {}
}

void declaracao(Parser *parser)
{
    TipoDado t = tipo(parser);
    if (parser->tokenAtual.tipo == TOK_IDENTIFIER)
    {
        inserirSimbolo(parser->tabela, parser->tokenAtual.lexema, t, parser->tokenAtual.linha);
        casar(parser, TOK_IDENTIFIER);
    }
    else
    {
        erroSintatico(parser->tokenAtual.linha, TOK_IDENTIFIER, parser->tokenAtual.tipo);
    }
    casar(parser, TOK_SEMICOLON);
}

TipoDado tipo(Parser *parser)
{
    if (parser->tokenAtual.tipo == TOK_INT)
    {
        casar(parser, TOK_INT);
        return TIPO_INT;  
    }
    else if (parser->tokenAtual.tipo == TOK_FLOAT)
    {
        casar(parser, TOK_FLOAT); 
        return TIPO_FLOAT;  
    }
    else
    {
        erroSintatico (parser->tokenAtual.linha, TOK_INT, parser->tokenAtual.tipo);
        return TIPO_ERRO;
    }
}

void listaCmd(Parser *parser)
{
    if (parser->tokenAtual.tipo == TOK_IDENTIFIER || parser->tokenAtual.tipo == TOK_PRINT || parser->tokenAtual.tipo == TOK_IF || parser->tokenAtual.tipo == TOK_WHILE || parser->tokenAtual.tipo == TOK_LBRACE)
    {
        comando(parser);
        listaCmd(parser);
    } else {}
}

void comando(Parser *parser)
{
    if (parser->tokenAtual.tipo == TOK_IDENTIFIER)
    {
        atribuicao(parser);
    }
    else if (parser->tokenAtual.tipo == TOK_PRINT)
    {
        Print(parser);
    }
    else if (parser->tokenAtual.tipo == TOK_IF)
    {
        If(parser);
    }
    else if (parser->tokenAtual.tipo == TOK_WHILE)
    {
        While(parser);
    }
    else if (parser->tokenAtual.tipo == TOK_LBRACE)
    {
        bloco(parser);
    }
    else{
        erroSintatico(parser->tokenAtual.linha, TOK_IDENTIFIER , parser->tokenAtual.tipo);
    }

}

void atribuicao(Parser *parser)
{
    char varNome[TAM_LEXEMA];
    int linha = parser->tokenAtual.linha;
    strcpy(varNome, parser->tokenAtual.lexema);
    casar(parser, TOK_IDENTIFIER);
    
    EntradaSimbolo *simbolo = buscarSimbolo(parser->tabela, varNome);
    if (!simbolo)
    {
        erroSemantico(linha, "Variavel nao declarada.");
    }
    casar(parser, TOK_ASSIGN);
    TipoDado tExpr = expressao(parser);
    if (simbolo && tExpr != TIPO_ERRO && simbolo->tipo != tExpr)
    {
        erroSemantico(linha, "Atribuicao com tipos incompativeis.");
    }
    casar(parser, TOK_SEMICOLON);
}

void Print(Parser *parser)
{
    int linha = parser->tokenAtual.linha;
    casar(parser, TOK_PRINT);
    casar(parser, TOK_LPAREN);
    TipoDado t = expressao(parser);
    if (t == TIPO_ERRO)
    {
        erroSemantico(linha, "Expressao invalida no print.");
    }
    casar(parser, TOK_RPAREN);
    casar(parser, TOK_SEMICOLON);
} 

void If(Parser *parser)
{
    casar(parser, TOK_IF);
    casar(parser, TOK_LPAREN);
    condicao(parser);
    casar(parser, TOK_RPAREN);
    bloco(parser);
    opcionalElse(parser);
}

void opcionalElse(Parser *parser)
{
    if (parser->tokenAtual.tipo == TOK_ELSE)
    {
        casar(parser, TOK_ELSE);
        bloco(parser);
    } else {}
}

void While(Parser *parser)
{
        casar(parser, TOK_WHILE);
        casar(parser, TOK_LPAREN);
        condicao(parser);
        casar(parser, TOK_RPAREN);
        bloco(parser);
}

void bloco(Parser *parser)
{
    casar(parser, TOK_LBRACE);
    parser->tabela = criarEscopo(parser->tabela);
    listaDecl(parser);
    listaCmd(parser);
    parser->tabela = destruirEscopo(parser->tabela);
    casar(parser, TOK_RBRACE);
}

void condicao(Parser *parser)
{
    int linha = parser->tokenAtual.linha;
    TipoDado t1 = expressao(parser);
    relOp(parser);
    TipoDado t2 = expressao(parser);
    if (t1 == TIPO_ERRO || t2 == TIPO_ERRO)
    {
        erroSemantico(linha, "Condicao invalida.");
    }
}

void relOp(Parser *parser)
{
    if (parser->tokenAtual.tipo == TOK_LT)
    {
        casar(parser, TOK_LT);
    }
    else if (parser->tokenAtual.tipo == TOK_LE)
    {
        casar(parser, TOK_LE);
    }
    else if (parser->tokenAtual.tipo == TOK_GT)
    {
        casar(parser, TOK_GT);
    }
    else if (parser->tokenAtual.tipo == TOK_GE)
    {
        casar(parser, TOK_GE);
    }
    else if (parser->tokenAtual.tipo == TOK_EQ)
    {
        casar(parser, TOK_EQ);
    }
    else if (parser->tokenAtual.tipo == TOK_NE)
    {
        casar(parser, TOK_NE);
    }
    else{
        erroSintatico(parser->tokenAtual.linha, TOK_EQ, parser->tokenAtual.tipo);
    }
}

TipoDado expressao(Parser *parser)
{
    TipoDado tTermo = termo(parser);
    return exprLinha(parser, tTermo);
}

TipoDado exprLinha(Parser *parser, TipoDado esq)
{
    if (parser->tokenAtual.tipo == TOK_PLUS)
    {
        int linha = parser->tokenAtual.linha;
        casar(parser, TOK_PLUS);
        TipoDado dir = termo(parser);
        
        TipoDado res;
        if (esq == TIPO_FLOAT || dir == TIPO_FLOAT)
        {
            res = TIPO_FLOAT;
        }
        else
        {
            res = TIPO_INT;
        }
        if (esq == TIPO_ERRO || dir == TIPO_ERRO) res = TIPO_ERRO;

        return exprLinha(parser, res);
    }
    else if (parser->tokenAtual.tipo == TOK_MINUS)
    {
        int linha = parser->tokenAtual.linha;
        casar(parser, TOK_MINUS);
        TipoDado dir = termo(parser);

        TipoDado res;
        if (esq == TIPO_FLOAT || dir == TIPO_FLOAT)
        {
            res = TIPO_FLOAT;
        }
        else
        {
            res = TIPO_INT;
        }
        if (esq == TIPO_ERRO || dir == TIPO_ERRO) res = TIPO_ERRO;
    
        return exprLinha (parser, res);
    }
    else
    {
        return esq;
    }
}

TipoDado termo(Parser *parser)
{
    TipoDado tFator = fator(parser);
    return termoLinha(parser, tFator);
}

TipoDado termoLinha(Parser *parser, TipoDado esq)
{
    if(parser->tokenAtual.tipo == TOK_MULT)
    {
        casar(parser, TOK_MULT);
        TipoDado dir = fator(parser);

        TipoDado res;
        if (esq == TIPO_FLOAT || dir == TIPO_FLOAT)
        {
            res = TIPO_FLOAT;
        }
        else
        {
            res = TIPO_INT;
        }
        if (esq == TIPO_ERRO || dir == TIPO_ERRO) res = TIPO_ERRO;

        return termoLinha(parser, res);
    }
    else if (parser->tokenAtual.tipo == TOK_DIV)
    {
        casar(parser, TOK_DIV);
        TipoDado dir = fator(parser);
        TipoDado res;
        if (esq == TIPO_FLOAT || dir == TIPO_FLOAT)
        {
            res = TIPO_FLOAT;
        }
        else
        {
            res = TIPO_INT;
        }
        if (esq == TIPO_ERRO || dir == TIPO_ERRO) res = TIPO_ERRO;

        return termoLinha(parser, res);
    } 
    else 
    {
        return esq;
    }
}

TipoDado fator (Parser *parser)
{
    if(parser->tokenAtual.tipo == TOK_IDENTIFIER)
    {
        EntradaSimbolo *simbolo = buscarSimbolo(parser->tabela, parser->tokenAtual.lexema);
        if (!simbolo)
        {
            erroSemantico(parser->tokenAtual.linha, "Variavel nao declarada.");
            casar(parser, TOK_IDENTIFIER);
            return TIPO_ERRO;
        }
        casar(parser, TOK_IDENTIFIER);
        return simbolo->tipo;
    }
    else if (parser->tokenAtual.tipo == TOK_INT_CONST)
    {
        casar(parser, TOK_INT_CONST);
        return TIPO_INT;
    }
    else if (parser->tokenAtual.tipo == TOK_FLOAT_CONST)
    {
        casar(parser, TOK_FLOAT_CONST);
        return TIPO_FLOAT;
    }
    else if (parser->tokenAtual.tipo == TOK_LPAREN)
    {
        casar(parser, TOK_LPAREN);
        TipoDado t = expressao(parser);
        casar(parser, TOK_RPAREN);
        return t;
    } 
    else
    {
        erroSintatico(parser->tokenAtual.linha, TOK_INT_CONST, parser->tokenAtual.tipo);
        return TIPO_ERRO;
    }
}

int analisarPrograma(Parser *parser)
{
    parser->tabela = criarEscopo(NULL);
    avancarEntrada(parser);
    programa(parser);

    if (parser->tokenAtual.tipo != TOK_EOF)
    {
        erroSintatico(parser->tokenAtual.linha, TOK_EOF, parser->tokenAtual.tipo);
        return 0;
    }
    parser->tabela = destruirEscopo(parser->tabela);
    return 1;
}

int inserirSimbolo(TabelaSimbolos *tabela, const char *lexema, TipoDado tipo, int linha)
{
    if (tabela == NULL){
        return 0;
    }

    EntradaSimbolo *atual = tabela->entradas;
    while (atual != NULL)
    {
        if(strcmp(atual->lexema, lexema) == 0)
        {
            erroSemantico(linha, "Variável já declarada nesse escopo.");
            return 0;
        }
        atual = atual->prox;
    }
    EntradaSimbolo *novo = (EntradaSimbolo *) malloc(sizeof(EntradaSimbolo));
    if (novo == NULL)
    {
        fprintf(stderr, "Erro de alocacao de memoria para o simbolo.\n");
        return 0;
    }

    strncpy(novo->lexema, lexema, TAM_LEXEMA-1);
    novo->lexema[TAM_LEXEMA - 1] = '\0';
    novo->tipo = tipo;
    novo->linha = linha;
    novo->prox = tabela->entradas;
    tabela->entradas = novo;
    return 1;
}

EntradaSimbolo *buscarSimbolo(TabelaSimbolos *tabela, const char *lexema)
{
    for (; tabela != NULL; tabela = tabela->anterior)
    {
        EntradaSimbolo *atual = tabela->entradas;
        while (atual != NULL)
        {
            if (strcmp(atual->lexema, lexema) == 0)
                return atual;
            atual = atual->prox;
        }
    }
    return NULL;
}

/* -------------------- Programa principal: testar o lexer -------------------- */

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s arquivo.csmall\n", argv[0]);
        return EXIT_FAILURE;
    }

    CodigoFonte codigo;
    if (!abrirCodigoFonte(&codigo, argv[1]))
    {
        return EXIT_FAILURE;
    }

    Parser parser;
    parser.codigo = &codigo;

    printf("Iniciando analise sintatica do arquivo: %s\n\n", argv[1]);
    printf("%-14s %-20s %s\n", "TOKEN", "LEXEMA", "LINHA");
    printf("-----------------------------------------------\n");
    
    analisarPrograma(&parser);
    fecharCodigoFonte(&codigo);

    return EXIT_SUCCESS;
}