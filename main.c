#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX_MAP_IDENTS (512)
#define MAX_FUNCTION_ARGS (8)
#define MAX_STRING_CHARS (64)

char* TermOps[] = {
    "=", "==", "!=", "<", ">", "<=", ">=", "+", "-", "&", "^", "|", NULL
};

char* FactOps[] = {
    "*", "%", "/", NULL
};

char* UnaryOps[] = {
    "$" /* print */, "~", "+", "-", "&", "*", "(", NULL
};

char* Reserved[] = {
    "let", "while", "if", "elif", "else", "ret", NULL
};

typedef char String[MAX_STRING_CHARS];

typedef struct
{
    String name;
    int redirects;
    /* -1 for variables */
    int params;
}
Ident;

/* Open address hashing */
typedef Ident Map[MAX_MAP_IDENTS];

typedef struct
{
    /* Defaults as l-value */
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
    return Hash(string) % MAX_MAP_IDENTS;
}

int Next(int index)
{
    return (index + 1) % MAX_MAP_IDENTS;
}

int IsStringIn(char* string, char* strings[])
{
    char** at;
    for(at = strings; *at; at++)
        if(Equal(*at, string))
            return 1;
    return 0;
}

int IsCharIn(int c, char* strings[])
{
    char** at;
    char* string;
    for(at = strings; *at; at++)
        for(string = *at; *string; string++)
            if(*string == c)
                return 1;
    return 0;
}

Ident* Find(Map idents, char* name)
{
    int i;
    int index;
    assert(!IsStringIn(name, Reserved));
    index = Index(name);
    for(i = 0; i < MAX_MAP_IDENTS; i++)
    {
        Ident* at = &idents[index];
        if(Equal(at->name, name))
            return at;
        index = Next(index);
    }
    return NULL;
}

void Insert(Map idents, Ident ident)
{
    int i;
    int index;
    Ident* found = Find(idents, ident.name);
    assert(!found);
    index = Index(ident.name);
    for(i = 0; i < MAX_MAP_IDENTS; i++)
    {
        Ident* at = &idents[i];
        if(at->name[0] == '\0')
        {
            *at = ident;
            break;
        }
        index = Next(index);
    }
}

void Delete(Map idents, char* name)
{
    Ident* ident = Find(idents, name);
    if(ident)
        ident->name[0] = '\0';
}

int IsTermOp(char* op)
{
    return IsStringIn(op, TermOps);
}

int IsFactOp(char* op)
{
    return IsStringIn(op, FactOps);
}

int IsTermOpChar(int c)
{
    return IsCharIn(c, TermOps);
}

int IsFactOpChar(int c)
{
    return IsCharIn(c, FactOps);
}

int Read(FILE* file)
{
    int c = fgetc(file);
    /* Python style comments */
    if(c == '#')
        while(c != '\n')
            c = fgetc(file);
    return c;
}

void Stream(FILE* file, int clause(int), String string)
{
    int i = 0;
    char c;
    while(clause(c = Read(file)))
    {
        assert(i < MAX_STRING_CHARS);
        /* No need to fill buffer with spaces */
        string[clause == isspace ? 0 : i++] = c;
    }
    ungetc(c, file);
    assert(i < MAX_STRING_CHARS);
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
    return ungetc(Read(file), file);
}

void Alpha(FILE* file, String string)
{
    Space(file);
    Stream(file, isalpha, string);
}

int isalnumu(int c)
{
    return isalnum(c) || c == '_';
}

void Alnumu(FILE* file, String string)
{
    Space(file);
    Stream(file, isalnumu, string);
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
        assert(*with++ == Read(file));
}

Value Expression(FILE* file, Map idents);

void Args(FILE* file, Map idents, int expected)
{
    int args = 0;
    Match(file, "(");
    if(Peek(file) == ')')
    {
        Match(file, ")");
        return;
    }
    else
    {
        while(1)
        {
            Expression(file, idents);
            args += 1;
            if(Peek(file) == ',')
            {
                Match(file, ",");
                continue;
            }
            if(Peek(file) == ')')
                break;
        }
        Match(file, ")");
    }
    assert(args == expected);
}

Value Identifier(FILE* file, Map idents)
{
    Value value = { 0 };
    String alnum;
    Ident* ident;
    /* No need for digit check */
    Alnumu(file, alnum);
    ident = Find(idents, alnum);
    assert(ident);
    value.redirects = ident->redirects;
    if(Peek(file) == '(')
    {
        assert(ident->params >= 0);
        Args(file, idents, ident->params);
    }
    return value;
}

Value Char(FILE* file)
{
    Value value = { 1, -1 };
    Match(file, "'");
    if(Peek(file) == '\\')
        Match(file, "\\");
    Read(file);
    Match(file, "'");
    return value;
}

