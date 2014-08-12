/******************************************************************************
 * Copyright (C) 2007 Tetsuya Kimata <kimata@acapulco.dyndns.org>
 *
 * All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must
 *    not claim that you wrote the original software. If you use this
 *    software in a product, an acknowledgment in the product
 *    documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must
 *    not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 * $Id: TemplateLexer.cpp 2907 2009-01-14 12:56:44Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <algorithm>
#include <cstring>
#include <cctype>
#include <iomanip>

#include "apr_strings.h"

#include "TemplateLexer.h"
#include "Message.h"
#include "Auxiliary.h"
#include "Macro.h"
#include "File.h"
#include "TemporaryPool.h"
#include "SourceInfo.h"

using namespace std;

SOURCE_INFO_ADD("$Id: TemplateLexer.cpp 2907 2009-01-14 12:56:44Z svn $");

#define AS_TOKEN(pointer)           reinterpret_cast<token_t *>(pointer)
#define AS_TOKEN_P(pointer)         reinterpret_cast<token_t **>(pointer)

#define IS_INPUT_MATCH(pattern)                                     \
        (((input_.top + LITERAL_STRLEN(pattern)) <= input_.end) &&  \
         (strncmp(input_.top, pattern, LITERAL_STRLEN(pattern)) == 0) && \
         (!isalnum(*(input_.top + LITERAL_STRLEN(pattern)))))
#define PUSH_TOKEN_AND_FORWARD(type, pattern)                       \
    push_token(create_token(type));                                 \
    input_.top += LITERAL_STRLEN(pattern)
#define PUSH_TOKEN_AND_FORWARD1(type)                               \
    push_token(create_token(type));                                 \
    input_.top++;

#ifdef DEBUG_TemplateLexer
#define DUMP_TOKEN_AND_THROW(message)                               \
    dump_token_array(token_array_, id_array_); THROW(message)
#else
#define DUMP_TOKEN_AND_THROW(message)                               \
    THROW(message)
#endif

const char TemplateLexer::CODE_BEGIN_STR[]      = "<!--@";
const char TemplateLexer::CODE_END_STR[]        = "@-->";

const char TemplateLexer::FOREACH_STR[]         = "foreach";
const char TemplateLexer::WHILE_STR[]           = "while";
const char TemplateLexer::IF_STR[]              = "if";
const char TemplateLexer::ELSE_STR[]            = "else";
const char TemplateLexer::PRINT_STR[]           = "print";
const char TemplateLexer::IN_STR[]              = "in";

const char TemplateLexer::ASSIGN_C              = '=';
const char TemplateLexer::PLUS_ASSIGN_STR[]     = "+=";
const char TemplateLexer::MINUS_ASSIGN_STR[]    = "-=";

const char TemplateLexer::VARIABLE_C            = '$';

const char TemplateLexer::EQUAL_STR[]           = "==";
const char TemplateLexer::NOTEQUAL_STR[]        = "!=";
const char TemplateLexer::GREATER_THAN_C        = '>';
const char TemplateLexer::LESS_THAN_C           = '<';

const char TemplateLexer::PLUS_C                = '+';
const char TemplateLexer::PLUS_PLUS_STR[]       = "++";
const char TemplateLexer::MINUS_C               = '-';
const char TemplateLexer::MINUS_MINUS_STR[]     = "--";
const char TemplateLexer::RESIDUE_C             = '%';

const char TemplateLexer::QUOTE_C               = '"';
const char TemplateLexer::ESCAPE_C              = '\\';
const char TemplateLexer::BRACE_LEFT_C          = '{';
const char TemplateLexer::BRACE_RIGHT_C         = '}';
const char TemplateLexer::PAREN_LEFT_C          = '(';
const char TemplateLexer::PAREN_RIGHT_C         = ')';
const char TemplateLexer::BRACKET_LEFT_C        = '[';
const char TemplateLexer::BRACKET_RIGHT_C       = ']';

const char TemplateLexer::DOT_C                 = '.';
const char TemplateLexer::COMMA_C               = ',';
const char TemplateLexer::DELIMITER_C           = ';';

const TemplateLexer::token_type_str TemplateLexer::token_type_list[] = {
    { TemplateLexer::FOREACH,            "foreach"      },
    { TemplateLexer::WHILE,              "while"        },
    { TemplateLexer::IF,                 "if"           },
    { TemplateLexer::ELSE,               "else"         },
    { TemplateLexer::PRINT,              "print"        },
    { TemplateLexer::IN,                 "in"           },
    { TemplateLexer::ASSIGN,             "="            },
    { TemplateLexer::PLUS_ASSIGN,        "+="           },
    { TemplateLexer::MINUS_ASSIGN,       "-="           },
    { TemplateLexer::NOTEQUAL,           "!="           },
    { TemplateLexer::EQUAL,              "=="           },
    { TemplateLexer::GREATER_THAN,       ">"            },
    { TemplateLexer::LESS_THAN,          "<"            },
    { TemplateLexer::PLUS,               "+"            },
    { TemplateLexer::MINUS,              "-"            },
    { TemplateLexer::RESIDUE,            "%"            },
    { TemplateLexer::PLUS_PLUS,          "++"           },
    { TemplateLexer::MINUS_MINUS,        "--"           },
    { TemplateLexer::BRACE_LEFT,         "{"            },
    { TemplateLexer::BRACE_RIGHT,        "}"            },
    { TemplateLexer::PAREN_LEFT,         "("            },
    { TemplateLexer::PAREN_RIGHT,        ")"            },
    { TemplateLexer::BRACKET_LEFT,       "["            },
    { TemplateLexer::BRACKET_RIGHT,      "]"            },
    { TemplateLexer::DOT,                "."            },
    { TemplateLexer::COMMA,              ","            },
    { TemplateLexer::DELIMITER,          ";"            },
    { TemplateLexer::INTEGER,            "integer"      },
    { TemplateLexer::STRING,             "string"       },
    { TemplateLexer::IDENTIFIER,         "identifier"   },
};


/******************************************************************************
 * public メソッド
 *****************************************************************************/
