#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cctype>

using U0 = void;
using U8 = std::uint8_t;
using U16 = std::uint16_t;
using U32 = std::uint32_t;
using U64 = std::uint64_t;

using I0 = void;
using I8 = std::int8_t;
using I16 = std::int16_t;
using I32 = std::int32_t;
using I64 = std::int64_t;

using F0 = void;
using F32 = float;
using F64 = double;

typedef enum {
        TK_EOF,
        TK_IDENT,
        TK_KEYWORD,
        TK_STR,
        TK_INT,
        TK_PLUS,
        TK_MINUS,
        TK_STAR,
        TK_DIV,
} TKType;

typedef struct {
        TKType type;
        std::string value;
} TK;

std::vector<TK> lex(std::string text) {
        std::vector<TK> res;
        U64 len = text.size();
        U64 i = 0;
        while (i < len) {
                char ch = text[i];
                if (std::isspace(ch)) {
                        i++;
                        continue;
                } else if (isalpha(ch) || ch == '_') {
                        std::string id_val;
                        while (i < len && (isalnum(text[i]))) {
                                id_val += text[i];
                                i++;
                        }
                        if (id_val == "Print") {
                                res.emplace_back(TK{TKType::TK_KEYWORD, id_val});
                        } else {
                                res.emplace_back(TK{TKType::TK_IDENT, id_val});
                        }
                } else if (ch == '"') {
                        std::string str_val;
                        i++;
                        while (i < len && text[i] != '"') {
                                str_val += text[i];
                                i++;
                        }
                        if (i >= len || text[i] != '"') {
                                /*#############
                                #### ERROR ####
                                #############*/
                                std::cout << "Expected '\"' at Ch: " << i + 1 << std::endl;;
                                exit(1);
                        } else {
                                res.emplace_back(TK{TKType::TK_STR, str_val});
                        }
                        i++;
                        continue;
                } else if (isdigit(ch)) {
                        std::string num_val;
                        while (i < len && isdigit(text[i])) {
                                num_val += text[i];
                                i++;
                        }
                        res.emplace_back(TK{TKType::TK_INT, num_val});
                        continue;
                } else if (ch == '+') {
                        res.emplace_back(TK{TKType::TK_PLUS, "+"});
                        i++;
                        continue;
                } else if (ch == '-') {
                        res.emplace_back(TK{TKType::TK_MINUS, "-"});
                        i++;
                        continue;
                } else if (ch == '*') {
                        res.emplace_back(TK{TKType::TK_STAR, "*"});
                        i++;
                        continue;
                } else if (ch == '/') {
                        res.emplace_back(TK{TKType::TK_DIV, "/"});
                        i++;
                        continue;
                } else {
                        /*#############
                        #### ERROR ####
                        #############*/
                        std::cout << "too lazy" << std::endl;
                        exit(1);
                }
        }
        res.emplace_back(TK{TKType::TK_EOF, "EOF"});
        return res;
}

/*
____SPHINX PRECENDENCE____

11 `,>>,<<
10 *,/,%
9  &
8  ^
7  |
6  +,-
5  <,>,<=,>=
4  ==, !=
3  &&
2  ^^
1  ||
0  =,<<=,>>=,*=,/=,%=,&=,|=,^=,+=,-=

__________________________
*/

U8 prec(TK op) {
        switch (op.type) {
                // exp bitwise

                case TKType::TK_STAR:
                case TKType::TK_DIV:
                // mod
                        return 10;
                // stuff
                case TKType::TK_PLUS:
                case TKType::TK_MINUS:
                        return 6;
                default:
                        return 0;
        }
}

