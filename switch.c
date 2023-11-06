#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX_MAP_IDENTS (512)
#define MAX_FUNCTION_PARAMS (8)
#define MAX_STRING_CHARS (64)
#define MAX_SCOPE_LOCALS (64)

#define RUNTIME_VARIABLE_STACK_SIZE (65536)
#define RUNTIME_FUNCTION_STACK_SIZE (512)

char* RelationalOps[] = {
    "==", "!=", "<", ">", "<=", ">=", NULL
};

char* TermOps[] = {
    "=", "+", "-", "&", "^", "|", NULL
};

char* FactOps[] = {
    "*", "%", "/", NULL
};

char* UnaryOps[] = {
    "!", "$", "~", "+", "-", "&", "*", "(", NULL
};

char* Reserved[] = {
    "int", "while", "if", "elif", "else", "ret", NULL
};

typedef char String[MAX_STRING_CHARS];

typedef enum
{
    TYPE_ARR = -2,
    TYPE_VAR = -1,
    /* 0 or more, where the value implies the number of function params */
    TYPE_FUN = 0
}
Type;

typedef struct
{
    String name;
    /* -1 for direct values, 0 for variables, 1 or more for pointers */
    int redirects;
    int sp;
    /* 1, otherwise N for arrays */
    int size;
    /* 0 or more imply identifier is function */
    int params;
    int param_redirects[MAX_FUNCTION_PARAMS];
}
Ident;

/* open address hashing */
typedef struct
{
    Ident ident[MAX_MAP_IDENTS];
    int sp;
    int label;
}
Map;

typedef struct
{
    /* defaults as l-value */
    int right;
    int redirects;
}
Value;

void Quit(const char* format, ...)
{
    va_list list;
    va_start(list, format);
    fprintf(stderr, "error: ");
    vfprintf(stderr, format, list);
    fprintf(stderr, "\n");
    va_end(list);
    exit(1);
}

