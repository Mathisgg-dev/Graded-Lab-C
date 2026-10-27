// Mathis Guillin-Goutte 260ADB198
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// AI assistance: ChatGPT 
// I used AI to get the outline for this assignment, which helped me know where to start and how to structure my code. Then, for simple functions or small pieces of code, I did them myself. When it got too complicated, I used AI to give me advice or correct me. I also had the AI check the code completely to make sure I hadn't made any mistakes, but the code was fine.
// Conversation link: https://chatgpt.com/share/6ae12fe0-0cf0-83eb-b9f9-ee3896385cd1

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>

// Information for the output files.
#define NAME "mathis"
#define LASTNAME "guillin-goutte"
#define ID "260ADB198"

// A value can be an integer or a float so it will simplificate the code
typedef struct {
    int is_float;
    long long i;
    double d;
} Value;

// Shared lexer state: current character, token, and source positions.
FILE *in;
int c, tok;
long pos, tok_pos, dot_pos, err;
Value num;

// Read a character and go to the next one.
void next_char(void) {
    c = fgetc(in);
    pos++;
}
// This allow to Keep the position of the first error only. 
void fail(long p) {
    if (!err) {
        err = p;
    }
}
// For the tokeniser I was lost so I ask IA for some way to do it, it give me a plan and after some correction by the IA i arrive at this result. 
// Prompt: "I need help for the tokeniser, can you give me some hint"
//Convert the input characters into tokens for the parser.
void next_token(void) {
    if (err) { tok = 'e'; return; }
    if (dot_pos) { tok = '?'; tok_pos = dot_pos; dot_pos = 0; return; }
    while (c == ' ' || c == '\t') next_char();
    tok_pos = pos;
    if (isdigit(c)) {
        tok = 'n';
        num.is_float = 0;
        num.i = 0;
        while (isdigit(c)) { num.i = num.i * 10 + c - '0'; next_char(); }
        if (c == '.') {
            long p = pos;
            next_char();
            if (!isdigit(c)) {
                dot_pos = p;
                return;
            }
            double all = num.i;
            double scale = 1;
            while (isdigit(c)) {
                all = all * 10 + c - '0';
                scale *= 10;
                next_char();
            }
            num.is_float = 1;
            num.d = all / scale;
        }
    } else if (c == '*') {
        next_char();
        if (c == '*') {
            tok = '^';
            next_char();
        } else {
            tok = '*';
        }
    } else if (c == '\n' || c == EOF) tok = 'e';
    else if (strchr("+-/()", c)) {
        tok = c;
        next_char();
    }
    else {
        tok = '?';
    }
}
// the function to calculate the power
long long int_pow(long long b, long long e) {
    long long result = 1;
    for (long long i = 0; i < e; i++) {
        result *= b;
    }
    return result;
}
// Prompt here: "can you correct me" and then I went step by step using IA when I was not sure.
//Apply an operator and report errors such as division by zero.
Value calc(int op, Value a, Value b, long op_pos, long b_pos) {
    Value r = {0, 0, 0};
    if (err) {
        return r;
    }
    if (op == '/' && (b.is_float ? b.d == 0.0 : b.i == 0)) {
        fail(b_pos);
        return r;
    }
    if (!a.is_float && !b.is_float) {
        if (op == '^' && b.i < 0) {
            fail(b_pos);
        }
        else if (op == '+') r.i = a.i + b.i;
        else if (op == '-') r.i = a.i - b.i;
        else if (op == '*') r.i = a.i * b.i;
        else if (op == '/') r.i = a.i / b.i;
        else r.i = int_pow(a.i, b.i);
        return r;
    }
    double x = a.is_float ? a.d : (double)a.i;
    double y = b.is_float ? b.d : (double)b.i;
    if (op == '+') {
        r.d = x + y;
    }
    else if (op == '-') {
        r.d = x - y;
    }
    else if (op == '*') {
        r.d = x * y;
    }
    else if (op == '/') {
        r.d = x / y;
    }
    else {
        r.d = pow(x, y);
    }
    r.is_float = 1;
    if (!isfinite(r.d)) {
        fail(op_pos);
    }
    return r;
}

Value expr(void);

