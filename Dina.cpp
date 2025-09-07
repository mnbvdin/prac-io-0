#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// Лексемы
enum type_of_lex {
    // TW:
    LEX_NULL, // 0
    LEX_AND,
    LEX_BOOL,
    LEX_DO,
    LEX_ELSE,
    LEX_IF,
    LEX_FALSE,
    LEX_INT, // 7
    LEX_NOT,
    LEX_OR,
    LEX_PROGRAM,
    LEX_READ,
    LEX_TRUE,
    LEX_WHILE,
    LEX_WRITE,
    LEX_CONTINUE,
    LEX_STR, // 16
    // TD:
    LEX_FIN, // 17
    LEX_SEMICOLON,
    LEX_COMMA,
    LEX_COLON,
    LEX_ASSIGN,
    LEX_LPAREN,
    LEX_RPAREN,
    LEX_EQ,
    LEX_LSS, // 25
    LEX_GTR,
    LEX_PLUS,
    LEX_MINUS,
    LEX_TIMES,
    LEX_SLASH,
    LEX_LEQ,
    LEX_NEQ,
    LEX_GEQ,
    LEX_LBRACK,
    LEX_RBRACK, // 35
    //
    LEX_NUM,       // 36
    LEX_ID,        // 37
    LEX_CONSTR,    // 38
    POLIZ_LABEL,   // 39
    POLIZ_ADDRESS, // 40
    POLIZ_GO,      // 41
    POLIZ_FGO,     // 42
};

class Lex {
    type_of_lex t_lex;
    int v_lex;
    string s_lex;

public:
    Lex(type_of_lex t = LEX_NULL, int v = 0, string s = "")
        : t_lex(t), v_lex(v), s_lex(s) {}
    type_of_lex get_type() const { return t_lex; }
    void put_type(type_of_lex t) { t_lex = t; }
    int get_value() const { return v_lex; }
    void put_value(int v) { v_lex = v; }
    string get_string() const { return s_lex; }
    void put_string(string s) { s_lex = s; }
};
class Ident {
    string name;

public:
    bool operator==(const string& s) const { return name == s; }
    Ident(const string n) { name = n; }
    string get_name() const { return name; }
    void put_name(const string n) { name = n; }
};

vector<Ident> TID;
int put(const string& buf) {
    vector<Ident>::iterator k;
    if ((k = find(TID.begin(), TID.end(), buf)) != TID.end())
        return k - TID.begin();
    TID.push_back(Ident(buf));
    return TID.size() - 1;
}

// Лексический анализ