TemplateLexer::TemplateLexer(apr_pool_t *pool, const char *input,
                             apr_size_t size, apr_array_header_t *id_array)
  : pool_(pool),
    input_(input, size),
    id_array_(id_array),
    is_analyzed_(false)
{
    token_array_ = apr_array_make(pool_, 256, sizeof(token_t *));
    if (id_array_ == NULL) {
        id_array_ = apr_array_make(pool_, 32, sizeof(char *));
    }
}

void TemplateLexer::analyze()
{
    if (LIKELY(is_analyzed_)) {
        return;
    }

    while (get_next_token()) ;

    is_analyzed_ = true;
}

TemplateLexer *TemplateLexer::get_instance(apr_pool_t *pool,
                                           const char *tmpl_file_path)
{
    File tmpl_file(pool, tmpl_file_path);
    apr_mmap_t *tmpl_file_map;
    char *tmpl_content;

    tmpl_file.open(APR_READ|APR_BINARY);
    tmpl_file_map = tmpl_file.mmap(0, 0, APR_MMAP_READ, true);

    APR_PALLOC(tmpl_content, char *, pool, tmpl_file_map->size);
    memcpy(tmpl_content, tmpl_file_map->mm, tmpl_file_map->size);

    return new TemplateLexer(pool, tmpl_content, tmpl_file_map->size);
}

