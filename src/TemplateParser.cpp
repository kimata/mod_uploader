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
 * $Id: TemplateParser.cpp 2872 2008-04-27 14:18:02Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <cstring>
#include <memory>

#include "TemplateParser.h"
#include "File.h"
#include "Message.h"
#include "SourceInfo.h"

using namespace std;

SOURCE_INFO_ADD("$Id: TemplateParser.cpp 2872 2008-04-27 14:18:02Z svn $");

#define AS_NODE(pointer)            reinterpret_cast<node_t *>(pointer)
#define AS_UNCONST_NODE(pointer)    const_cast<node_t *>(pointer)

#define IS_INPUT_MATCH(pattern)                                     \
    ((input_.top != input_.end) &&                                  \
     ((*(input_.top))->type == Lexer::pattern))

#define EXPECT_INPUT_MATCH(pattern, message)                        \
    do {                                                            \
        if (!IS_INPUT_MATCH(pattern)) {                             \
            DUMP_TOKEN_AND_THROW(message);                          \
        }                                                           \
    } while (0)

#ifdef DEBUG_TemplateParser
static bool parser_is_trace = false;
#define DUMP_TOKEN_AND_THROW(message) dump_read_token(); THROW(message)
#ifdef _MSC_VER
#define TRACE_FUNC if (parser_is_trace) cerr << "CALL: " << __LINE__ << endl
#else
#define TRACE_FUNC if (parser_is_trace) cerr << "CALL: " << __func__ << endl
#endif
#else
#define DUMP_TOKEN_AND_THROW(message) THROW(message)
#define TRACE_FUNC
#endif

const TemplateParser::node_type_str TemplateParser::node_type_list[] = {
    { TemplateParser::FOREACH,              "foreach"       },
    { TemplateParser::WHILE,                "while"         },
    { TemplateParser::IF,                   "if"            },
    { TemplateParser::PRINT,                "print"         },
    { TemplateParser::ARRAY_REF,            "array ref"     },
    { TemplateParser::HASH_REF,             "hash ref"      },
    { TemplateParser::ASSIGN,               "="             },
    { TemplateParser::PLUS_ASSIGN,          "+="            },
    { TemplateParser::MINUS_ASSIGN,         "-="            },
    { TemplateParser::NOTEQUAL,             "!="            },
    { TemplateParser::EQUAL,                "=="            },
    { TemplateParser::GREATER_THAN,         "\\>"           },
    { TemplateParser::LESS_THAN,            "\\<"           },
    { TemplateParser::PLUS,                 "+"             },
    { TemplateParser::MINUS,                "-"             },
    { TemplateParser::RESIDUE,              "%"             },
    { TemplateParser::PLUS_PLUS,            "++"            },
    { TemplateParser::MINUS_MINUS,          "--"            },
    { TemplateParser::BANK,                 "bank"          },
    { TemplateParser::INTEGER,              "integer"       },
    { TemplateParser::STRING,               "string"        },
    { TemplateParser::IDENTIFIER,           "identifier"    },
    { TemplateParser::KEY,                  "key"           },
};


/******************************************************************************
 * public メソッド
 *****************************************************************************/
TemplateParser::TemplateParser(apr_pool_t *pool, void *node_memory,
                               apr_array_header_t *token_array,
                               apr_array_header_t *id_array,
                               apr_array_header_t *key_array)
  : pool_(pool),
    input_(token_array),
    top_node_(NULL),
    id_array_(id_array),
    key_array_(key_array),
    node_pool_(AS_NODE(node_memory)),
    node_count_(0),
    is_parsed_(false)
{
    init();
}

TemplateParser::TemplateParser(apr_pool_t *pool, void *node_memory,
                               TemplateLexer *lexer,
                               apr_array_header_t *key_array)
  : pool_(pool),
    input_(lexer->get_token_array()),
    top_node_(NULL),
    id_array_(lexer->get_id_array()),
    key_array_(key_array),
    node_pool_(AS_NODE(node_memory)),
    node_count_(0),
    is_parsed_(false)
{
    init();
}

