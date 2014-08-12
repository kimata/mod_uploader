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
 * $Id: TemplateParser.h 2873 2008-04-27 14:19:47Z svn $
 *****************************************************************************/

#ifndef TEMPLATEPARSER_H
#define TEMPLATEPARSER_H

#include "Environment.h"

#include "TemplateLexer.h"
#include "Uncopyable.h"


/**
 * @brief テンプレートをパースするクラス．
 *
 * ASSIGN と VARIABLE 周りで手を抜いているので汚いです．
 *
 * <pre>
 * PROGRAM    ::= STMT_LIST
 * STMT_LIST  ::= STMT*
 * STMT_LIST  ::= STMT_LIST STMT | STMT
 * STMT       ::= FOREACH | WHILE | IF | PRINT | EXPR ';'
 * FOREACH    ::= 'foreach' '(' IDENTIFIER 'in' VARIABLE ')' '{' STMT_LIST  '}'
 * WHILE      ::= 'while' '(' COMPARE ')' '{' STMT_LIST  '}'
 * IF         ::= 'if' '(' COMPARE ')' '{' STMT_LIST '}' ('else' '{' STMT_LIST '}')?
 * PRINT      ::= 'print' EXPR_LIST ';'
 * EXPR_LIST  ::= EXPR (',' EXPR)*
 * EXPR       ::= ASSIGN | COMPARE
 * ASSIGN     ::= IDENTIFIER ASSIGN_OP COMPARE
 * COMPARE    ::= ARITHMETIC COMPARE_OP ARITHMETIC | ARITHMETIC
 * ARITHMETIC ::= ARITHMETIC '+' MULTIPLY | ARITHMETIC '-' MULTIPLY |
 *                MULTIPLY
 * MULTIPLY   ::= MULTIPLY '%' TERM |
 *                TERM
 * TERM       ::= MONADIC_OP VARIABLE | VARIABLE MONADIC_OP |
 *                VARIABLE | INTEGER | '(' EXPR ')'
 * VARIABLE   ::= IDENTIFIER |
 *                IDENTIFIER '.' IDENTIFIER |
 *                IDENTIFIER '[' (INTEGER | IDENTIFIER) ']' |
 *                IDENTIFIER '[' (INTEGER | IDENTIFIER) ']' '.' IDENTIFIER
 * </pre>
 *
 * 左再帰を排除したバージョン．
 *
 * <pre>
 * PROGRAM    ::= STMT_LIST
 * STMT_LIST  ::= STMT*
 * STMT       ::= FOREACH | IF | PRINT | EXPR ';'
 * FOREACH    ::= 'foreach' '(' IDENTIFIER 'in' VARIABLE ')' '{' STMT_LIST  '}'
 * WHILE      ::= 'while' '(' COMPARE ')' '{' STMT_LIST  '}'
 * IF         ::= 'if' '(' ARITHMETIC COMPARE_ ')' '{' STMT_LIST '}' ELSE_
 * ELSE_      ::= 'else' '{' STMT_LIST '}' | ε
 * PRINT      ::= 'print' EXPR_LIST ';'
 * EXPR_LIST  ::= EXPR (',' EXPR)*
 * EXPR       ::= ASSIGN | COMPARE
 * ASSIGN     ::= IDENTIFIER ASSIGN_OP COMPARE
 * COMPARE    ::= ARITHMETIC COMPARE_
 * COMPARE_   ::= COMPARE_OP ARITHMETIC | ε
 * ARITHMETIC ::= MULTIPLY ARITHMETIC_
 * ARITHMETIC_::= '+' MULTIPLY ARITHMETIC_ | '-' MULTIPLY ARITHMETIC_ | ε
 * MULTIPLY   ::= TERM MULTIPLY_
 * MULTIPLY_  ::= '%' TERM MULTIPLY_ | ε
 * TERM       ::= MONADIC_OP VARIABLE | VARIABLE MONADIC_OP |
 *                VARIABLE | INTEGER | STRING | '(' EXPR ')'
 * VARIABLE   ::= IDENTIFIER VARIABLE_
 * VARIABLE_  ::= '.' IDENTIFIER | '[' (INTEGER | IDENTIFIER) ']' VARIABLE__ | ε
 * VARIABLE__ ::= '.' IDENTIFIER | ε
 * </pre>
 */