void TemplateLexer::dump_token_array(apr_array_header_t *token_array,
                                     apr_array_header_t *id_array)
{
    int i;
    token_t **tokens;

    tokens = AS_TOKEN_P(AVOID_ALIGN_WARN token_array->elts);
    for (i = 0; i < token_array->nelts; i++) {
        dump_token(tokens[i], id_array);
    }
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
void TemplateLexer::get_next_assign_token()
{
#ifdef DEBUG
    if (input_.top == input_.end) {
        DUMP_TOKEN_AND_THROW(MESSAGE_BUG_FOUND);
    }
#endif

    if (((input_.top + 1) != input_.end) &&
        (*(input_.top + 1) == EQUAL_STR[1])) {
        push_token(create_token(EQUAL));
        input_.top += LITERAL_STRLEN(EQUAL_STR);
    } else {
        push_token(create_token(ASSIGN));
        input_.top++;
    }
}

void TemplateLexer::get_next_plus_token()
{
#ifdef DEBUG
    if (input_.top == input_.end) {
        DUMP_TOKEN_AND_THROW(MESSAGE_BUG_FOUND);
    }
#endif

    if ((input_.top + 1) != input_.end) {
        if (*(input_.top + 1) == PLUS_PLUS_STR[1]) {
            push_token(create_token(PLUS_PLUS));
            input_.top += LITERAL_STRLEN(PLUS_PLUS_STR);
            return;
        } else if (*(input_.top+1) == PLUS_ASSIGN_STR[1]) {
            push_token(create_token(PLUS_ASSIGN));
            input_.top += LITERAL_STRLEN(PLUS_ASSIGN_STR);
            return;
        }
    }

    push_token(create_token(PLUS));
    input_.top++;
}

void TemplateLexer::get_next_minus_token()
{
#ifdef DEBUG
    if (input_.top == input_.end) {
        DUMP_TOKEN_AND_THROW(MESSAGE_BUG_FOUND);
    }
#endif

    if ((input_.top + 1) != input_.end) {
        if (*(input_.top + 1) == MINUS_ASSIGN_STR[1]) {
            push_token(create_token(MINUS_MINUS));
            input_.top += LITERAL_STRLEN(MINUS_MINUS_STR);
            return;
        } else if (*(input_.top+1) == MINUS_ASSIGN_STR[1]) {
            push_token(create_token(MINUS_ASSIGN));
            input_.top += LITERAL_STRLEN(MINUS_ASSIGN_STR);
            return;
        }
    }

    push_token(create_token(MINUS));
    input_.top++;
}

void TemplateLexer::get_next_quote_token()
{
    const char *quote_start;
    char *str_start;
    char *str_end;
    const char *pos;

#ifdef DEBUG
    if (input_.top == input_.end) {
        DUMP_TOKEN_AND_THROW(MESSAGE_BUG_FOUND);
    }
#endif

    quote_start = ++input_.top;
    while ((input_.top != input_.end) &&
           !((*(input_.top) == QUOTE_C) &&
             (*(input_.top - 1) != ESCAPE_C))) {
        input_.top++;
    }
    if (input_.top == input_.end) {
        THROW(MESSAGE_TMPL_STRING_ENDED);
    }

    APR_PALLOC(str_start, char *, pool_, input_.top - quote_start + 1);
    str_end = str_start;
    for (pos = quote_start; pos != input_.top; pos++) {
        if (*pos != ESCAPE_C) {
            *(str_end++) = *pos;
        }
    }
    *str_end = '\0';

    push_token(create_str_token(str_start, str_end - str_start));
    input_.top++;
}

void TemplateLexer::get_next_ident_token()
{
    const char *ident_start;

#ifdef DEBUG
    if (input_.top == input_.end) {
        DUMP_TOKEN_AND_THROW(MESSAGE_BUG_FOUND);
    }
#endif

    ident_start = const_cast<char *>(input_.top++);
    while ((input_.top != input_.end) &&
           (isalpha(static_cast<unsigned char>(*(input_.top))) ||
            isdigit(static_cast<unsigned char>(*(input_.top))) ||
            (*(input_.top) == '_'))) {
        input_.top++;
    }

    push_token(create_ident_token(ident_start, input_.top - ident_start));
}

void TemplateLexer::get_next_int_token()
{
    int i;

#ifdef DEBUG
    if (input_.top == input_.end) {
        DUMP_TOKEN_AND_THROW(MESSAGE_BUG_FOUND);
    }
#endif

    i = *(input_.top++) - '0';

    while ((input_.top != input_.end) &&
           (isdigit(static_cast<unsigned char>(*(input_.top))))) {
        i = i*10 + (*(input_.top) - '0');
        input_.top++;
    }

    push_token(create_int_token(i));
}

bool TemplateLexer::get_next_token(bool is_expr_only)
{
    if (UNLIKELY(input_.top == input_.start) && !is_expr_only) {
        return get_next_str_token();
    }

    if (input_.top == input_.end) {
        return false;
    }

    while (isspace(static_cast<unsigned char>(*(input_.top)))) {
        if (++(input_.top) == input_.end) {
            if (is_expr_only) {
                return false;
            } else {
                DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_BROKEN);
            }
        }
    }

    switch (*(input_.top)) {
    case ASSIGN_C:       get_next_assign_token();                   break;
    case PLUS_C:         get_next_plus_token();                     break;
    case MINUS_C:        get_next_minus_token();                    break;
    case QUOTE_C:        get_next_quote_token();                    break;
    case RESIDUE_C:      PUSH_TOKEN_AND_FORWARD1(RESIDUE);          break;
    case GREATER_THAN_C: PUSH_TOKEN_AND_FORWARD1(GREATER_THAN);     break;
    case LESS_THAN_C:    PUSH_TOKEN_AND_FORWARD1(LESS_THAN);        break;
    case BRACE_LEFT_C:   PUSH_TOKEN_AND_FORWARD1(BRACE_LEFT);       break;
    case BRACE_RIGHT_C:  PUSH_TOKEN_AND_FORWARD1(BRACE_RIGHT);      break;
    case PAREN_LEFT_C:   PUSH_TOKEN_AND_FORWARD1(PAREN_LEFT);       break;
    case PAREN_RIGHT_C:  PUSH_TOKEN_AND_FORWARD1(PAREN_RIGHT);      break;
    case BRACKET_LEFT_C: PUSH_TOKEN_AND_FORWARD1(BRACKET_LEFT);     break;
    case BRACKET_RIGHT_C:PUSH_TOKEN_AND_FORWARD1(BRACKET_RIGHT);    break;
    case DOT_C:          PUSH_TOKEN_AND_FORWARD1(DOT);              break;
    case COMMA_C:        PUSH_TOKEN_AND_FORWARD1(COMMA);            break;
    case DELIMITER_C:    PUSH_TOKEN_AND_FORWARD1(DELIMITER);        break;
    default:
        if (IS_INPUT_MATCH(CODE_END_STR)) {
            if (is_expr_only) {
                DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_TOKEN_INVALID);
            } else {
                input_.top += LITERAL_STRLEN(CODE_END_STR);
                return get_next_str_token();
            }
        } else if (IS_INPUT_MATCH(FOREACH_STR)) {
            PUSH_TOKEN_AND_FORWARD(FOREACH, FOREACH_STR);
        } else if (IS_INPUT_MATCH(WHILE_STR)) {
            PUSH_TOKEN_AND_FORWARD(WHILE, WHILE_STR);
        } else if (IS_INPUT_MATCH(IF_STR)) {
            PUSH_TOKEN_AND_FORWARD(IF, IF_STR);
        } else if (IS_INPUT_MATCH(ELSE_STR)) {
            PUSH_TOKEN_AND_FORWARD(ELSE, ELSE_STR);
        } else if (IS_INPUT_MATCH(PRINT_STR)) {
            PUSH_TOKEN_AND_FORWARD(PRINT, PRINT_STR);
        } else if (IS_INPUT_MATCH(IN_STR)) {
            PUSH_TOKEN_AND_FORWARD(IN, IN_STR);
        } else if (IS_INPUT_MATCH(NOTEQUAL_STR)) {
            PUSH_TOKEN_AND_FORWARD(NOTEQUAL, NOTEQUAL_STR);
        } else if (isalpha(static_cast<unsigned char>(*(input_.top)))) {
            get_next_ident_token();
        } else if (isdigit(static_cast<unsigned char>(*(input_.top)))) {
            get_next_int_token();
        } else {
            DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_TOKEN_INVALID);
        }
        break;
    }

    return true;
}