void TemplateParser::parse()
{
    if (LIKELY(is_parsed_)) {
        return;
    }

    top_node_ = parse_program();
    optimize(top_node_);

    is_parsed_ = true;
}

const TemplateParser::node_t *TemplateParser::parse(apr_pool_t *pool,
                                                    const char *file_path,
                                                    const char ***ids,
                                                    const char ***keys)
{
    TemplateLexer *lexer;
    void *node_memory;

    lexer = TemplateLexer::get_instance(pool, file_path);
    auto_ptr<TemplateLexer> lexer_ap(lexer);

    APR_PALLOC(node_memory, void *, pool,
               TemplateParser::calc_node_memory_size(lexer->get_token_array()));

    TemplateParser parser(pool, node_memory, lexer);
    parser.parse();

    *ids = AS_CONST_CHAR_P(AVOID_ALIGN_WARN lexer->get_id_array()->elts);
    *keys = AS_CONST_CHAR_P(AVOID_ALIGN_WARN parser.get_key_array()->elts);

    return parser.get_node_tree();
}

#ifdef DEBUG_TemplateParser
void TemplateParser::print_dot()
{
    cout << "digraph node_tree {"               << endl;
    cout << "rankdir    = \"LR\""               << endl;
    cout << "ratio      = \"auto\";"            << endl;
    cout << "ranksep    = \"0.3\";"             << endl;
    cout << "node ["                            << endl;
    cout << "    fontname   = \"Helvetica\""    << endl;
    cout << "    fontsize   = \"7\""            << endl;
    cout << "    shape      = \"record\""       << endl;
    cout << "];"                                << endl;
    print_dot_node(top_node_);
    cout << "}"                                 << endl;
}
#endif


/******************************************************************************
 * private メソッド
 *****************************************************************************/
void TemplateParser::init()
{
    if (key_array_ == NULL) {
        key_array_ = apr_array_make(pool_, 32, sizeof(char *));
    }
}

void TemplateParser::optimize(const node_t *node)
{
    if (node == NULL) {
        return;
    }

    // 不要な BANK ノードの削除
    if (can_optimize_bank(node->child.left)) {
        AS_UNCONST_NODE(node)->child.left = node->child.left->child.left;
        node_count_--;
    }
    if (can_optimize_bank(node->child.center)) {
        AS_UNCONST_NODE(node)->child.center = node->child.center->child.left;
        node_count_--;
    }
    if (can_optimize_bank(node->child.right)) {
        AS_UNCONST_NODE(node)->child.right = node->child.right->child.left;
        node_count_--;
    }

    optimize(node->child.left);
    optimize(node->child.center);
    optimize(node->child.right);
}

bool TemplateParser::can_optimize_bank(const node_t *node)
{
    return (node != NULL) && (node->type == BANK) &&
        (node->child.center == NULL);
}

/**
 * <pre>
 * PROGRAM    ::= STMT_LIST
 * </pre>
 */
const TemplateParser::node_t *TemplateParser::parse_program()
{
    TRACE_FUNC;

    return parse_stmt_list();
}