// It read a number or a parenthesized expression.
Value primary(void) {
    Value v = {0, 0, 0};
    if (tok == 'n') {
        v = num;
        next_token();
    }
    else if (tok == '(') {
        next_token();
        v = expr();
        if (tok == ')') {
            next_token();
        } else {
            fail(tok_pos);
        }
    } else {
        fail(tok_pos);
    }
    return v;
}
// manage the power 
Value power(void) {
    Value v = primary();
    if (tok == '^') {
        long op_pos = tok_pos;
        next_token();
        long b_pos = tok_pos;
        v = calc('^', v, power(), op_pos, b_pos);
    }
    return v;
}
// manage the multiplication and division before addition and subtraction.
Value term(void) {
    Value v = power();
    while (tok == '*' || tok == '/') {
        int op = tok;
        long op_pos = tok_pos;
        next_token();
        long b_pos = tok_pos;
        v = calc(op, v, power(), op_pos, b_pos);
    }
    return v;
}
// manage addition and subtraction.
Value expr(void) {
    Value v = term();
    while (tok == '+' || tok == '-') {
        int op = tok;
        long op_pos = tok_pos;
        next_token();
        long b_pos = tok_pos;
        v = calc(op, v, term(), op_pos, b_pos);
    }
    return v;
}
// As the previous functions, IA help me by giving to me advices and some corrections.
//Process each line, skipping blank lines and comments.
void run(FILE *out) {
    next_char();
    while (c != EOF && !err) {
        while (c == ' ' || c == '\t') next_char();
        if (c == '#') {
            while (c != '\n' && c != EOF) next_char();
        } else if (c != '\n' && c != EOF) {
            next_token();
            Value v = expr();
            if (tok != 'e') fail(tok_pos);
            if (!err && v.is_float) fprintf(out, "%.15g\n", v.d);
            else if (!err) fprintf(out, "%lld\n", v.i);
        }
        if (c == '\n') next_char();
    }
}
//As the previous functions, IA help me by giving to me advices and some corrections.
//Extract a file or directory name from its path.
void get_base(const char *path, char *base) {
    size_t n = strlen(path);
    size_t s;
    while (n > 1 && path[n - 1] == '/') n--;
    s = n;
    while (s && path[s - 1] != '/') s--;
    size_t len = n - s;
    if (len > 299) len = 299;
    memcpy(base, path + s, len);
    base[len] = '\0';
    n = strlen(base);
    if (n > 4 && !strcmp(base + n - 4, ".txt")) base[n - 4] = '\0';
}
// this create the output directory.
int make_dir(const char *path) {
    struct stat st;
    if (mkdir(path, 0755) == 0) {
        return 0;
    }
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode) ? 0 : -1;
}
//To be honest, an AI wrote this function for me because I tried but couldn't do it myself, but I made sure to understand everything it wrote.
//Prompt:"Make the proccess_file function which can process an input file"
//Read one input file and write its results to an output file.
int process_file(const char *path, const char *outdir) {
    in = fopen(path, "r");
    if (!in) {
        perror(path);
        return 1;
    }
    if (make_dir(outdir)) {
        perror(outdir);
        fclose(in);
        return 1;
    }
    char base[300], outpath[5000];
    get_base(path, base);
    int n = snprintf(outpath, sizeof outpath, "%s/%s_%s_%s_%s.txt", outdir, base, NAME, LASTNAME, ID);
    if (n < 0 || (size_t)n >= sizeof outpath) {
        fprintf(stderr, "calc: output path too long\n");
        fclose(in);
        return 1;
    }
    FILE *out = fopen(outpath, "w");
    if (!out) {
        perror(outpath);
        fclose(in);
        return 1;
    }
    pos = 0;
    err = 0;
    dot_pos = 0;
    run(out);
    fclose(in);
    if (err) {
        fclose(out);
        out = fopen(outpath, "w");
        if (!out) {
            perror(outpath);
            return 1;
        }
        fprintf(out, "ERROR:%ld\n", err);
    }
    if (fclose(out)) {
        perror(outpath);
        return 1;
    }
    return 0;
}

//As the previous functions, IA help me by giving to me advices and some corrections.
// Process every .txt file directly inside the given directory.
int process_dir(const char *dir, const char *outdir) {
    DIR *d = opendir(dir);
    if (!d) {
        perror(dir);
        return 1;
    }
    if (make_dir(outdir)) {
        perror(outdir);
        closedir(d);
        return 1;
    }
    int rc = 0;
    struct dirent *e;
    while ((e = readdir(d))) {
        size_t n = strlen(e->d_name);
        if (n <= 4 || strcmp(e->d_name + n - 4, ".txt")) {
            continue;
        }
        char path[5000];
        int len = snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
        if (len < 0 || (size_t)len >= sizeof path) {
            rc = 1;
            continue;
        }
        struct stat st;
        if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
            rc |= process_file(path, outdir);
        }
    }
    closedir(d);
    return rc;
}

//As the previous functions, IA help me by giving to me advices and some corrections.
// Read command-line options and choose file or directory mode.
int main(int argc, char **argv) {
    char *dir = NULL;
    char *outdir = NULL;
    char *input = NULL;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--dir")) {
            if (dir || i + 1 >= argc || argv[i + 1][0] == '-') {
                fprintf(stderr, "calc: invalid directory option\n");
                return 1;
            }
            dir = argv[++i];
        } else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output-dir")) {
            if (outdir || i + 1 >= argc || argv[i + 1][0] == '-') {
                fprintf(stderr, "calc: invalid output option\n");
                return 1;
            }
            outdir = argv[++i];
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "calc: unknown option: %s\n", argv[i]);
            return 1;
        }
        else if (input) {
            fprintf(stderr, "calc: multiple input files\n");
            return 1;
        }
        else input = argv[i];
    }
    if ((dir == NULL) == (input == NULL)) {
        fprintf(stderr, "usage: calc [-d DIR | --dir DIR] [-o OUTDIR | --output-dir OUTDIR] [input.txt]\n");
        return 1;
    }
    char base[300], defout[400];
    if (!outdir) {
        get_base(dir ? dir : input, base);
        int n = snprintf(defout, sizeof defout, "%s_%s_%s", base, NAME, ID);
        if (n < 0 || (size_t)n >= sizeof defout) {
            fprintf(stderr, "calc: output directory name too long\n");
            return 1;
        }
        outdir = defout;
    }
    return dir ? process_dir(dir, outdir) : process_file(input, outdir);
}