void parse(std::vector<TK> &tks) {
        U64 ahead = 0;
        U64 i = 0;
        U8 negA = 0;
        while (i < tks.size() - 1) {
                TK tk = tks[i];
                if (tk.type == TKType::TK_MINUS) {
                        negA = 1 - negA;
                        tks.erase(tks.begin());
                        std::cout << "negA\n";
                        continue;
                }
                if (tk.type == TKType::TK_INT) { // >INT< ___ ___
                        I64 numA = std::stoll(tk.value);
                        if (negA != 0) { numA = -numA; std::cout << "-numA\n"; }
                        U8 negB = 0;
                        // ##################
                        // #### ADDITION ####
                        // ##################
                        if (i + 1 < tks.size() - 1 && tks[i + 1].type == TKType::TK_PLUS) { // INT >PLUS< ___
                                if (i + 3 < tks.size() - 1 && prec(tks[i + 3]) > 6) { // 6 IS PREC OF ADDITION
                                        i+=2;
                                        ahead++;
                                        continue;
                                }
                                if (i + 2 < tks.size() - 1 && tks[i + 2].type == TKType::TK_INT) { // INT PLUS >INT<
                                        I64 numB = std::stoll(tks[i + 2].value);
                                        I64 numC = numA + numB;
                                        auto it = tks.erase(tks.begin() + i, tks.begin() + i + 3);
                                        tks.insert(it, TK{TKType::TK_INT, std::to_string(numC)});
                                        if (ahead > 0) {
                                                i -= 2;
                                                ahead--;
                                        }
                                        continue;
                                } else {
                                        std::cout << "\tINT PLUS >___<\n\tExpected INT!" << std::endl;
                                        i+=2;
                                        continue;
                                }
                        }
                        // #####################
                        // #### SUBTRACTION ####
                        // #####################
                        if (i + 1 < tks.size() - 1 && tks[i + 1].type == TKType::TK_MINUS) { // INT >MINUS< ___
                                if (i + 3 < tks.size() - 1 && prec(tks[i + 3]) > 6) { // 6 IS PREC OF SUBTRACTION
                                        i+=2;
                                        ahead++;
                                        continue;
                                }
                                if (i + 2 < tks.size() - 1 && tks[i + 2].type == TKType::TK_INT) { // INT MINUS >INT<
                                        I64 numB = std::stoll(tks[i + 2].value);
                                        I64 numC = numA - numB;
                                        auto it = tks.erase(tks.begin() + i, tks.begin() + i + 3);
                                        tks.insert(it, TK{TKType::TK_INT, std::to_string(numC)});
                                        if (ahead > 0) {
                                                i-=2;
                                                ahead--;
                                        }
                                        continue;
                                } else {
                                        std::cout << "\tINT MINUS >___<\n\tExpected INT!" << std::endl;
                                        i+=2;
                                        continue;
                                }
                        }
                        // ########################
                        // #### MULTIPLICATION ####
                        // ########################
                        if (i + 1 < tks.size() - 1 && tks[i + 1].type == TKType::TK_STAR) { // INT >STAR< ___
                                if (i + 3 < tks.size() - 1 && prec(tks[i + 3]) > 10) { // 10 IS PREC OF MULTIPLICATION
                                        i+=2;
                                        ahead++;
                                        continue;
                                }
                                if (i + 2 < tks.size() - 1 && tks[i + 2].type == TKType::TK_INT) { // INT STAR >INT<
                                        I64 numB = std::stoll(tks[i + 2].value);
                                        I64 numC = numA * numB;
                                        auto it = tks.erase(tks.begin() + i, tks.begin() + i + 3);
                                        tks.insert(it, TK{TKType::TK_INT, std::to_string(numC)});
                                        if (ahead > 0) {
                                                i-=2;
                                                ahead--;
                                        }
                                        continue;
                                } else {
                                        std::cout << "\tINT STAR >___<\n\tExpected INT!" << std::endl;
                                        i+=2;
                                        continue;
                                }
                        }
                        // ##################
                        // #### DIVISION ####
                        // ##################
                        if (i + 1 < tks.size() - 1 && tks[i + 1].type == TKType::TK_DIV) { // INT >DIV< ___
                                if (i + 3 < tks.size() - 1 && prec(tks[i + 3]) > 10) { // 10 IS PREC OF DIVISION
                                        i+=2;
                                        ahead++;
                                        continue;
                                }
                                while (i + 2 < tks.size() - 1 && tks[i + 2].type == TKType::TK_MINUS) { // INT DIV MINUS ___
                                        negB = 1 - negB;
                                        std::cout << "neg\n";
                                        i++;
                                }
                                if (i + 2 < tks.size() - 1 && tks[i + 2].type == TKType::TK_INT) { // INT DIV >INT<
                                        I64 numB = std::stoll(tks[i + 2].value);
                                        if (negB != 0) { numB = -numB; }
                                        if (numB == 0) {
                                                std::cout << "\tCannot divide by 0!" << std::endl;
                                                i+=2;
                                                continue;
                                        }
                                        I64 numC = numA / numB;
                                        auto it = tks.erase(tks.begin() + i, tks.begin() + i + 3);
                                        tks.insert(it, TK{TKType::TK_INT, std::to_string(numC)});
                                        if (ahead > 0) {
                                                i-=2;
                                                ahead--;
                                        }
                                        continue;
                                }
                                std::cout << "\tINT DIV >___<\n\tExpected INT!" << std::endl;
                                i+=2;
                                continue;
                        }
                        // ###############
                        if (ahead > 0) {
                                // Jump Back
                                i-=2;
                                ahead--;
                                continue;
                        }
                        // Must be lone int
                        i++;
                        continue;
                } else {
                        i++;
                }
        }
}


I32 main() {
        std::string input;
        while (1) {
                std::cout << "sphinx=> ";
                std::getline(std::cin, input);
                if (input == "exit") {
                        break;
                }
                std::vector<TK> output = lex(input);
                //std::cout << "#### LEXER ####\n";
                //for (const auto &tk : output) {
                //      std::cout << "[" << tk.type << ":'" << tk.value << "'], ";
                //}
                std::cout << "\t_____OUTPUT_____\n\n";
                parse(output);
                std::cout << "\t";
                for (const auto &tk : output) {
                         std::cout << tk.value << " ";
                }
                std::cout << "\n";
        }
        return 0;
}

/*

Parsing
IDK

1 + 2 * 3 ^ 4 + 5
SEES +, checks for higher prec.
SEES *, checks for higher prec.
SEES ^, checks for higher prec. Sees (lower) or equal prec so evals.
1 + 2 * 81 + 5          # Technically could implement it remembering the previous precs.
SEES +, checks for higher prec.
SEES *, checks for higher prec. Sees (lower) or equal prec so evals.
1 + 162 + 5
SEES +, checks for higher prec. Sees lower or (equal) prec so evals.
163 + 5
SEES +, checks for higher prec. Only operation so evals.
= 168

*/