/**
 * <pre>
 * STMT_LIST  ::= STMT*
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_stmt_list()
{
    node_t *node;
    node_t *child_node;
    node_t *curr_node;

    TRACE_FUNC;

    if (input_.top == input_.end) {
        return NULL;
    }

    if ((child_node = AS_NODE(parse_stmt())) == NULL) {
        return NULL;
    }

    node = create_node(BANK);
    node->child.left = child_node;

    curr_node = node;
    while (input_.top != input_.end) {
        if ((child_node = parse_stmt()) == NULL) {
            break;
        }

        if (curr_node->child.center == NULL) {
            curr_node->child.center = child_node;
        } else {
            curr_node->child.right = create_node(BANK);
            curr_node = AS_UNCONST_NODE(curr_node->child.right);
            curr_node->child.left = child_node;
        }
    }

    return node;
}

/**
 * <pre>
 * STMT       ::= FOREACH | WHILE | IF | PRINT | EXPR ';'
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_stmt()
{
    node_t *node;

    TRACE_FUNC;

    if (input_.top == input_.end) {
        return NULL;
    }

    switch ((*(input_.top))->type) {
    case Lexer::FOREACH:               // FOREACH
        return parse_foreach();
    case Lexer::WHILE:                 // WHILE
        return parse_while();
    case Lexer::IF:                    // IF
        return parse_if();
    case Lexer::PRINT:                 // PRINT
        return parse_print();
    default:                           // EXPR ';'
        node = parse_expr();

        if (node == NULL) {
            return NULL;
        }
        EXPECT_INPUT_MATCH(DELIMITER, MESSAGE_TMPL_STMT_PARSE_FAILED);
        input_.top++;

        return node;
    }
}

/**
 * <pre>
 * FOREACH    ::= 'foreach' '(' VARIABLE 'in' VARIABLE ')' '{' STMT_LIST  '}'
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_foreach()
{
    node_t *node;
    node_t *child_node;

    TRACE_FUNC;

#ifdef DEBUG
    if ((input_.top == input_.end) ||
        ((*(input_.top))->type != Lexer::FOREACH)) {
        DUMP_TOKEN_AND_THROW(MESSAGE_BUG_FOUND);
    }
#endif

    // foreach
    node = create_node((*(input_.top))->type);
    input_.top++;

    // (
    EXPECT_INPUT_MATCH(PAREN_LEFT, MESSAGE_TMPL_FOREACH_PARSE_FAILED);
    input_.top++;

    // 変数
    EXPECT_INPUT_MATCH(IDENTIFIER, MESSAGE_TMPL_FOREACH_PARSE_FAILED);
    child_node = create_node((*(input_.top))->type);
    child_node->id = (*(input_.top))->id;
    node->child.left = child_node;
    input_.top++;

    // in
    EXPECT_INPUT_MATCH(IN, MESSAGE_TMPL_FOREACH_PARSE_FAILED);
    input_.top++;

    // 変数
    if ((node->child.center = parse_variable()) == NULL) {
        DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_FOREACH_PARSE_FAILED);
    }

    // )
    EXPECT_INPUT_MATCH(PAREN_RIGHT, MESSAGE_TMPL_FOREACH_PARSE_FAILED);
    input_.top++;

    // {
    EXPECT_INPUT_MATCH(BRACE_LEFT, MESSAGE_TMPL_FOREACH_PARSE_FAILED);
    input_.top++;

    node->child.right = parse_stmt_list();

    // {
    EXPECT_INPUT_MATCH(BRACE_RIGHT, MESSAGE_TMPL_FOREACH_PARSE_FAILED);
    input_.top++;

    return node;
}

/**
 * <pre>
 * WHILE      ::= 'while' '(' VARIABLE 'in' VARIABLE ')' '{' STMT_LIST  '}'
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_while()
{
    node_t *node;

    TRACE_FUNC;

#ifdef DEBUG
    if ((input_.top == input_.end) ||
        ((*(input_.top))->type != Lexer::WHILE)) {
        DUMP_TOKEN_AND_THROW(MESSAGE_BUG_FOUND);
    }
#endif

    node = create_node((*(input_.top))->type);
    input_.top++;

    EXPECT_INPUT_MATCH(PAREN_LEFT, MESSAGE_TMPL_WHILE_PARSE_FAILED);
    input_.top++;

    if ((node->child.left = parse_compare()) == NULL) {
        DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_WHILE_PARSE_FAILED);
    }

    EXPECT_INPUT_MATCH(PAREN_RIGHT, MESSAGE_TMPL_WHILE_PARSE_FAILED);
    input_.top++;

    EXPECT_INPUT_MATCH(BRACE_LEFT, MESSAGE_TMPL_WHILE_PARSE_FAILED);
    input_.top++;

    node->child.right = parse_stmt_list();

    EXPECT_INPUT_MATCH(BRACE_RIGHT, MESSAGE_TMPL_WHILE_PARSE_FAILED);
    input_.top++;

    return node;
}

/**
 * <pre>
 * IF         ::= 'if' '(' COMPARE ')' '{' STMT_LIST '}' ELSE_
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_if()
{
    node_t *node;

    TRACE_FUNC;

#ifdef DEBUG
    if ((input_.top == input_.end) ||
        ((*(input_.top))->type != Lexer::IF)) {
        DUMP_TOKEN_AND_THROW(MESSAGE_BUG_FOUND);
    }
#endif

    node = create_node((*(input_.top))->type);
    input_.top++;

    EXPECT_INPUT_MATCH(PAREN_LEFT, MESSAGE_TMPL_IF_PARSE_FAILED);
    input_.top++;

    if ((node->child.center = parse_compare()) == NULL) {
        DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_IF_PARSE_FAILED);
    }

    EXPECT_INPUT_MATCH(PAREN_RIGHT, MESSAGE_TMPL_IF_PARSE_FAILED);
    input_.top++;

    EXPECT_INPUT_MATCH(BRACE_LEFT, MESSAGE_TMPL_IF_PARSE_FAILED);
    input_.top++;

    node->child.left = parse_stmt_list();

    EXPECT_INPUT_MATCH(BRACE_RIGHT, MESSAGE_TMPL_IF_PARSE_FAILED);
    input_.top++;

    node->child.right = parse_else_();

    return node;
}

/**
 * <pre>
 * ELSE_      ::= 'else' '{' STMT_LIST '}' | EPSILON
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_else_()
{
    node_t *node;

    TRACE_FUNC;

    if (input_.top == input_.end) {
        return NULL;
    }

    if ((*(input_.top))->type != Lexer::ELSE) { // EPSILON
        return NULL;
    }
    input_.top++;

    EXPECT_INPUT_MATCH(BRACE_LEFT, MESSAGE_TMPL_ELSE_PARSE_FAILED);
    input_.top++;

    node = parse_stmt_list();

    EXPECT_INPUT_MATCH(BRACE_RIGHT, MESSAGE_TMPL_ELSE_PARSE_FAILED);
    input_.top++;

    return node;
}

/**
 * <pre>
 * PRINT      ::= 'print' EXPR_LIST ';'
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_print()
{
    node_t *node;

    TRACE_FUNC;

#ifdef DEBUG
    if ((input_.top == input_.end) ||
        ((*(input_.top))->type != Lexer::PRINT)) {
        DUMP_TOKEN_AND_THROW(MESSAGE_BUG_FOUND);
    }
#endif

    node = create_node((*(input_.top))->type);
    input_.top++;

    node->child.left = parse_expr_list();

    EXPECT_INPUT_MATCH(DELIMITER, MESSAGE_TMPL_ELSE_PARSE_FAILED);
    input_.top++;

    return node;
}

/**
 * <pre>
 * EXPR_LIST  ::= EXPR (',' EXPR)*
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_expr_list()
{
    node_t *node;
    node_t *child_node;
    node_t *curr_node;

    TRACE_FUNC;

    if (input_.top == input_.end) {
        return NULL;
    }

    if ((child_node = parse_expr()) == NULL) {
        return NULL;
    }

    node = create_node(BANK);
    node->child.left = child_node;

    curr_node = node;
    while (IS_INPUT_MATCH(COMMA)) {
        input_.top++;

        if ((child_node = parse_expr()) == NULL) {
            DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_EXPR_PARSE_FAILED);
        }

        if (curr_node->child.center == NULL) {
            curr_node->child.center = child_node;
        } else {
            curr_node->child.right = create_node(BANK);
            curr_node = AS_UNCONST_NODE(curr_node->child.right);
            curr_node->child.left = child_node;
        }
    }

    return node;
}

/**
 * <pre>
 * EXPR       ::= ASSIGN | COMPARE
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_expr()
{
    TRACE_FUNC;

    if (input_.top == input_.end) {
        return NULL;
    }

    if (((*(input_.top))->type == Lexer::IDENTIFIER) &&
        ((input_.top + 1) != input_.end) &&
        (((*(input_.top + 1))->type == Lexer::ASSIGN) ||
         ((*(input_.top + 1))->type == Lexer::PLUS_ASSIGN) ||
         ((*(input_.top + 1))->type == Lexer::MINUS_ASSIGN))) { // ASSIGN
        return parse_assign();
    } else {// COMPARE
        return parse_compare();
    }
}

/**
 * <pre>
 * ASSIGN     ::= IDENTIFIER ASSIGN_OP COMPARE
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_assign()
{
    node_t *node;
    node_t *left_node;
    node_t *right_node;

    TRACE_FUNC;

    if (input_.top == input_.end) {
        return NULL;
    }

    EXPECT_INPUT_MATCH(IDENTIFIER, MESSAGE_TMPL_ASSIGN_PARSE_FAILED);

    left_node = create_node((*(input_.top))->type);
    left_node->id = (*(input_.top))->id;
    input_.top++;

    if (((*(input_.top))->type != Lexer::ASSIGN) &&
        ((*(input_.top))->type != Lexer::PLUS_ASSIGN) &&
        ((*(input_.top))->type != Lexer::MINUS_ASSIGN)) {
        DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_ASSIGN_PARSE_FAILED);
    }

    node = create_node((*(input_.top))->type);
    input_.top++;

    if ((right_node = parse_compare()) == NULL) {
        DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_ASSIGN_PARSE_FAILED);
    }

    node->child.left = left_node;
    node->child.right = right_node;

    return node;
}

/**
 * <pre>
 * COMPARE    ::= ARITHMETIC COMPARE_
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_compare()
{
    node_t *node;
    node_t *child_node;

    TRACE_FUNC;

    if (input_.top == input_.end) {
        return NULL;
    }

    child_node = parse_arithmetic();
    if (child_node == NULL) {
        return NULL;
    }

    if ((node = parse_compare_()) == NULL) {
        node = child_node;
    } else {
        node->child.left = child_node;
    }

    return node;
}

/**
 * <pre>
 * COMPARE_   ::= COMPARE_OP ARITHMETIC | EPSILON
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_compare_()
{
    node_t *node;

    TRACE_FUNC;

    if (input_.top == input_.end) {
        return NULL;
    }

    if (((*(input_.top))->type != Lexer::EQUAL) &&
        ((*(input_.top))->type != Lexer::NOTEQUAL) &&
        ((*(input_.top))->type != Lexer::GREATER_THAN) &&
        ((*(input_.top))->type != Lexer::LESS_THAN)) { // EPSILON
        return NULL;
    }

    node = create_node((*(input_.top))->type);
    input_.top++;

    if ((node->child.right = parse_arithmetic()) == NULL) {
        DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_COMPARE_PARSE_FAILED);
    }

    return node;
}

/**
 * <pre>
 * ARITHMETIC ::= MULTIPLY ARITHMETIC_
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_arithmetic()
{
    node_t *node;
    node_t *child_node;

    TRACE_FUNC;

    if ((child_node = parse_multiply()) == NULL) {
        return NULL;
    }

    node = parse_arithmetic_();
    if (node == NULL) {
        node = child_node;
    } else {
        node->child.left = child_node;
    }

    return node;
}

/**
 * <pre>
 * ARITHMETIC_::= '+' MULTIPLY ARITHMETIC_ | '-' MULTIPLY ARITHMETIC_ | EPSILON
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_arithmetic_()
{
    node_t *node;
    node_t *child_node;

    TRACE_FUNC;

    if (input_.top == input_.end) {
        return NULL;
    }

    switch ((*(input_.top))->type) {
    case Lexer::PLUS:                  // '+' MULTIPLY ARITHMETIC_
    case Lexer::MINUS:                 // '-' MULTIPLY ARITHMETIC_
        child_node = create_node((*(input_.top))->type);
        input_.top++;

        if ((child_node->child.right = parse_multiply()) == NULL) {
            DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_ARITHMETIC_PARSE_FAILED);
        }

        if ((node = parse_arithmetic_()) == NULL) {
            node = child_node;
        } else {
            node->child.left = child_node;
        }
        break;
    default:                           // EPSILON
        node = NULL;
        break;
    }

    return node;
}

/**
 * <pre>
 * MULTIPLY   ::= TERM MULTIPLY_
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_multiply()
{
    node_t *node;
    node_t *child_node;

    TRACE_FUNC;

    if ((child_node = parse_term()) == NULL) {
        return NULL;
    }

    if ((node = parse_multiply_()) == NULL) {
        node = child_node;
    } else {
        node->child.left = child_node;
    }

    return node;
}

/**
 * <pre>
 * MULTIPLY_  ::= '%' TERM MULTIPLY_ | EPSILON
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_multiply_()
{
    node_t *node;
    node_t *child_node;

    TRACE_FUNC;

    if (input_.top == input_.end) {
        return NULL;
    }

    switch ((*(input_.top))->type) {
    case Lexer::RESIDUE:               // '%' TERM MULTIPLY_
        child_node = create_node((*(input_.top))->type);
        input_.top++;

        if ((child_node->child.right = parse_term()) == NULL) {
            DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_MULTIPLY_PARSE_FAILED);
        }

        if ((node = parse_multiply_()) == NULL) {
            node = child_node;
        } else {
            node->child.left = child_node;
        }
        break;
    default:                           // EPSILON
        node = NULL;
        break;
    }

    return node;
}

/**
 * <pre>
 * TERM       ::= MONADIC_OP VARIABLE | VARIABLE MONADIC_OP |
 *                VARIABLE | INTEGER | STRING | '(' EXPR ')'
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_term()
{
    node_t *node;
    node_t *child_node;

    TRACE_FUNC;

    if (input_.top == input_.end) {
        return NULL;
    }

    switch ((*(input_.top))->type) {
    case Lexer::PLUS_PLUS:
    case Lexer::MINUS_MINUS:           // MONADIC_OP VARIABLE
        node = create_node((*(input_.top))->type);
        input_.top++;

        if ((node->child.right = parse_variable()) == NULL) {
            DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_TERM_PARSE_FAILED);
        }

        break;
    case Lexer::IDENTIFIER:            // VARIABLE | VARIABLE MONADIC_OP
        node = parse_variable();
        if ((input_.top != input_.end) &&
            (((*(input_.top))->type == Lexer::PLUS_PLUS) ||
             ((*(input_.top))->type == Lexer::MINUS_MINUS))) {
            child_node = node;
            node = create_node((*(input_.top))->type);
            node->child.left = child_node;
            input_.top++;
        }
        break;
    case Lexer::INTEGER:               // INTEGER
        node = create_node((*(input_.top))->type);
        node->i = (*(input_.top))->i;
        input_.top++;
        break;
    case Lexer::STRING:                // STRING
        node = create_node((*(input_.top))->type);
        node->s = (*(input_.top))->s;
        node->l = strlen((*(input_.top))->s);
        input_.top++;
        break;
    case Lexer::PAREN_LEFT:            // '(' EXPR ')'
        input_.top++;

        if (((node = parse_expr()) == NULL) ||
            (input_.top == input_.end) ||
            ((*(input_.top))->type != Lexer::PAREN_RIGHT)) {
            DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_TERM_PARSE_FAILED);
        }
        input_.top++;
        break;
    default:
        node = NULL;
        break;
    }

    return node;
}

/**
 * <pre>
 * VARIABLE   ::= IDENTIFIER VARIABLE_
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_variable()
{
    node_t *node;
    node_t *child_node;

    TRACE_FUNC;

    if (input_.top == input_.end) {
        return NULL;
    }

    if ((*(input_.top))->type != Lexer::IDENTIFIER) {
        return NULL;
    }

    child_node = create_node((*(input_.top))->type);
    child_node->id = (*(input_.top))->id;
    input_.top++;

    node = parse_variable_();

    if (node  == NULL) {
        node = child_node;
    } else {
        node->child.left = child_node;
    }

    return node;
}

/**
 * <pre>
 * VARIABLE_  ::= '.' IDENTIFIER | '[' (INTEGER | IDENTIFIER) ']' VARIABLE__ | EPSILON
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_variable_()
{
    node_t *node;
    node_t *child_node;

    TRACE_FUNC;

    if (input_.top == input_.end) {
        return NULL;
    }

    if ((*(input_.top))->type == Lexer::DOT) { // '.' IDENTIFIER
        input_.top++;
        node = create_node(HASH_REF);

        EXPECT_INPUT_MATCH(IDENTIFIER, MESSAGE_TMPL_VARIABLE_PARSE_FAILED);

        child_node = create_node(KEY);
        child_node->id
            = get_key(AS_CONST_CHAR_P(AVOID_ALIGN_WARN id_array_->elts)
                      [(*(input_.top))->id]);
        node->child.right = child_node;
        input_.top++;
    } else if ((*(input_.top))->type == Lexer::BRACKET_LEFT) { // '[' (INTEGER | IDENTIFIER) ']' VARIABLE__
        input_.top++;
        node = create_node(ARRAY_REF);

        if (IS_INPUT_MATCH(INTEGER)) {
            child_node = create_node((*(input_.top))->type);
            child_node->i = (*(input_.top))->i;
            input_.top++;
        } else if (IS_INPUT_MATCH(IDENTIFIER)) {
            child_node = create_node((*(input_.top))->type);
            child_node->id = (*(input_.top))->id;
            input_.top++;
        } else {
            DUMP_TOKEN_AND_THROW(MESSAGE_TMPL_VARIABLE_PARSE_FAILED);
        }

        EXPECT_INPUT_MATCH(BRACKET_RIGHT, MESSAGE_TMPL_STMT_PARSE_FAILED);
        input_.top++;
        if ((node->child.right = parse_variable__()) == NULL) {
            node->child.right = child_node;
        } else {
            AS_UNCONST_NODE(node->child.right)->child.left = child_node;
        }
    } else {
        node = NULL;
    }

    return node;
}

/**
 * <pre>
 * VARIABLE__ ::= '.' IDENTIFIER | EPSILON
 * </pre>
 */