Value Number(FILE* file)
{
    String number;
    Value value = { 1, -1 };
    Digit(file, number);
    return value;
}

Value Factor(FILE* file, Map idents);

Value Deref(FILE* file, Map idents)
{
    Value value;
    Match(file, "*");
    value = Factor(file, idents);
    assert(value.redirects > 0);
    value.redirects -= 1;
    value.right = 0;
    return value;
}

Value Ref(FILE* file, Map idents)
{
    Value value;
    Match(file, "&");
    value = Factor(file, idents);
    assert(value.right == 0);
    assert(value.redirects >= 0);
    value.redirects += 1;
    value.right = 1;
    return value;
}

Value Paren(FILE* file, Map idents)
{
    Value value;
    Match(file, "(");
    value = Expression(file, idents);
    Match(file, ")");
    return value;
}

Value Neg(FILE* file, Map idents)
{
    Value value;
    Match(file, "-");
    value = Factor(file, idents);
    assert(value.redirects <= 0);
    return value;
}

Value Pos(FILE* file, Map idents)
{
    Value value;
    Match(file, "+");
    value = Factor(file, idents);
    assert(value.redirects <= 0);
    return value;
}

Value Inv(FILE* file, Map idents)
{
    Value value;
    Match(file, "~");
    value = Factor(file, idents);
    assert(value.redirects <= 0);
    return value;
}

Value Print(FILE* file, Map idents)
{
    Match(file, "$");
    return Factor(file, idents);
}

Value Unary(FILE* file, Map idents)
{
    Value none = { 0 };
    int c = Peek(file);
    assert(IsCharIn(c, UnaryOps));
    if(c == '$')
        return Print(file, idents);
    else
    if(c == '~')
        return Inv(file, idents);
    else
    if(c == '+')
        return Pos(file, idents);
    else
    if(c == '-')
        return Neg(file, idents);
    else
    if(c == '&')
        return Ref(file, idents);
    else
    if(c == '*')
        return Deref(file, idents);
    else
    if(c == '(')
        return Paren(file, idents);
    assert(1 && "uncaught unary operator");
    return none;
}

Value Factor(FILE* file, Map idents)
{
    int c = Peek(file);
    if(c == '\'')
        return Char(file);
    else
    if(isdigit(c))
        return Number(file);
    else
    if(isalnumu(c))
        return Identifier(file, idents);
    else
    return
        Unary(file, idents);
}

void Add(void)
{
}

void Sub(void)
{
}

void And(void)
{
}

void Xor(void)
{
}

void Or(void)
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

void EqualTo(void)
{
}

void NotEqualTo(void)
{
}

void LessThan(void)
{
}

void GreaterThan(void)
{
}

void LessThanOrEqualTo(void)
{
}

void GreaterThanOrEqualTo(void)
{
}

Value Term(FILE* file, Map idents)
{
    String op;
    Value l = Factor(file, idents);
    for(FactOp(file, op); IsFactOp(op); FactOp(file, op))
    {
        Value r = Factor(file, idents);
        if(l.redirects > 0)
            assert(r.redirects <= 0);
        if(r.redirects > 0)
            assert(l.redirects <= 0);
        if(Equal(op, "*"))
            Mul();
        else
        if(Equal(op, "%"))
            Mod();
        else
        if(Equal(op, "/"))
            Div();
        l.redirects = MAX(l.redirects, r.redirects);
        l.right = 1;
    }
    return l;
}

Value Expression(FILE* file, Map idents)
{
    String op;
    Value l = Term(file, idents);
    Value r;
    for(TermOp(file, op); IsTermOp(op); TermOp(file, op))
    {
        if(Equal(op, "="))
        {
            assert(l.right == 0);
            r = Expression(file, idents);
            assert(l.redirects == r.redirects);
            Assign();
        }
        else
        if(Equal(op, "=="))
        {
            r = Expression(file, idents);
            EqualTo();
        }
        else
        if(Equal(op, "!="))
        {
            r = Expression(file, idents);
            NotEqualTo();
        }
        else
        if(Equal(op, "<"))
        {
            r = Expression(file, idents);
            LessThan();
        }
        else
        if(Equal(op, ">"))
        {
            r = Expression(file, idents);
            GreaterThan();
        }
        else
        if(Equal(op, "<="))
        {
            r = Expression(file, idents);
            LessThanOrEqualTo();
        }
        else
        if(Equal(op, ">="))
        {
            r = Expression(file, idents);
            GreaterThanOrEqualTo();
        }
        else
        {
            r = Term(file, idents);
            if(l.redirects > 0)
                assert(r.redirects <= 0);
            if(r.redirects > 0)
                assert(l.redirects <= 0);
            if(Equal(op, "+"))
                Add();
            else
            if(Equal(op, "-"))
                Sub();
            else
            if(Equal(op, "&"))
                And();
            else
            if(Equal(op, "^"))
                Xor();
            else
            if(Equal(op, "|"))
                Or();
        }
        l.redirects = MAX(l.redirects, r.redirects);
        l.right = 1;
    }
    return l;
}

