#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define LEN(a) (sizeof(a) / sizeof(a[0]))
#define MAP_IDENTS (128)
#define STRING_MAX (128)

char* TermOps[] = { "+", "-", "=", NULL };
char* FactOps[] = { "*", "%", "/", NULL };

typedef char String[STRING_MAX];

typedef struct
{
    String name;
    int redirects;
}
Ident;

typedef struct
{
    Ident ident[MAP_IDENTS];
    int count;
}
Map;

typedef struct
{
    int right;
    int redirects;
}
Value;

int Equal(char* a, char* b)
{
    return strcmp(a, b) == 0;
}

unsigned Hash(char* string)
{
    unsigned h = 0;
    unsigned char* p;
    for(p = (unsigned char*) string; *p; p++)
        h = 31 * h + *p;
    return h;
}

int Index(char* string)
{
    return Hash(string) % MAP_IDENTS;
}

int Next(int index)
{
    return (index + 1) % MAP_IDENTS;
}

Ident* Find(Map* map, char* name)
{
    int i;
    int index = Index(name);
    for(i = 0; i < MAP_IDENTS; i++)
    {
        Ident* at = &map->ident[index];
        if(Equal(at->name, name))
            return at;
        index = Next(index);
    }
    return NULL;
}

void Insert(Map* map, Ident ident)
{
    int i;
    int index = Index(ident.name);
    for(i = 0; i < MAP_IDENTS; i++)
    {
        Ident* at = &map->ident[i];
        if(at->name[0] == '\0')
        {
            *at = ident;
            map->count += 1;
            break;
        }
        index = Next(index);
    }
}

void Delete(Map* map, char* name)
{
    Ident* ident = Find(map, name);
    if(ident)
    {
        ident->name[0] = '\0';
        map->count -= 1;
    }
}

int IsOp(char* op, char* which[])
{
    char** at;
    for(at = which; *at; at++)
        if(Equal(*at, op))
            return 1;
    return 0;
}

int IsOpChar(int c, char* which[])
{
    char** at;
    char* op;
    for(at = which; *at; at++)
        for(op = *at; *op; op++)
            if(*op == c)
                return 1;
    return 0;
}

int IsTermOp(char* op)
{
    return IsOp(op, TermOps);
}

int IsFactOp(char* op)
{
    return IsOp(op, FactOps);
}

int IsTermOpChar(int c)
{
    return IsOpChar(c, TermOps);
}

int IsFactOpChar(int c)
{
    return IsOpChar(c, FactOps);
}

void Stream(FILE* file, int clause(int), String string)
{
    int i = 0;
    char c;
    while(clause(c = fgetc(file)))
        if(clause != isspace)
            string[i++] = c;
    ungetc(c, file);
    string[i] = '\0';
}

void Space(FILE* file)
{
    String string;
    Stream(file, isspace, string);
}

int Peek(FILE* file)
{
    Space(file);
    return ungetc(fgetc(file), file);
}

void Alpha(FILE* file, String string)
{
    Space(file);
    Stream(file, isalpha, string);
}

void Digit(FILE* file, String string)
{
    Space(file);
    Stream(file, isdigit, string);
}

void TermOp(FILE* file, String string)
{
    Space(file);
    Stream(file, IsTermOpChar, string);
}

void FactOp(FILE* file, String string)
{
    Space(file);
    Stream(file, IsFactOpChar, string);
}

void Match(FILE* file, char* with)
{
    Space(file);
    while(*with)
        assert(*with++ == fgetc(file));
}

Value Identifier(FILE* file, Map* map)
{
    Value value = { 0 };
    String alpha;
    Ident* ident;
    Alpha(file, alpha);
    ident = Find(map, alpha);
    assert(ident);
    value.redirects = ident->redirects;
    return value;
}

Value Direct(FILE* file)
{
    String digit;
    Value value = { 1, -1 };
    Digit(file, digit);
    return value;
}

Value Factor(FILE* file, Map* map);