TemplateParser::node_t *TemplateParser::parse_variable__()
{
    node_t *node;
    node_t *child_node;

    TRACE_FUNC;

    if (input_.top == input_.end) {
        return NULL;
    }

    if ((*(input_.top))->type != Lexer::DOT) { // EPSILON
        return NULL;
    }

    input_.top++;
    node = create_node(HASH_REF);

    EXPECT_INPUT_MATCH(IDENTIFIER, MESSAGE_TMPL_VARIABLE_PARSE_FAILED);

    child_node = create_node(KEY);
    child_node->id
        = get_key(AS_CONST_CHAR_P(AVOID_ALIGN_WARN id_array_->elts)
                  [(*(input_.top))->id]);
    node->child.right = child_node;

    input_.top++;

    return node;
}

apr_size_t TemplateParser::get_key(const char *str)
{
    const char **keys;
    int i;

    keys = AS_CONST_CHAR_P(AVOID_ALIGN_WARN key_array_->elts);
    for (i = 0; i < key_array_->nelts; i++) {
        if (strcmp(keys[i], str) == 0) {
            return i;
        }
    }

    *AS_CONST_CHAR_P(apr_array_push(key_array_)) = str;

    return static_cast<apr_size_t>(i);
}

TemplateParser::node_t *TemplateParser::create_node(node_type_t type)
{
    node_t *node;

    node = node_pool_++;
    new(node) node_t(type);

    node_count_++;

    return node;
}

