#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// トークンの種類
typedef enum{
    TK_RESERVED, // 記号
    TK_NUM,      // 整数トークン
    TK_EOF,      // 終了トークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token{
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMの時の数値
    char *str;      // トークン文字列
};

// 入力プログラム
char *user_input;
// 現在のトークン
Token *token;

// エラー用関数
// printfと同じ引数をとる
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 次のトークンが期待したoperatorの時トークンを１つ読み進めてtrueを返す
bool consume(char op){
    if(token->kind != TK_RESERVED || token->str[0] != op)
        return false;
    token = token->next;
    return true;
}

// 次のトークンが期待したoperatorの時トークンを１つ読み進める
// それ以外ではエラーを返す
void expect(char op){
    if(token->kind != TK_RESERVED || token->str[0] != op)
        error_at(token->str, "'%c'ではありません", op);
    token = token->next;
}

// 次のトークンが数値の時トークンを１つ読み進めてその数値を返す
// それ以外ではエラーを返す
int expect_number(){
    if(token->kind != TK_NUM)
        error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof(){
    return token->kind == TK_EOF;
}

// 新しいトークンを生成してトークン列に繋げる
// トークン列の末尾のポインタを返す
Token *new_token(TokenKind kind, Token *cur, char *str){
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// 入力文字列pをトークナイズする
Token *tokenize(char *p){
    Token head;
    head.next = NULL;
    head.str = p - 1;
    Token *cur = &head;

    while(*p){
        if(isspace(*p)){
            p++;
            continue;
        }

        if(*p == '+' || *p == '-'){
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if(isdigit(*p)){
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(cur->str + 1, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

int main(int argc, char **argv){
    if(argc != 2){
    error("引数の個数が正しくありません");
    return 1;
    }

    // トークナイズする
    user_input = argv[1];
    token = tokenize(user_input);

    // アセンブリ前半部分の出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 式の最初が数であることをチェックして最初の命令を出力
    printf("    mov rax, %d\n", expect_number());


    // operatorと数のペアを消費しつつアセンブリを出力
    while(!at_eof()){
        if(consume('+')){
            printf("    add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("    sub rax, %d\n", expect_number());
    }

    printf("    ret\n");
    return 0;
}