class TemplateParser: public Uncopyable
{
public:
    enum node_type_t {
        FOREACH,                // foreach
        WHILE,                  // while
        IF,                     // if
        PRINT,                  // print

        STRING,                 // 文字列
        IDENTIFIER,             // 識別子
        KEY,                    // キー
        INTEGER,                // 整数

        ARRAY_REF,              // []
        HASH_REF,               // .

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

        BANK,

        DEFAULT
    };

    struct Node;

    typedef struct Branch {
        const struct Node *left;
        const struct Node *center;
        const struct Node *right;

        Branch()
          : left(NULL),
            center(NULL),
            right(NULL)
        {

        }
    } branch_t;

    typedef struct Node {
        node_type_t type;
        branch_t child;
        union {
            apr_size_t id;
            int i;
            struct {
                const char *s;
                apr_size_t l;
            };
        };
#ifdef DEBUG_TemplateParser
            apr_size_t dot_id;
#endif

        Node(node_type_t type_arg)
          : type(type_arg),
            s(0),
            l(0)
#ifdef DEBUG_TemplateParser
          , dot_id(0)
#endif
        {

        }
    } node_t;


    TemplateParser(apr_pool_t *pool, void *node_memory,
                   apr_array_header_t *token_array,
                   apr_array_header_t *id_array,
                   apr_array_header_t *key_array=NULL);
    TemplateParser(apr_pool_t *pool, void *node_memory, TemplateLexer *lexer,
                   apr_array_header_t *key_array=NULL);
    void parse();
    const node_t *get_node_tree()
    {
        return top_node_;
    };
    apr_array_header_t *get_key_array() const
    {
        return key_array_;
    };
    apr_size_t get_node_count() const
    {
        return node_count_;
    };
    static apr_size_t calc_node_memory_size(apr_array_header_t *token_array)
    {
        // 最悪の場合のノード数はトークン数の 2 倍以下
        return sizeof(node_t) * token_array->nelts * 2;
    };
    static const node_t *parse(apr_pool_t *pool, const char *file_path,
                               const char ***ids, const char ***keys);
#ifdef DEBUG_TemplateParser
    void print_dot();
#endif

private:
    typedef TemplateLexer Lexer;
    typedef TemplateLexer::token_t token_t;

    typedef struct Input {
        const token_t **top;
        const token_t **start;
        const token_t **end;

        Input(apr_array_header_t *token_array)
          : top(reinterpret_cast<const token_t **>(AVOID_ALIGN_WARN
                                                   token_array->elts)),
            start(top),
            end(start + token_array->nelts)
        {

        }
    } input_t;


    typedef struct {
        node_type_t type;
        const char *str;
    } node_type_str;

    static const node_type_str node_type_list[];

    void init();
    void optimize(const node_t *node);
    bool can_optimize_bank(const node_t *node);

    const node_t *parse_program();
    node_t *parse_stmt_list();
    node_t *parse_stmt();
    node_t *parse_foreach();
    node_t *parse_while();
    node_t *parse_if();
    node_t *parse_else_();
    node_t *parse_print();
    node_t *parse_expr_list();
    node_t *parse_expr();
    node_t *parse_assign();
    node_t *parse_compare();
    node_t *parse_compare_();
    node_t *parse_arithmetic();
    node_t *parse_arithmetic_();
    node_t *parse_multiply();
    node_t *parse_multiply_();
    node_t *parse_term();
    node_t *parse_variable();
    node_t *parse_variable_();
    node_t *parse_variable__();

    apr_size_t get_key(const char *str);

    node_t *create_node(node_type_t type);
    node_t *create_node(TemplateLexer::token_type_t token_type);

    void dump_read_token();
#ifdef DEBUG_TemplateParser
    apr_size_t print_dot_node(const node_t *node);
#endif
    static const char *get_node_type(const node_t *node);

    apr_pool_t *pool_;

    input_t input_;

    const node_t *top_node_;

    apr_array_header_t *id_array_;
    apr_array_header_t *key_array_;

    node_t *node_pool_;
    apr_size_t node_count_;

    bool is_parsed_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