TemplateParser::node_t *TemplateParser::create_node(Lexer::token_type_t token_type)
{
    node_type_t type;

    // びみょー．こんなことしなくていい気もするけど，両者の間で意味合い
    // がちょっと違うし，どうしたものか．
    switch (token_type) {
    case Lexer::FOREACH         : type = FOREACH;       break;
    case Lexer::WHILE           : type = WHILE;         break;
    case Lexer::IF              : type = IF;            break;
    case Lexer::PRINT           : type = PRINT;         break;
    case Lexer::STRING          : type = STRING;        break;
    case Lexer::IDENTIFIER      : type = IDENTIFIER;    break;
    case Lexer::INTEGER         : type = INTEGER;       break;
    case Lexer::ASSIGN          : type = ASSIGN;        break;
    case Lexer::PLUS_ASSIGN     : type = PLUS_ASSIGN;   break;
    case Lexer::MINUS_ASSIGN    : type = MINUS_ASSIGN;  break;
    case Lexer::EQUAL           : type = EQUAL;         break;
    case Lexer::NOTEQUAL        : type = NOTEQUAL;      break;
    case Lexer::GREATER_THAN    : type = GREATER_THAN;  break;
    case Lexer::LESS_THAN       : type = LESS_THAN;     break;
    case Lexer::PLUS            : type = PLUS;          break;
    case Lexer::MINUS           : type = MINUS;         break;
    case Lexer::RESIDUE         : type = RESIDUE;       break;
    case Lexer::PLUS_PLUS       : type = PLUS_PLUS;     break;
    case Lexer::MINUS_MINUS     : type = MINUS_MINUS;   break;
    default                     : type = DEFAULT;       break;
    }

    return create_node(type);
}