class Scanner {
    FILE* file;
    char c;
    int look(const string buffer, const char** list) {
        int i = 0;
        while (list[i]) {
            if (buffer == list[i])
                return i;
            ++i;
        }
        return 0;
    }
    void gc() { c = fgetc(file); }

public:
    static const char* TW[], * TD[];
    Scanner(const char* program) {
        if (!(file = fopen(program, "r")))
            throw "INVALID FILE";
    }
    ~Scanner() { fclose(file); }
    Lex get_lex() {
        enum state { H, IDENT, NUMB, SLASH, COM1, COM2, ALE, NEQ, STR };
        int d, j;
        string buffer;
        state CS = H;
        do {
            gc();
            switch (CS) {
            case H:
                if (c == EOF)
                    return Lex(LEX_FIN);
                if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
                    ;
                else if (isalpha(c)) {
                    buffer.push_back(c);
                    CS = IDENT;
                }
                else if (isdigit(c)) {
                    d = c - '0';
                    CS = NUMB;
                }
                else if (c == '/') {
                    buffer.push_back(c);
                    CS = SLASH;
                }
                else if (c == '=' || c == '<' || c == '>') {
                    buffer.push_back(c);
                    CS = ALE;
                }
                else if (c == '!') {
                    buffer.push_back(c);
                    CS = NEQ;
                }
                else if (c == '"') {
                    CS = STR;
                }
                else if (c == '@') {
                    return Lex(LEX_FIN);
                }
                else {
                    buffer.push_back(c);
                    if ((j = look(buffer, TD))) {
                        return Lex((type_of_lex)(j + (int)LEX_FIN), j);
                    }
                    else
                        throw c;
                }
                break;
            case IDENT:
                if (isalpha(c) || isdigit(c)) {
                    buffer.push_back(c);
                }
                else {
                    ungetc(c, file);
                    if ((j = look(buffer, TW))) {
                        return Lex((type_of_lex)j, j);
                    }
                    else {
                        j = put(buffer);
                        return Lex(LEX_ID, j);
                    }
                }
                break;
            case NUMB:
                if (isdigit(c)) {
                    d = d * 10 + (c - '0');
                }
                else {
                    ungetc(c, file);
                    return Lex(LEX_NUM, d);
                }
                break;
            case SLASH:
                if (c == '*') {
                    buffer.pop_back();
                    CS = COM1;
                }
                else if (c == '@') {
                    throw c;
                }
                else {
                    ungetc(c, file);
                    j = look(buffer, TD);
                    return Lex((type_of_lex)(j + (int)LEX_FIN), j);
                }
                break;
            case COM1:

                if (c == '*') {
                    CS = COM2;
                }
                else if (c == '@') {
                    throw c;
                }
                break;
            case COM2:

                if (c == '/') {
                    CS = H;
                }
                else if (c == '@') {
                    throw c;
                }
                else if (c != '*') {
                    CS = COM1;
                }
                break;
            case ALE:
                if (c == '=') {
                    buffer.push_back(c);
                    j = look(buffer, TD);
                    return Lex((type_of_lex)(j + (int)LEX_FIN), j);
                }
                else {
                    ungetc(c, file);
                    j = look(buffer, TD);
                    return Lex((type_of_lex)(j + (int)LEX_FIN), j);
                }
                break;
            case NEQ:
                if (c == '=') {
                    buffer.push_back(c);
                    j = look(buffer, TD);
                    return Lex(LEX_NEQ, j);
                }
                else
                    throw '!';
                break;
            case STR:
                if (c == '"') {
                    string res = "";
                    for (int i = 0; i < (int)buffer.size(); ++i) {
                        res += buffer[i];
                    }
                    return Lex(LEX_CONSTR, 0, res);
                }
                else if (c == EOF || c == '@')
                    throw '@';
                buffer.push_back(c);
                break;
            }
        } while (true);
    }
};

const char* Scanner::TW[] = { "",         "and",    "boolean", "do",    "else",
                             "if",       "false",  "int",     "not",   "or",
                             "program",  "read",   "true",    "while", "write",
                             "continue", "string", NULL };

const char* Scanner::TD[] = { "@",  ";",  ",",  ":", "=", "(", ")",
                             "==", "<",  ">",  "+", "-", "*", "/",
                             "<=", "!=", ">=", "{", "}", NULL };

ostream& operator<<(ostream& s, Lex l) {
    string t, t1;
    type_of_lex type = l.get_type();
    if (type <= LEX_STR) {
        t = Scanner::TW[type];
        t1 = ", TW position: ";
    }
    else if (type >= LEX_FIN && type <= LEX_RBRACK) {
        t = Scanner::TD[type - LEX_FIN];
        t1 = ", TD position: ";
    }
    else if (type == LEX_NUM) {
        t = "NUMBER";
        t1 = ", value: ";
    }
    else if (type == LEX_ID) {
        t = TID[l.get_value()].get_name();
        t1 = ", TID position: ";
    }
    else if (type == LEX_CONSTR) {
        t = "STRING CONST";
        t1 = ", value: ";
        s << t << t1 << l.get_string() << endl;
        return s;
    }
    else
        throw l;
    s << t << t1 << l.get_value() << endl;
    return s;
}

// Переменные



int main() {
    char* program = "test.txt";
    try {
        Scanner s = Scanner(program);
        Lex lex;
        vector<Lex> Lexs;
        while ((lex = s.get_lex()).get_type() != LEX_FIN) {
            Lexs.push_back(lex);
            std::cout << lex;
        };
    }
    catch (char c) {
        std::cerr << "INVALID SYMBOL: " << c << endl;
        return -1;
    }
    catch (const char* s) {
        std::cerr << s << endl;
        return -1;
    }
    printf("\nTID table\n\n");
    for (std::vector<Ident>::iterator it = TID.begin(); it != TID.end(); ++it) {
        std::cout << (*it).get_name() << ", TID position: " << it - TID.begin()
            << std::endl;
    }
}