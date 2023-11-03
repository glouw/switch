#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX_MAP_IDENTS (512)
#define MAX_FUNCTION_ARGS (8)
#define MAX_STRING_CHARS (64)
#define MAX_RUNTIME_VARIABLES (65536)
#define MAX_RUNTIME_FUNCTIONS (512)
#define MAX_SCOPE_LOCALS (64)

static int SP = 0;

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
    "$", "~", "+", "-", "&", "*", "(", NULL
};

char* Reserved[] = {
    "let", "while", "if", "elif", "else", "ret", NULL
};

typedef char String[MAX_STRING_CHARS];

typedef struct
{
    String name;
    int redirects;
    int sp;
    /* -1 for variables, 0 or more for functions */
    int params;
    int param_redirects[MAX_FUNCTION_ARGS];
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

void Insert(Map idents, Ident* ident)
{
    int i;
    int index;
    Ident* found = Find(idents, ident->name);
    assert(!found);
    index = Index(ident->name);
    for(i = 0; i < MAX_MAP_IDENTS; i++)
    {
        Ident* at = &idents[i];
        if(at->name[0] == '\0')
        {
            *at = *ident;
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
    /* Python style comments */
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
        assert(i < MAX_STRING_CHARS);
        /* No need to fill buffer with spaces */
        string[clause == isspace ? 0 : i++] = c;
    }
    ungetc(c, in);
    assert(i < MAX_STRING_CHARS);
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
        assert(*with++ == Read(in));
}

Value Expression(FILE* in, FILE* out, Map idents);


void Check(int l, int r)
{
    assert(l == 0 ? r < 1 : r == l);
}

void Args(FILE* in, FILE* out, Map idents, Ident* ident)
{
    int args = 0;
    Match(in, "(");
    if(Peek(in) == ')')
    {
        Match(in, ")");
        return;
    }
    else
    {
        while(1)
        {
            Value r;
            assert(args < ident->params);
            r = Expression(in, out, idents);
            Check(ident->redirects, r.redirects);
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

Value Identifier(FILE* in, FILE* out, Map idents)
{
    Value value = { 0 };
    String alnum;
    Ident* ident;
    Alnumu(in, alnum);
    ident = Find(idents, alnum);
    assert(ident);
    value.redirects = ident->redirects;
    if(Peek(in) == '(')
    {
        assert(ident->params >= 0);
        Args(in, out, idents, ident);
        fprintf(out, "\tCAL(%s);\n", ident->name);
    }
    else
        fprintf(out, "\tPSH(%d); /* %s */\n", ident->sp, ident->name);
    return value;
}

Value Char(FILE* in, FILE* out)
{
    int c;
    Value value = { 1, -1 };
    Match(in, "'");
    if(Peek(in) == '\\')
        Match(in, "\\");
    c = Read(in);
    Match(in, "'");
    fprintf(out, "\tPSH('%c');\n", c);
    return value;
}

Value Number(FILE* in, FILE* out)
{
    String number;
    Value value = { 1, -1 };
    Digit(in, number);
    fprintf(out, "\tPSH(%s);\n", number);
    return value;
}

Value Factor(FILE* in, FILE* out, Map idents);

Value Deref(FILE* in, FILE* out, Map idents)
{
    Value value;
    Match(in, "*");
    value = Factor(in, out, idents);
    assert(value.redirects > 0);
    value.redirects -= 1;
    value.right = 0;
    fprintf(out, "\tNOP(); /* is a NOP for deref okay? */\n");
    return value;
}

Value Ref(FILE* in, FILE* out, Map idents)
{
    Value value;
    Match(in, "&");
    value = Factor(in, out, idents);
    assert(value.right == 0);
    assert(value.redirects >= 0);
    value.redirects += 1;
    value.right = 1;
    fprintf(out, "\tNOP(); /* is a NOP for ref okay? */\n");
    return value;
}

Value Paren(FILE* in, FILE* out, Map idents)
{
    Value value;
    Match(in, "(");
    value = Expression(in, out, idents);
    Match(in, ")");
    return value;
}

Value Neg(FILE* in, FILE* out, Map idents)
{
    Value value;
    Match(in, "-");
    value = Factor(in, out, idents);
    assert(value.redirects <= 0);
    fprintf(out, "\tPSH(-1);\n");
    fprintf(out, "\tMUL();\n");
    return value;
}

Value Pos(FILE* in, FILE* out, Map idents)
{
    Value value;
    Match(in, "+");
    value = Factor(in, out, idents);
    assert(value.redirects <= 0);
    fprintf(out, "\tNOP();\n");
    return value;
}

Value Inv(FILE* in, FILE* out, Map idents)
{
    Value value;
    Match(in, "~");
    value = Factor(in, out, idents);
    assert(value.redirects <= 0);
    fprintf(out, "\tINV();\n");
    return value;
}

Value Print(FILE* in, FILE* out, Map idents)
{
    Value value;
    Match(in, "$");
    value = Factor(in, out, idents);
    fprintf(out, "\tPRT();\n");
    return value;
}

Value Unary(FILE* in, FILE* out, Map idents)
{
    Value none = { 0 };
    int c = Peek(in);
    assert(IsCharIn(c, UnaryOps));
    if(c == '$')
        return Print(in, out, idents);
    else
    if(c == '~')
        return Inv(in, out, idents);
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
    assert(1 && "uncaught unary operator");
    return none;
}

Value Factor(FILE* in, FILE* out, Map idents)
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

void Get(FILE* out)
{
    fprintf(out, "\tGET();\n");
}

void Add(FILE* out)
{
    fprintf(out, "\tADD();\n");
}

void Sub(FILE* out)
{
    fprintf(out, "\tSUB();\n");
}

void And(FILE* out)
{
    fprintf(out, "\tAND();\n");
}

void Xor(FILE* out)
{
    fprintf(out, "\tXOR();\n");
}

void Or(FILE* out)
{
    fprintf(out, "\tORR();\n");
}

void Mul(FILE* out)
{
    fprintf(out, "\tMUL();\n");
}

void Div(FILE* out)
{
    fprintf(out, "\tDIV();\n");
}

void Mod(FILE* out)
{
    fprintf(out, "\tMOD();\n");
}

void Assign(FILE* out)
{
    fprintf(out, "\tMOV();\n");
}

void EqualTo(FILE* out)
{
    fprintf(out, "\tEQT();\n");
}

void NotEqualTo(FILE* out)
{
    fprintf(out, "\tNEQ();\n");
}

void LessThan(FILE* out)
{
    fprintf(out, "\tLTH();\n");
}

void GreaterThan(FILE* out)
{
    fprintf(out, "\tGTH();\n");
}

void LessThanOrEqualTo(FILE* out)
{
    fprintf(out, "\tLTE();\n");
}

void GreaterThanOrEqualTo(FILE* out)
{
    fprintf(out, "\tGTE();\n");
}

Value Term(FILE* in, FILE* out, Map idents)
{
    String op;
    Value l = Factor(in, out, idents);
    for(FactOp(in, op); IsFactOp(op); FactOp(in, op))
    {
        Value r = Factor(in, out, idents);
        if(l.redirects > 0)
            assert(r.redirects <= 0);
        if(r.redirects > 0)
            assert(l.redirects <= 0);
        if(Equal(op, "*"))
            Mul(out);
        else
        if(Equal(op, "%"))
            Mod(out);
        else
        if(Equal(op, "/"))
            Div(out);
        l.redirects = MAX(l.redirects, r.redirects);
        l.right = 1;
    }
    return l;
}

Value Expression(FILE* in, FILE* out, Map idents)
{
    String op;
    Value l = Term(in, out, idents);
    Value r;
    for(TermOp(in, op); IsTermOp(op); TermOp(in, op))
    {
        if(Equal(op, "="))
        {
            assert(l.right == 0);
            r = Expression(in, out, idents);
            Check(l.redirects, r.redirects);
            Assign(out);
        }
        else
        if(IsStringIn(op, RelationalOps))
        {
            if(l.redirects > -1)
                Get(out);
            r = Expression(in, out, idents);
            if(r.redirects > -1)
                Get(out);

            if(Equal(op, "=="))
                EqualTo(out);
            else
            if(Equal(op, "!="))
                NotEqualTo(out);
            else
            if(Equal(op, "<"))
                LessThan(out);
            else
            if(Equal(op, ">"))
                GreaterThan(out);
            else
            if(Equal(op, "<="))
                LessThanOrEqualTo(out);
            else
            if(Equal(op, ">="))
                GreaterThanOrEqualTo(out);
        }
        else
        {
            if(l.redirects > -1)
                Get(out);
            r = Term(in, out, idents);
            if(l.redirects > 0)
                assert(r.redirects <= 0);
            if(r.redirects > 0)
                assert(l.redirects <= 0);
            if(r.redirects > -1)
                Get(out);

            if(Equal(op, "+"))
                Add(out);
            else
            if(Equal(op, "-"))
                Sub(out);
            else
            if(Equal(op, "&"))
                And(out);
            else
            if(Equal(op, "^"))
                Xor(out);
            else
            if(Equal(op, "|"))
                Or(out);
        }
        l.redirects = MAX(l.redirects, r.redirects);
        l.right = 1;
    }
    return l;
}

Ident LetDec(FILE* in, int local)
{
    char first;
    Ident ident = { 0 };
    String let;
    Alpha(in, let);
    assert(Equal(let, "let"));
    while(Peek(in) == '*')
    {
        Match(in, "*");
        ident.redirects += 1;
    }
    Alnumu(in, ident.name);
    first = ident.name[0];
    assert(isalpha(first) || first == '_');
    if(local)
    {
        ident.sp = SP;
        SP += 1;
    }
    return ident;
}

int End(FILE* in)
{
    Space(in);
    return Peek(in) == EOF;
}

void Array(FILE* in, FILE* out, Map idents, Ident* array)
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
        r = Expression(in, out, idents);
        expressions += 1;
        Check(array->redirects, r.redirects);
        if(Peek(in) == ',')
            Match(in, ",");
        else
            break;
    }
    Match(in, "}");
    Match(in, ";");
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

void LetDef(FILE* in, FILE* out, Map idents, Ident* ident)
{
    ident->params = -1;
    if(Peek(in) == '[')
        Array(in, out, idents, ident);
    else
    {
        Value r;
        Match(in, "=");
        r = Expression(in, out, idents);
        Match(in, ";");
        Check(ident->redirects, r.redirects);
    }
    Insert(idents, ident);
}

void Rewind(FILE* in, char* string)
{
    fseek(in, -strlen(string), SEEK_CUR);
}

void Block(FILE* in, FILE* out, Map idents);

void If(FILE* in, FILE* out, Map idents)
{
    Match(in, "(");
    Expression(in, out, idents);
    Match(in, ")");
    Block(in, out, idents);
    while(1)
    {
        String key = { 0 };
        Alpha(in, key);
        if(Equal(key, "elif"))
        {
            Match(in, "(");
            Expression(in, out, idents);
            Match(in, ")");
            Block(in, out, idents);
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
}

void Ret(FILE* in, FILE* out, Map idents)
{
    Expression(in, out, idents);
    fprintf(out, "\tRET();\n");
    Match(in, ";");
}

void While(FILE* in, FILE* out, Map idents)
{
    Match(in, "(");
    Expression(in, out, idents);
    Match(in, ")");
    Block(in, out, idents);
}

void Block(FILE* in, FILE* out, Map idents)
{
    int i;
    int count = 0;
    String locals[MAX_SCOPE_LOCALS];
    Match(in, "{");
    while(Peek(in) != '}')
    {
        String key = { 0 };
        Alpha(in, key);
        if(Equal(key, "let"))
        {
            Ident ident;
            Rewind(in, key);
            ident = LetDec(in, 1);
            LetDef(in, out, idents, &ident);
            assert(count < MAX_SCOPE_LOCALS);
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
        Delete(idents, locals[i]);
        SP -= 1;
    }
}

Ident Params(FILE* in, Map idents, String* names)
{
    Ident ident = LetDec(in, 0);
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
            Ident param = LetDec(in, 1);
            Insert(idents, &param);
            assert(ident.params < MAX_FUNCTION_ARGS);
            strcpy(names[ident.params], param.name);
            ident.param_redirects[ident.params] = param.redirects;
            ident.params += 1;
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

void Func(FILE* in, FILE* out, Map idents)
{
    int i = 0;
    String params[MAX_FUNCTION_ARGS];
    Ident ident = Params(in, idents, params);
    fprintf(out, "%s:\n", ident.name);
    Insert(idents, &ident);
    Block(in, out, idents);
    for(i = 0; i < ident.params; i++)
    {
        Delete(idents, params[i]);
        SP -= 1;
    }
    assert(SP == 0);
    fprintf(out, "\tPSH(0);\n");
    fprintf(out, "\tRET();\n");
}

void Header(FILE* out)
{
    fprintf(out,
        "#define MID() stack[sp - 2]\n"
        "#define TOP() stack[sp - 1]\n"
        "#define CAL(name) func[fp++] = __LINE__; goto name; case __LINE__:\n"
        "#define RET() if(fp == 0) return TOP(); else { pc = func[--fp]; goto begin; }\n"
        "#define PSH(var) stack[sp++] = var\n"
        "#define POP() --sp\n"
        "#define GET() TOP() = stack[TOP()]\n"
        "#define ADD() MID() += TOP(); POP();\n"
        "#define SUB() MID() -= TOP(); POP();\n"
        "#define PRT() putchar(TOP());\n");

    fprintf(out,
        "int main(void)\n"
        "{\n"
        "\tint func[%d];\n"
        "\tint stack[%d];\n"
        "\tint fp = 0;\n"
        "\tint pc = 0;\n"
        "\tint sp = 0;\n"
        "\tgoto main;\n"
        "begin:\n"
        "\tswitch(pc)\n"
        "\t{\n",
            MAX_RUNTIME_FUNCTIONS,
            MAX_RUNTIME_VARIABLES);
}

void Footer(FILE* out)
{
    fprintf(out, "\t}\n\treturn 0;\n}");
}

void Program(FILE* in, FILE* out, Map idents)
{
    Header(out);
    while(!End(in))
        Func(in, out, idents);
    Footer(out);
}

int main(void)
{
    FILE* in = fopen("main.sw", "r");
    FILE* out = fopen("out.c", "w");
    Map idents = { 0 };
    Program(in, out, idents);
    fclose(in);
    fclose(out);
    return 0;
}