void TemplateParser::dump_read_token()
{
    const token_t **node;

    node = input_.start;
    while (1) {
        TemplateLexer::dump_token(*node, id_array_);

        if (node++ == input_.top) {
            break;
        }
    }
}

#ifdef DEBUG_TemplateParser
apr_size_t TemplateParser::print_dot_node(const node_t *node)
{
    static apr_size_t dot_id = 1;
    apr_size_t child_dot_id;

    if (node->dot_id == 0) {
        AS_UNCONST_NODE(node)->dot_id = dot_id++;
    }

    cout << "\"node_" << node->dot_id <<  "\" [" << endl;
    cout << "    label = \"" << get_node_type(node);
    if (node->type == IDENTIFIER) {
        cout << "\\n[" << AS_CONST_CHAR_P(id_array_->elts)[node->id] << "] ";
    } else if (node->type == KEY) {
        cout << "\\n[" << AS_CONST_CHAR_P(key_array_->elts)[node->id] << "] ";
    }
    cout << " | <0>left | <1>center | <2>right\"" << endl;
    cout << "    width= \"0.4\"" << endl;
    cout << "];" << endl;

    if (node->child.left != NULL) {
        child_dot_id = print_dot_node(node->child.left);
        cout << "\"node_" << node->dot_id << "\":0 -> \"node_";
        cout << child_dot_id << "\"" << endl;
    }
    if (node->child.center != NULL) {
        child_dot_id = print_dot_node(node->child.center);
        cout << "\"node_" << node->dot_id << "\":1 -> \"node_";
        cout << child_dot_id << "\"" << endl;

    }
    if (node->child.right != NULL) {
        child_dot_id = print_dot_node(node->child.right);
        cout << "\"node_" << node->dot_id << "\":2 -> \"node_";
        cout << child_dot_id << "\"" << endl;
    }

    return node->dot_id;
}
#endif