bool TemplateLexer::get_next_str_token()
{
    bool is_code_begin;
    const char *str_end;
    const char *var_start;

#ifdef DEBUG
    if (input_.top == input_.end) {
        DUMP_TOKEN_AND_THROW(MESSAGE_BUG_FOUND);
    }
#endif

    str_end = AS_CONST_CHAR(memmem(input_.top, input_.end - input_.top,
                                   CODE_BEGIN_STR,
                                   LITERAL_STRLEN(CODE_BEGIN_STR)));

    if (str_end == NULL) {
        is_code_begin = false;
        str_end = input_.end;
    } else {
        is_code_begin = true;
    }

    push_token(create_token(PRINT));

    while ((input_.top != str_end) &&
           ((var_start = strnchr(input_.top, str_end - input_.top,
                                 VARIABLE_C)) != NULL)) { // 変数記号を探索
        if ((var_start != input_.top) &&
            *(var_start-1) == ESCAPE_C) { // 直前にエスケープがある場合を除外
            push_token(create_str_token(input_.top, var_start - input_.top-1));
            push_token(create_token(COMMA));
            input_.top = var_start + 1;
            continue;
        }
        if ((var_start+1) == str_end) { // 変数記号で終わるのはおかしい
            DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_VARIABLE_ENDED);
        }

        push_token(create_str_token(input_.top, var_start - input_.top));
        push_token(create_token(COMMA));

        input_.top = var_start + 1;
        get_next_variable(str_end);

        if (input_.top == str_end) {
            goto STR_END;
        }

        push_token(create_token(COMMA));
    }

    push_token(create_str_token(input_.top, str_end - input_.top));

 STR_END:
    if (is_code_begin) {
        input_.top = str_end + LITERAL_STRLEN(CODE_BEGIN_STR);
    } else {
        input_.top = str_end;
    }

    push_token(create_token(DELIMITER));

    return true;
}