Value Deref(FILE* file, Map* map)
{
    Value value;
    Match(file, "*");
    value = Factor(file, map);
    assert(value.redirects > 0);
    value.redirects -= 1;
    value.right = 0;
    return value;
}

Value Ref(FILE* file, Map* map)
{
    Value value;
    Match(file, "&");
    value = Factor(file, map);
    assert(value.right == 0);
    assert(value.redirects >= 0);
    value.redirects += 1;
    value.right = 1;
    return value;
}

Value Expression(FILE* file, Map* map);

Value Paren(FILE* file, Map* map)
{
    Value value;
    Match(file, "(");
    value = Expression(file, map);
    Match(file, ")");
    return value;
}

Value Neg(FILE* file, Map* map)
{
    Match(file, "-");
    return Factor(file, map);
}

Value Pos(FILE* file, Map* map)
{
    Match(file, "+");
    return Factor(file, map);
}

Value Unary(FILE* file, Map* map)
{
    int c = Peek(file);
    if(c == '+') return Pos(file, map);
    if(c == '-') return Neg(file, map);
    if(c == '&') return Ref(file, map);
    if(c == '*') return Deref(file, map);
    if(c == '(') return Paren(file, map);
    printf("unknown unary operator %c\n", c);
    exit(1);
}

Value Factor(FILE* file, Map* map)
{
    int c = Peek(file);
    if(isdigit(c)) return Direct(file);
    if(isalpha(c)) return Identifier(file, map);
    return Unary(file, map);
}

void Add(void)
{
}

void Sub(void)
{
}

void Mul(void)
{
}

void Div(void)
{
}

void Mod(void)
{
}

void Assign(void)
{
}

Value Term(FILE* file, Map* map)
{
    String op;
    Value l = Factor(file, map);
    for(FactOp(file, op); IsFactOp(op); FactOp(file, op))
    {
        Value r = Factor(file, map);
        if(l.redirects > 0) assert(r.redirects <= 0);
        if(r.redirects > 0) assert(l.redirects <= 0);
        if(Equal(op, "*")) Mul();
        if(Equal(op, "%")) Mod();
        if(Equal(op, "/")) Div();
        l.redirects = MAX(l.redirects, r.redirects);
        l.right = 1;
    }
    return l;
}

Value Expression(FILE* file, Map* map)
{
    String op;
    Value l = Term(file, map);
    Value r;
    for(TermOp(file, op); IsTermOp(op); TermOp(file, op))
    {
        if(Equal(op, "="))
        {
            assert(l.right == 0);
            r = Expression(file, map);
            assert(l.redirects == r.redirects);
            Assign();
        }
        else
        {
            r = Term(file, map);
            if(l.redirects > 0) assert(r.redirects <= 0);
            if(r.redirects > 0) assert(l.redirects <= 0);
            if(Equal(op, "+")) Add();
            if(Equal(op, "-")) Sub();
        }
        l.redirects = MAX(l.redirects, r.redirects);
        l.right = 1;
    }
    return l;
}

Ident LetDec(FILE* file, Map* map)
{
    String let;
    Alpha(file, let);
    Ident ident = { 0 };
    while(Peek(file) == '*')
    {
        Match(file, "*");
        ident.redirects += 1;
    }
    return ident;
}

int End(FILE* file)
{
    Space(file);
    return Peek(file) == EOF;
}

void LetDef(FILE* file, Map* map)
{
    Value r;
    Ident ident = LetDec(file, map);
    Alpha(file, ident.name);
    Insert(map, ident);
    Match(file, "=");
    r = Expression(file, map);
    Match(file, ";");
    if(ident.redirects > 0)
        assert(r.redirects == ident.redirects);
}

void Program(FILE* file, Map* map)
{
    while(!End(file))
        LetDef(file, map);
}

int main(void)
{
    FILE* file = fopen("main.sw", "r");
    Map map = { 0 };
    Program(file, &map);
    fclose(file);
    return 0;
}
