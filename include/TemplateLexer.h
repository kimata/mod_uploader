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
 * $Id: TemplateLexer.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef TEMPLATE_LEXER_H
#define TEMPLATE_LEXER_H

#include "Environment.h"

#include "apr_tables.h"

#include "Macro.h"
#include "Uncopyable.h"

#ifdef _MSC_VER
#undef IN
#endif


/**
 * @brief テンプレートを字句解析するクラス．
 *
 * <pre>
 * STRING           ::= '"' (ALPHABET | DIGIT | ...)* '"'
 * IDENTIFIER       ::= ALPHABET (ALPHABET | DIGIT | '_')+
 * INTEGER          ::= ('+' | '-')? (DIGIT)+
 * ASSIGN_OP        ::= '='
 * COMPARE_OP       ::= '==' | '!=' | '>' | '<'
 * ARITHMETIC_OP	::= '+' | '-'
 * MULTIPLY_OP      ::= '%'
 * MONADIC_OP       ::= '++' | '--'
 * ALPHABET         ::= 'a' | 'b' | ... | 'z' | 'A' | 'B' | ... | 'Z'
 * DIGIT            ::= '0' | '1' | ... | '9'
 * </pre>
 */
class TemplateLexer: public Uncopyable
{
public:
    TemplateLexer(apr_pool_t *pool, const char *input, apr_size_t size,
                  apr_array_header_t *id_array=NULL);
    void analyze();
    apr_array_header_t *get_token_array()
    {
        analyze();
        return token_array_;
    };
    apr_array_header_t *get_id_array()
    {
        analyze();
        return id_array_;
    };

    static TemplateLexer *get_instance(apr_pool_t *pool,
                                       const char *file_path);
    static void dump_token_array(apr_array_header_t *token_array,
                                 apr_array_header_t *id_array);

private:
    enum token_type_t {
        FOREACH,                // foreach
        WHILE,                  // while
        IF,                     // if
        ELSE,                   // else
        PRINT,                  // print
        IN,                     // in

        STRING,                 // 文字列
        IDENTIFIER,             // 識別子
        INTEGER,                // 整数

        ASSIGN,                 // =
        PLUS_ASSIGN,            // +=
        MINUS_ASSIGN,           // -=

        EQUAL,                  // ==
        NOTEQUAL,               // !=
        GREATER_THAN,           // >
        LESS_THAN,              // <

        PLUS,                   // +
        MINUS,                  // -
        RESIDUE,                // %
        PLUS_PLUS,              // ++
        MINUS_MINUS,            // --

        BRACE_LEFT,             // {
        BRACE_RIGHT,            // }
        PAREN_LEFT,             // (
        PAREN_RIGHT,            // )
        BRACKET_LEFT,           // [
        BRACKET_RIGHT,          // ]

        DOT,                    // .
        COMMA,                  // ,
        DELIMITER,              // ;

        DEFAULT
    };

    typedef struct Token {
        token_type_t type;
        union {
            apr_size_t id;
            int i;
            const char *s;
        };
    } token_t;

    static const char CODE_BEGIN_STR[];
    static const char CODE_END_STR[];

    static const char FOREACH_STR[];
    static const char WHILE_STR[];
    static const char IF_STR[];
    static const char ELSE_STR[];
    static const char PRINT_STR[];
    static const char IN_STR[];

    static const char ASSIGN_C;
    static const char PLUS_ASSIGN_STR[];
    static const char MINUS_ASSIGN_STR[];

    static const char VARIABLE_C;

    static const char EQUAL_STR[];
    static const char NOTEQUAL_STR[];
    static const char GREATER_THAN_C;
    static const char LESS_THAN_C;

    static const char PLUS_C;
    static const char PLUS_PLUS_STR[];
    static const char MINUS_C;
    static const char MINUS_MINUS_STR[];
    static const char RESIDUE_C;

    static const char QUOTE_C;
    static const char ESCAPE_C;
    static const char BRACE_LEFT_C;
    static const char BRACE_RIGHT_C;
    static const char PAREN_LEFT_C;
    static const char PAREN_RIGHT_C;
    static const char BRACKET_LEFT_C;
    static const char BRACKET_RIGHT_C;

    static const char DOT_C;
    static const char COMMA_C;
    static const char DELIMITER_C;

    typedef struct Input {
        const char *top;
        const char *start;
        const char *end;

        Input(const char *input, apr_size_t size)
          : top(AS_CONST_CHAR(input)),
            start(top),
            end(start + size)
        {

        }
    } input_t;

    typedef struct TokenPool {
        token_t *top;
        token_t *end;

        TokenPool()
          : top(NULL),
            end(NULL)
       {

       }
    } token_pool_t;

    typedef struct {
        token_type_t type;
        const char *str;
    } token_type_str;

    static const token_type_str token_type_list[];

    bool get_next_token(bool is_expr_only=false);
    bool get_next_str_token();
    void get_next_variable(const char *str_end);

    void get_next_assign_token();
    void get_next_plus_token();
    void get_next_minus_token();
    void get_next_quote_token();
    void get_next_ident_token();
    void get_next_int_token();

    apr_size_t get_id(const char *str, apr_size_t length);

    void push_token(token_t *token);

    token_t *create_token(token_type_t type=DEFAULT);
    token_t *create_str_token(const char *str, apr_size_t length);
    token_t *create_ident_token(const char *str, apr_size_t length);
    token_t *create_int_token(int i);

    static void dump_token(const token_t *token, apr_array_header_t *id_array);
    static const char *get_token_type(const token_t *token);

    friend class TemplateParser;

    apr_pool_t *pool_;

    input_t input_;

    apr_array_header_t *token_array_;
    apr_array_header_t *id_array_;

    token_pool_t token_pool_;

    bool is_analyzed_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