void TemplateLexer::get_next_variable(const char *str_end)
{
    const char *var_start;
    const char *var_end;
    const char *real_end;

#ifdef DEBUG
    if (input_.top == str_end) {
        DUMP_TOKEN_AND_THROW(MESSAGE_BUG_FOUND);
    }
#endif

    var_start = input_.top;

    if (*(input_.top) == BRACE_LEFT_C) {//  「${ ... }」形式　
        input_.top++;

        if ((var_end = strnchr(input_.top, str_end - input_.top,
                               BRACE_RIGHT_C)) == NULL) {
            DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_VARIABLE_FORMAT_INVALID);
        }

        real_end = input_.end;
        input_.end = var_end;
        while (get_next_token(true)) ;
        input_.top++;
        input_.end = real_end;
    } else {// 「$...」形式
        if (!isalpha(static_cast<unsigned char>(*(input_.top)))) {
            DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_VARIABLE_NAME_INVALID);
        }
        do {
            input_.top++;
        } while ((input_.top != str_end) &&
                 (isalpha(static_cast<unsigned char>(*(input_.top))) ||
                  isdigit(static_cast<unsigned char>(*(input_.top))) ||
                  (*(input_.top) == '_')));

        push_token(create_ident_token(var_start, input_.top - var_start));
    }
}

apr_size_t TemplateLexer::get_id(const char *str, apr_size_t length)
{
    const char **ids;
    int i;

    ids = AS_CONST_CHAR_P(AVOID_ALIGN_WARN id_array_->elts);
    for (i = 0; i < id_array_->nelts; i++) {
        if (strncmp(ids[i], str, max(strlen(ids[i]), length)) == 0) {
            return i;
        }
    }

    *AS_CONST_CHAR_P(apr_array_push(id_array_))
        = apr_pstrmemdup(id_array_->pool, str, length);

    return static_cast<apr_size_t>(i);
}