int Equal(char* a, char* b)
{
    return strcmp(a, b) == 0;
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

/* generic */
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

Ident* Find(Map* idents, char* name)
{
    int i;
    int index;
    if(IsStringIn(name, Reserved))
        Quit("%s is a reserved keyword", name);
    index = Index(name);
    for(i = 0; i < MAX_MAP_IDENTS; i++)
    {
        Ident* at = &idents->ident[index];
        if(Equal(at->name, name))
            return at;
        index = Next(index);
    }
    return NULL;
}

void Insert(Map* idents, Ident* ident)
{
    int i;
    int index;
    Ident* found = Find(idents, ident->name);
    if(found)
        Quit("%s already defined", found->name);
    index = Index(ident->name);
    for(i = 0; i < MAX_MAP_IDENTS; i++)
    {
        Ident* at = &idents->ident[i];
        if(at->name[0] == '\0')
        {
            *at = *ident;
            break;
        }
        index = Next(index);
    }
}

void Delete(Map* idents, char* name)
{
    Ident* ident = Find(idents, name);
    if(ident)
        ident->name[0] = '\0';
}

int IsTermOp(char* op)
{
    return IsStringIn(op, TermOps)
        || IsStringIn(op, RelationalOps);
}

int IsFactOp(char* op)
{
    return IsStringIn(op, FactOps);
}

int IsTermOpChar(int c)
{
    return IsCharIn(c, TermOps)
        || IsCharIn(c, RelationalOps);
}

int IsFactOpChar(int c)
{
    return IsCharIn(c, FactOps);
}

int Read(FILE* in)
{
    int c = fgetc(in);
    /* python style comments */
    if(c == '#')
        while(c != '\n')
            c = fgetc(in);
    return c;
}

void Stream(FILE* in, int clause(int), String string)
{
    int i = 0;
    char c;
    while(clause(c = Read(in)))
    {
        if(i >= MAX_STRING_CHARS)
            Quit("max string size %d exceeded", MAX_STRING_CHARS);
        /* will not fill white space buffers */
        string[clause == isspace ? 0 : i++] = c;
    }
    ungetc(c, in);
    if(i >= MAX_STRING_CHARS)
        Quit("max string size %d exceeded", MAX_STRING_CHARS);
    string[i] = '\0';
}

void Space(FILE* in)
{
    String string;
    Stream(in, isspace, string);
}

int Peek(FILE* in)
{
    Space(in);
    return ungetc(Read(in), in);
}

void Alpha(FILE* in, String string)
{
    Space(in);
    Stream(in, isalpha, string);
}

int isalnumu(int c)
{
    return isalnum(c) || c == '_';
}

void Alnumu(FILE* in, String string)
{
    Space(in);
    Stream(in, isalnumu, string);
}

void Digit(FILE* in, String string)
{
    Space(in);
    Stream(in, isdigit, string);
}

void TermOp(FILE* in, String string)
{
    Space(in);
    Stream(in, IsTermOpChar, string);
}

void FactOp(FILE* in, String string)
{
    Space(in);
    Stream(in, IsFactOpChar, string);
}

void Match(FILE* in, char* with)
{
    Space(in);
    while(*with)
    {
        int c = Read(in);
        if(*with != c)
            Quit("expected %c but got %c", *with, c);
        with += 1;
    }
}

Value Expression(FILE* in, FILE* out, Map* idents);

void AssignCheck(int l, int r)
{
    if(l == 0)
    {
        if(r > l)
            Quit("pointers cannot be assigned to variables");
    }
    else
    {
        if(l != r)
            Quit("pointer redirection level must match");
    }
}

void OpCheck(int l, int r)
{
    if(l > 0 && r > 0)
    {
        if(l != r)
            Quit("pointer redirection level must match");
    }
}

Value Rval(FILE* out, Value value)
{
    if(value.right == 0)
    {
        fprintf(out, "\tRVL();\n");
        value.right = 1;
    }
    return value;
}

void Args(FILE* in, FILE* out, Map* idents, Ident* ident)
{
    int args = 0;
    Match(in, "(");
    if(Peek(in) == ')')
        Match(in, ")");
    else
    {
        while(1)
        {
            Value r;
            if(args >= ident->params)
                Quit("%s(...) param count of %d exceeded", ident->name, ident->params);
            r = Rval(out, Expression(in, out, idents));
            OpCheck(ident->param_redirects[args], r.redirects);
            args += 1;
            if(Peek(in) == ',')
            {
                Match(in, ",");
                continue;
            }
            if(Peek(in) == ')')
                break;
        }
        Match(in, ")");
    }
}

Value Identifier(FILE* in, FILE* out, Map* idents)
{
    Value value = { 0 };
    String alnum;
    Ident* ident;
    Alnumu(in, alnum);
    ident = Find(idents, alnum);
    if(ident == NULL)
        Quit("%s not defined", alnum);
    value.redirects = ident->redirects;
    if(Peek(in) == '(') /* function */
    {
        if(ident->params < TYPE_FUN)
            Quit("expected %s to be a function", ident->name);
        fprintf(out, "\tSET();\n");
        Args(in, out, idents, ident);
        fprintf(out, "\tCAL(%s);\n", ident->name);
    }
    else
        fprintf(out, "\tREF(%d); /* %s */\n", ident->sp, ident->name);
    if(ident->params == TYPE_ARR) value.right = 1;
    if(ident->params == TYPE_VAR) value.right = 0;
    if(ident->params >= TYPE_FUN) value.right = 1;
    return value;
}

Value Char(FILE* in, FILE* out)
{
    char ch[3] = { '\0' };
    Value value = { 1, -1 };
    Match(in, "'");
    if(Peek(in) == '\\') /* escape chars */
    {
        Match(in, "\\");
        ch[0] = '\\';
        ch[1] = Read(in);
    }
    else
        ch[0] = Read(in);
    Match(in, "'");
    fprintf(out, "\tINT('%s');\n", ch);
    return value;
}

Value Number(FILE* in, FILE* out)
{
    String number;
    Value value = { 1, -1 };
    Digit(in, number);
    fprintf(out, "\tINT(%s);\n", number);
    return value;
}

Value Factor(FILE* in, FILE* out, Map* idents);

Value Deref(FILE* in, FILE* out, Map* idents)
{
    Value value;
    Match(in, "*");
    value = Rval(out, Factor(in, out, idents));
    if(value.redirects <= 0)
        Quit("could not dereference");
    value.redirects -= 1;
    value.right = 0;
    return value;
}

Value Ref(FILE* in, FILE* out, Map* idents)
{
    Value value;
    Match(in, "&");
    value = Factor(in, out, idents);
    if(value.right != 0)
        Quit("expected lvalue");
    value.redirects += 1;
    value.right = 1;
    return value;
}

Value Paren(FILE* in, FILE* out, Map* idents)
{
    Value value;
    Match(in, "(");
    value = Expression(in, out, idents);
    Match(in, ")");
    return value;
}

Value Neg(FILE* in, FILE* out, Map* idents)
{
    Value value;
    Match(in, "-");
    value = Rval(out, Factor(in, out, idents));
    if(value.redirects > 0)
        Quit("pointer arithmetic not allowed");
    fprintf(out, "\tINT(-1);\n");
    fprintf(out, "\tMUL();\n");
    return value;
}

Value Pos(FILE* in, FILE* out, Map* idents)
{
    Match(in, "+");
    return Rval(out, Factor(in, out, idents));
}

Value Flp(FILE* in, FILE* out, Map* idents)
{
    Value value;
    Match(in, "~");
    value = Rval(out, Factor(in, out, idents));
    if(value.redirects > 0)
        Quit("pointer arithmetic not allowed");
    fprintf(out, "\tFLP();\n");
    return value;
}

Value Prt(FILE* in, FILE* out, Map* idents)
{
    Value value;
    Match(in, "$");
    value = Rval(out, Factor(in, out, idents));
    fprintf(out, "\tPRT();\n");
    return value;
}

Value Not(FILE* in, FILE* out, Map* idents)
{
    Value value;
    Match(in, "!");
    value = Rval(out, Factor(in, out, idents));
    fprintf(out, "\tNOT();\n");
    return value;
}

Value Unary(FILE* in, FILE* out, Map* idents)
{
    Value none = { 0 };
    int c = Peek(in);
    if(!IsCharIn(c, UnaryOps))
        Quit("%c is not a supported unary operator", c);
    if(c == '$')
        return Prt(in, out, idents);
    else
    if(c == '!')
        return Not(in, out, idents);
    else
    if(c == '~')
        return Flp(in, out, idents);
    else
    if(c == '+')
        return Pos(in, out, idents);
    else
    if(c == '-')
        return Neg(in, out, idents);
    else
    if(c == '&')
        return Ref(in, out, idents);
    else
    if(c == '*')
        return Deref(in, out, idents);
    else
    if(c == '(')
        return Paren(in, out, idents);
    return none;
}

Value Factor(FILE* in, FILE* out, Map* idents)
{
    int c = Peek(in);
    if(c == '\'')
        return Char(in, out);
    else
    if(isdigit(c))
        return Number(in, out);
    else
    if(isalnumu(c))
        return Identifier(in, out, idents);
    else
        return Unary(in, out, idents);
}

Value Term(FILE* in, FILE* out, Map* idents)
{
    String op;
    Value l = Factor(in, out, idents);
    for(FactOp(in, op); IsFactOp(op); FactOp(in, op))
    {
        Value r;
        l = Rval(out, l);
        r = Rval(out, Factor(in, out, idents));
        OpCheck(l.redirects, r.redirects);
        if(Equal(op, "*"))
            fprintf(out, "\tMUL();\n");
        else
        if(Equal(op, "%"))
            fprintf(out, "\tMOD();\n");
        else
        if(Equal(op, "/"))
            fprintf(out, "\tDIV();\n");
        l.redirects = MAX(l.redirects, r.redirects);
        l.right = 1;
    }
    return l;
}

Value Expression(FILE* in, FILE* out, Map* idents)
{
    String op;
    Value l = Term(in, out, idents);
    Value r;
    for(TermOp(in, op); IsTermOp(op); TermOp(in, op))
    {
        if(Equal(op, "="))
        {
            /* remain as lvalue for left side */
            if(l.right == 1)
                Quit("expected lvalue");
            /* rvalue moves into lvalue */
            r = Rval(out, Expression(in, out, idents));
            AssignCheck(l.redirects, r.redirects);
            fprintf(out, "\tMOV();\n");
            fprintf(out, "\tDRP();\n");
        }
        else
        if(IsStringIn(op, RelationalOps))
        {
            l = Rval(out, l);
            r = Rval(out, Expression(in, out, idents));
            OpCheck(l.redirects, r.redirects);
            if(Equal(op, "=="))
                fprintf(out, "\tEQT();\n");
            else
            if(Equal(op, "!="))
                fprintf(out, "\tNEQ();\n");
            else
            if(Equal(op, "<"))
                fprintf(out, "\tLTH();\n");
            else
            if(Equal(op, ">"))
                fprintf(out, "\tGTH();\n");
            else
            if(Equal(op, "<="))
                fprintf(out, "\tLTE();\n");
            else
            if(Equal(op, ">="))
                fprintf(out, "\tGTE();\n");
        }
        else
        {
            l = Rval(out, l);
            r = Rval(out, Term(in, out, idents));
            OpCheck(l.redirects, r.redirects);
            if(Equal(op, "+"))
                fprintf(out, "\tADD();\n");
            else
            if(Equal(op, "-"))
                fprintf(out, "\tSUB();\n");
            else
            if(Equal(op, "&"))
                fprintf(out, "\tAND();\n");
            else
            if(Equal(op, "^"))
                fprintf(out, "\tXOR();\n");
            else
            if(Equal(op, "|"))
                fprintf(out, "\tORR();\n");
        }
        l.redirects = MAX(l.redirects, r.redirects);
        l.right = 1;
    }
    return l;
}

void Typeify(Ident* ident, Type type)
{
    ident->params = type;
    if(ident->params == TYPE_ARR) ident->size = 0;
    if(ident->params == TYPE_VAR) ident->size = 1;
    if(ident->params >= TYPE_FUN) ident->size = 1;
}

Ident Int(FILE* in, Map* idents, Type type)
{
    char first;
    Ident ident = { 0 };
    String decl;
    Alpha(in, decl);
    if(!Equal(decl, "int"))
        Quit("only type int is supported");
    while(Peek(in) == '*')
    {
        Match(in, "*");
        ident.redirects += 1;
    }
    Alnumu(in, ident.name);
    first = ident.name[0];
    if(!isalpha(first) && first != '_')
        Quit("%s must start with letter or underscore", ident.name);
    ident.sp = idents->sp;
    Typeify(&ident, type);
    return ident;
}

int End(FILE* in)
{
    Space(in);
    return Peek(in) == EOF;
}

int Array(FILE* in, FILE* out, Map* idents, Ident* ident)
{
    int expressions = 0;
    int size;
    String digit;
    Match(in, "[");
    Digit(in, digit);
    size = atoi(digit);
    Match(in, "]");
    Match(in, "=");
    Match(in, "{");
    while(1)
    {
        Value r;
        if(Peek(in) == '}')
            break;
        r = Rval(out, Expression(in, out, idents));
        expressions += 1;
        AssignCheck(ident->redirects, r.redirects);
        if(Peek(in) == ',')
            Match(in, ",");
        else
            break;
    }
    Match(in, "}");
    Match(in, ";");
    if(size == 0)
    {
        if(expressions == 0)
            Quit("empty brace for array %s", ident->name);
        size = expressions;
    }
    else
    {
        if(expressions == 1)
        {
            int i;
            for(i = expressions; i < size; i++)
                fprintf(out, "\tDUP();\n");
        }
        else
        if(size != expressions)
            Quit("brace size of %d does not match array %s size of %d", expressions, ident->name, size);
    }
    return size;
}

void Rewind(FILE* in, char* string)
{
    fseek(in, -strlen(string), SEEK_CUR);
}

void Block(FILE* in, FILE* out, Map* idents);

void If(FILE* in, FILE* out, Map* idents)
{
    int l0 = idents->label++;
    int lx = idents->label++;
    Match(in, "(");
    Rval(out, Expression(in, out, idents));
    Match(in, ")");
    fprintf(out, "\tBRZ(L%d);\n", l0);
    Block(in, out, idents);
    fprintf(out, "\tBRA(L%d);\n", lx);
    fprintf(out, "L%d:\n", l0);
    while(1)
    {
        String key = { 0 };
        Alpha(in, key);
        if(Equal(key, "elif"))
        {
            int l1 = idents->label++;
            Match(in, "(");
            Rval(out, Expression(in, out, idents));
            Match(in, ")");
            fprintf(out, "\tBRZ(L%d);\n", l1);
            Block(in, out, idents);
            fprintf(out, "\tBRA(L%d);\n", lx);
            fprintf(out, "L%d:\n", l1);
            continue;
        }
        else
        if(Equal(key, "else"))
        {
            Block(in, out, idents);
            break;
        }
        else
        {
            Rewind(in, key);
            break;
        }
    }
    fprintf(out, "L%d:\n", lx);
}

void Ret(FILE* in, FILE* out, Map* idents)
{
    Rval(out, Expression(in, out, idents));
    fprintf(out, "\tRET();\n");
    Match(in, ";");
}

void While(FILE* in, FILE* out, Map* idents)
{
    int l0 = idents->label++;
    int lx = idents->label++;
    fprintf(out, "L%d:\n", l0);
    Match(in, "(");
    Rval(out, Expression(in, out, idents));
    fprintf(out, "\tBRZ(L%d);\n", lx);
    Match(in, ")");
    Block(in, out, idents);
    fprintf(out, "\tBRA(L%d);\n", l0);
    fprintf(out, "L%d:\n", lx);
}

void Block(FILE* in, FILE* out, Map* idents)
{
    int i;
    int count = 0;
    String locals[MAX_SCOPE_LOCALS];
    Match(in, "{");
    while(Peek(in) != '}')
    {
        String key = { 0 };
        Alpha(in, key);
        if(Equal(key, "int"))
        {
            Ident ident;
            Rewind(in, key);
            /* assume variable */
            ident = Int(in, idents, TYPE_VAR);
            if(Peek(in) == '[')
            {
                /* but override with array if necessary */
                Typeify(&ident, TYPE_ARR);
                ident.size = Array(in, out, idents, &ident);
                ident.redirects += 1;
            }
            else
            {
                Value r;
                Match(in, "=");
                r = Rval(out, Expression(in, out, idents));
                Match(in, ";");
                AssignCheck(ident.redirects, r.redirects);
            }
            idents->sp += ident.size;
            Insert(idents, &ident);
            if(count >= MAX_SCOPE_LOCALS)
                Quit("max scope local count of %d has been reached", MAX_SCOPE_LOCALS);
            strcpy(locals[count], ident.name);
            count += 1;
        }
        else
        if(Equal(key, "if"))
            If(in, out, idents);
        else
        if(Equal(key, "ret"))
            Ret(in, out, idents);
        else
        if(Equal(key, "while"))
            While(in, out, idents);
        else /* plain expression with no assignment */
        {
            Rewind(in, key);
            Expression(in, out, idents);
            Match(in, ";");
            fprintf(out, "\tPOP(); /* no assignment */\n");
        }
    }
    Match(in, "}");
    for(i = 0; i < count; i++)
    {
        Ident* found = Find(idents, locals[i]);
        idents->sp -= found->size;
        Delete(idents, found->name);
        fprintf(out, "\tPOP();\n");
    }
}

Ident Params(FILE* in, Map* idents, String* names)
{
    Ident ident = Int(in, idents, TYPE_FUN);
    Match(in, "(");
    if(Peek(in) == ')')
    {
        Match(in, ")");
        return ident;
    }
    else
    {
        while(1)
        {
            Ident param = Int(in, idents, TYPE_VAR);
            Insert(idents, &param);
            if(ident.params >= MAX_FUNCTION_PARAMS)
                Quit("exceeded %s param count of %d", ident.name, ident.params);
            strcpy(names[ident.params], param.name);
            ident.param_redirects[ident.params] = param.redirects;
            ident.params += param.size;
            idents->sp += param.size;
            if(Peek(in) == ',')
            {
                Match(in, ",");
                continue;
            }
            else
            if(Peek(in) == ')')
                break;
        }
        Match(in, ")");
        return ident;
    }
}

void Func(FILE* in, FILE* out, Map* idents)
{
    int i = 0;
    String params[MAX_FUNCTION_PARAMS];
    Ident ident = Params(in, idents, params);
    fprintf(out, "%s:\n", ident.name);
    Insert(idents, &ident);
    Block(in, out, idents);
    for(i = 0; i < ident.params; i++)
    {
        Ident* found = Find(idents, params[i]);
        idents->sp -= found->size;
        Delete(idents, found->name);
        fprintf(out, "\tPOP();\n");
    }
    if(idents->sp != 0)
        Quit("internal : Func() : idents->sp = %d", idents->sp);
    fprintf(out, "\tINT(0);\n");
    fprintf(out, "\tRET();\n");
}

void Header(FILE* out)
{
    /* stdlib standard headers for generic io */
    fprintf(out,
        "int putchar(int c);\n"
    );
    /* call stack setup, calling, and returning */
    fprintf(out,
        "#define POP() --sp\n"

        "#define SET()"
            "bs[fp] = sp\n"

        "#define CAL(name)"
            "fs[fp] = __LINE__;"
            "fp++;"
            "goto name;"
            "case __LINE__:\n"

        "#define RET()"
            /* otherwise, to the return register */
            "rr = vs[sp - 1];"
            "--fp;"
            "sp = bs[fp];"
            "fa = fs[fp];"
            "vs[sp] = rr;"
            /* functions always return vars of size 1 */
            "sp++;"
            "goto begin\n"
        );

    /* direct / indirect loads */
    fprintf(out,
        "#define REF(ref) vs[sp++] = ref + bs[fp - 1]\n"
        "#define DRP() vs[sp - 2] = vs[sp - 1]; --sp;\n"
        "#define MOV() vs[vs[sp - 2]] = vs[sp - 1];\n"
        "#define RVL() vs[sp - 1] = vs[vs[sp - 1]]\n"
        "#define DUP() sp++; vs[sp - 1] = vs[sp - 2];\n"
        /* int is the only type! */
        "#define INT(var) vs[sp++] = var\n"
    );
    /* unary operators */
    fprintf(out,
        "#define PRT() putchar(vs[sp - 1])\n"
        "#define NOT() vs[sp - 1] = vs[sp - 1] == 0 ? 1 : 0\n"
        "#define FLP() vs[sp - 1] = ~vs[sp - 1]\n"
        "#define POS() vs[sp - 1] = +vs[sp - 1]\n"
        "#define NEG() vs[sp - 1] = -vs[sp - 1]\n"
    );
    /* term and factor operators */
    fprintf(out,
        "#define ADD() vs[sp - 2]  += vs[sp - 1]; --sp\n"
        "#define SUB() vs[sp - 2]  -= vs[sp - 1]; --sp\n"
        "#define AND() vs[sp - 2]  &= vs[sp - 1]; --sp\n"
        "#define XOR() vs[sp - 2]  ^= vs[sp - 1]; --sp\n"
        "#define ORR() vs[sp - 2]  |= vs[sp - 1]; --sp\n"
        "#define MUL() vs[sp - 2]  *= vs[sp - 1]; --sp\n"
        "#define DIV() vs[sp - 2]  /= vs[sp - 1]; --sp\n"
        "#define MOD() vs[sp - 2] %%= vs[sp - 1]; --sp\n"
    );
    /* relational operators */
    fprintf(out,
        "#define EQT() vs[sp - 2] = vs[sp - 2] == vs[sp - 1]; --sp\n"
        "#define NEQ() vs[sp - 2] = vs[sp - 2] != vs[sp - 1]; --sp\n"
        "#define LTH() vs[sp - 2] = vs[sp - 2] <  vs[sp - 1]; --sp\n"
        "#define GTH() vs[sp - 2] = vs[sp - 2] >  vs[sp - 1]; --sp\n"
        "#define LTE() vs[sp - 2] = vs[sp - 2] <= vs[sp - 1]; --sp\n"
        "#define GTE() vs[sp - 2] = vs[sp - 2] >= vs[sp - 1]; --sp\n"
    );
    /* branching */
    fprintf(out,
        "#define BRZ(label) if(vs[--sp] == 0) goto label\n"
        "#define BRA(label) goto label\n"
    );
    /* entry + stack + register setup */
    fprintf(out,
        "int main(void) {\n"
        "\tint fs[%d] = { 0 };\n" /* function stack: top is prev function address */
        "\tint bs[%d] = { 0 };\n" /* base pointer stack: top is prev stack pointer */
        "\tint vs[%d] = { 0 };\n" /* variable stack */
        "\tregister int fp = 1;\n" /* frame pointer */
        "\tregister int fa = 0;\n" /* function address */
        "\tregister int sp = 0;\n" /* stack pointer */
        "\tregister int rr = 0;\n" /* return register */
        "\tgoto main;\n"
        "begin:\n"
        "\tif(fp == 0)\n"
            "\t\treturn vs[sp - 1];\n"
        "\tswitch(fa)\n"
        "\t{\n",
            RUNTIME_FUNCTION_STACK_SIZE,
            RUNTIME_FUNCTION_STACK_SIZE,
            RUNTIME_VARIABLE_STACK_SIZE
    );
}

void Footer(FILE* out)
{
    fprintf(out, "\t}\n\treturn 0;\n}");
}

void Program(FILE* in, FILE* out, Map* idents)
{
    Header(out);
    while(!End(in))
        Func(in, out, idents);
    Footer(out);
}

int main(int argc, char** argv)
{
    Map idents = { 0 };
    FILE* in;
    FILE* out;
    if(argc != 3)
    {
        puts("./switch in.sw, out.c");
        exit(1);
    }
    in = fopen(argv[1], "r");
    out = fopen(argv[2], "w");
    Program(in, out, &idents);
    fclose(in);
    fclose(out);
    return 0;
}