const char *TemplateParser::get_node_type(const node_t *node)
{
    for (apr_size_t i = 0; i < ARRAY_SIZE_OF(node_type_list); i++) {
        if (node->type == node_type_list[i].type) {
            return node_type_list[i].str;
        }
    }
    return "(UNKNOWN)";
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_TemplateParser
#include "TestRunner.h"

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name << " <TMPL_FILE_PATH>" << endl;
}

void run_parse(apr_pool_t *pool, const char *tmpl_file_path,
               apr_size_t dump_level)
{
    TemplateLexer *lexer;
    void *node_memory;

    show_test_name("parse");

    lexer = TemplateLexer::get_instance(pool, tmpl_file_path);
    auto_ptr<TemplateLexer> lexer_ap(lexer);

    APR_PALLOC(node_memory, void *, pool,
               TemplateParser::calc_node_memory_size(lexer->get_token_array()));

    if (dump_level > 2) {
        parser_is_trace = true;
    }

    TemplateParser parser(pool, node_memory, lexer);
    parser.parse();

    if (parser_is_trace) {
        show_line();
    }

    show_item("ident count", lexer->get_id_array()->nelts);
    show_item("key count", parser.get_key_array()->nelts);
    show_item("token count", lexer->get_token_array()->nelts);
    show_item("node count", parser.get_node_count());

    if (dump_level > 3) {
        parser.print_dot();
    }
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
    show_item("dump level", dump_level);
    show_line();

    run_parse(pool, tmpl_file_path, dump_level);
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