void TemplateLexer::push_token(token_t *token)
{
    *AS_TOKEN_P(apr_array_push(token_array_)) = token;
}

TemplateLexer::token_t *TemplateLexer::create_token(token_type_t type)
{
    token_t *token;

    if (UNLIKELY(token_pool_.top == token_pool_.end)) {
        APR_PALLOC(token_pool_.top, token_t *, pool_,
                   sizeof(token_t) * TPL_DEFAULT_TOKEN_POOL_COUNT);
        token_pool_.end = token_pool_.top + TPL_DEFAULT_TOKEN_POOL_COUNT;
    }

    token = token_pool_.top++;
    token->type = type;

    return token;
}

TemplateLexer::token_t *TemplateLexer::create_str_token(const char *str,
                                                        apr_size_t length)
{
    token_t *token;

    token = create_token(STRING);
    token->s = apr_pstrmemdup(pool_, str, length);

    return token;
}

TemplateLexer::token_t *TemplateLexer::create_ident_token(const char *str,
                                                          apr_size_t length)
{
    token_t *token;

    token = create_token(IDENTIFIER);
    token->id = get_id(str, length);

    return token;
}

TemplateLexer::token_t *TemplateLexer::create_int_token(int i)
{
    token_t *token;

    token = create_token(INTEGER);
    token->i = i;

    return token;
}

void TemplateLexer::dump_token(const token_t *token,
                               apr_array_header_t *id_array)
{
    const char *s;

    cerr << setw(60) << setfill('-') << '-' << endl;
    cerr << setw(10) << setfill(' ') << get_token_type(token) << ": ";

    if (token->type == TemplateLexer::IDENTIFIER) {
        cerr << AS_CONST_CHAR_P(AVOID_ALIGN_WARN id_array->elts)[token->id];
        cerr << '(' << token->id << ')';
    } else if (token->type == TemplateLexer::STRING) {
        TemporaryPool temp_pool;
        s = token->s;
        // 空白文字を読み飛ばす
        while (isspace(static_cast<unsigned char>(*s)) && (*s != '\0')) {
            s++;
        }
        cerr << apr_pstrndup(temp_pool.get(), s, 60); // 出力文字数を制限
    } else if (token->type == TemplateLexer::INTEGER) {
        cerr << token->i;
    }
    cerr << endl;
}

const char *TemplateLexer::get_token_type(const token_t *token)
{
    for (apr_size_t i = 0; i < ARRAY_SIZE_OF(token_type_list); i++) {
        if (token->type == token_type_list[i].type) {
            return token_type_list[i].str;
        }
    }
    return "(UNKNOWN)";
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_TemplateLexer
#include "TestRunner.h"

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name << " <TMPL_FILE_PATH>" << endl;
}

void run_analyze(apr_pool_t *pool, const char *tmpl_file_path,
                 apr_size_t dump_level)
{
    TemplateLexer *lexer;

    show_test_name("anlyze");

    lexer = TemplateLexer::get_instance(pool, tmpl_file_path);
    auto_ptr<TemplateLexer> lexer_ap(lexer);

    lexer->analyze();
    if (dump_level > 2) {
        TemplateLexer::dump_token_array(lexer->get_token_array(),
                                        lexer->get_id_array());
    }

    show_item("ident count", lexer->get_id_array()->nelts);
    show_item("token count", lexer->get_token_array()->nelts);
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    const char *tmpl_file_path;
    apr_size_t dump_level;

    if (argc == 1) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    tmpl_file_path = argv[1];
    if (argc >= 3) {
        dump_level = atoi(argv[2]);
    } else {
        dump_level = 0;
    }

    if (!File::is_exist(pool, tmpl_file_path)) {
        THROW(MESSAGE_TMPL_FILE_NOT_FOUND);
    }

    show_item("tmpl_file_path", tmpl_file_path);
    show_line();

    run_analyze(pool, tmpl_file_path, dump_level);
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