Ident LetDec(FILE* file)
{
    char first;
    Ident ident = { 0 };
    String let;
    Alpha(file, let);
    while(Peek(file) == '*')
    {
        Match(file, "*");
        ident.redirects += 1;
    }
    Alnumu(file, ident.name);
    first = ident.name[0];
    assert(isalpha(first) || first == '_');
    return ident;
}

int End(FILE* file)
{
    Space(file);
    return Peek(file) == EOF;
}

void Array(FILE* file, Map idents, Ident* array)
{
    int expressions = 0;
    int size;
    String digit;
    Match(file, "[");
    Digit(file, digit);
    size = atoi(digit);
    Match(file, "]");
    Match(file, "=");
    Match(file, "{");
    while(1)
    {
        Value r;
        if(Peek(file) == '}')
            break;
        r = Expression(file, idents);
        expressions += 1;
        assert(array->redirects == 0
            ? r.redirects < 1
            : r.redirects == array->redirects);
        if(Peek(file) == ',')
            Match(file, ",");
        else
            break;
    }
    Match(file, "}");
    Match(file, ";");
    if(size == 0)
    {
        assert(expressions > 0);
        size = expressions;
    }
    else
    {
        if(expressions == 1)
        {
            /* Fill size with expression */
        }
        else
            assert(size == expressions);
    }
    /* Arrays instantly promote to pointers */
    array->redirects += 1;
}

void LetDef(FILE* file, Map idents)
{
    Ident ident = LetDec(file);
    ident.params = -1;
    if(Peek(file) == '[')
        Array(file, idents, &ident);
    else
    {
        Value r;
        Match(file, "=");
        r = Expression(file, idents);
        Match(file, ";");
        assert(ident.redirects == 0
            ? r.redirects < 1
            : r.redirects == ident.redirects);
    }
    Insert(idents, ident);
}

void Rewind(FILE* file, char* string)
{
    fseek(file, -strlen(string), SEEK_CUR);
}

void Block(FILE* file, Map idents);

void If(FILE* file, Map idents)
{
    Match(file, "(");
    Expression(file, idents);
    Match(file, ")");
    Block(file, idents);
    while(1)
    {
        String key = { 0 };
        Alpha(file, key);
        if(Equal(key, "elif"))
        {
            Match(file, "(");
            Expression(file, idents);
            Match(file, ")");
            Block(file, idents);
            continue;
        }
        else
        if(Equal(key, "else"))
        {
            Block(file, idents);
            break;
        }
        else
        {
            Rewind(file, key);
            break;
        }
    }
}

void Ret(FILE* file, Map idents)
{
    Expression(file, idents);
    Match(file, ";");
}

void While(FILE* file, Map idents)
{
    Match(file, "(");
    Expression(file, idents);
    Match(file, ")");
    Block(file, idents);
}

void Block(FILE* file, Map idents)
{
    Match(file, "{");
    while(Peek(file) != '}')
    {
        String key = { 0 };
        Alpha(file, key);
        if(Equal(key, "let"))
        {
            Rewind(file, key);
            LetDef(file, idents);
        }
        else
        if(Equal(key, "if"))
            If(file, idents);
        else
        if(Equal(key, "ret"))
            Ret(file, idents);
        else
        if(Equal(key, "while"))
            While(file, idents);
        else
        {
            Rewind(file, key);
            Expression(file, idents);
            Match(file, ";");
        }
    }
    Match(file, "}");
}

int Params(FILE* file, Map idents, String* names)
{
    Match(file, "(");
    if(Peek(file) == ')')
    {
        Match(file, ")");
        return 0;
    }
    else
    {
        int params = 0;
        while(1)
        {
            Ident param = LetDec(file);
            Insert(idents, param);
            strcpy(names[params], param.name);
            params += 1;
            if(Peek(file) == ',')
            {
                Match(file, ",");
                continue;
            }
            else
            if(Peek(file) == ')')
                break;
        }
        Match(file, ")");
        return params;
    }
}

void FunctionDef(FILE* file, Map idents)
{
    int i = 0;
    String params[MAX_FUNCTION_ARGS];
    Ident ident = LetDec(file);
    ident.params = Params(file, idents, params);
    Insert(idents, ident);
    Block(file, idents);
    for(i = 0; i < ident.params; i++)
        Delete(idents, params[i]);
}

void Program(FILE* file, Map idents)
{
    while(!End(file))
        FunctionDef(file, idents);
}

int main(void)
{
    FILE* file = fopen("main.sw", "r");
    Map idents = { 0 };
    Program(file, idents);
    fclose(file);
    return 0;
}
