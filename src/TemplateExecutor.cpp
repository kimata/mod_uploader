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
 * $Id: TemplateExecutor.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef TEMPLATE_INSTANTIATION
#include "Environment.h"
#endif

#include <cstring>

#include "apr_strings.h"

#include "TemplateExecutor.h"
#include "Macro.h"
#include "Message.h"
#include "SourceInfo.h"

using namespace std;

#ifndef TEMPLATE_INSTANTIATION
SOURCE_INFO_ADD("$Id: TemplateExecutor.cpp 2756 2007-12-11 10:57:59Z svn $");
#endif

#ifdef DEBUG_TemplateExecutor
static bool executor_is_trace = false;
#define EXPECT_NODE_MATCH(pattern, message)                         \
    do {                                                            \
        if (node->type != Parser::pattern) {                        \
            THROW(message);                                         \
        }                                                           \
    } while (0)
#ifdef _MSC_VER
#define TRACE_FUNC if (executor_is_trace) cerr << "CALL: " << __LINE__ << endl
#else
#define TRACE_FUNC if (executor_is_trace) cerr << "CALL: " << __func__ << endl
#endif
#else
#define EXPECT_NODE_MATCH(pattern, message)
#define TRACE_FUNC
#endif

#define TEMPLATE_EXECUTOR_INLINE inline

// MEMO:
// デバッグ時以外は，実行時チェックを大幅にカットしてあります．

/******************************************************************************
 * public メソッド
 ****************************************************************************/
template<class W>
TemplateExecutor<W>::TemplateExecutor(apr_pool_t *pool,
                                      W& writer)
  : pool_(pool),
    writer_(writer)
{

}

template<class W>
void TemplateExecutor<W>::exec(const node_t *node,
                               variable_t **variables,
                               apr_size_t key_count)
{
    TRACE_FUNC;
    EXPECT_NODE_MATCH(BANK, MESSAGE_TMPL_STMT_LIST_EXEC_FAILED);

    variables_ = variables;

    prepare_exec(key_count);
    exec_stmt_list(node);
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
template<class W>
void TemplateExecutor<W>::prepare_exec(apr_size_t key_count)
{
    scalar_t *scr_pool;

    APR_PALLOC(var_pool_, variable_t *, pool_,
               (sizeof(variable_t) + sizeof(scalar_t)) * key_count);

    scr_pool = reinterpret_cast<scalar_t *>(var_pool_ + key_count);

    for (apr_size_t i = 0; i< key_count; i++) {
        var_pool_[i].type = Variable::SCALAR;
        var_pool_[i].s = scr_pool++;
    }
}

template<class W>
void TemplateExecutor<W>::exec_stmt_list(const node_t *node)
{
    TRACE_FUNC;
    EXPECT_NODE_MATCH(BANK, MESSAGE_TMPL_STMT_LIST_EXEC_FAILED);

    exec_stmt(node->child.left);

    if (node->child.center == NULL) {
        return;
    }
    exec_stmt(node->child.center);

    if (node->child.right == NULL) {
        return;
    }
    exec_stmt(node->child.right);
}

template<class W>
void TemplateExecutor<W>::exec_stmt(const node_t *node)
{
    TRACE_FUNC;

    switch (node->type) {
    case Parser::BANK:          // BANK
        return exec_stmt_list(node);
    case Parser::PRINT:         // PRINT
        return exec_print(node);
    case Parser::IF:            // IF
        return exec_if(node);
    case Parser::FOREACH:       // FOREACH
        return exec_foreach(node);
    case Parser::WHILE:         // WHILE
        return exec_while(node);
    default:
        calc_i_val(node);
    }
}

template<class W>
void TemplateExecutor<W>::exec_foreach(const node_t *node)
{
    variable_t *array_var;
    variable_t *element_var;
    apr_size_t element_id;

    TRACE_FUNC;
    EXPECT_NODE_MATCH(FOREACH, MESSAGE_TMPL_FOREACH_EXEC_FAILED);

    array_var = variables_[node->child.center->id];

#ifndef FASTEST
    if (array_var == NULL) {
        THROW(MESSAGE_TMPL_UNINITIALIZED_VAR_REFERRED);
    } else if (UNLIKELY(array_var->type != Variable::ARRAY)) {
        THROW(MESSAGE_TMPL_ARRAY_TYPE_MISMATCH);
    }
#endif

    element_var = array_var->v;
    element_id = node->child.left->id;
    while (element_var->type != Variable::END) {
        variables_[element_id] = element_var;
        exec_stmt(node->child.right);
        element_var++;
    }
}

template<class W>
void TemplateExecutor<W>::exec_while(const node_t *node)
{
    TRACE_FUNC;
    EXPECT_NODE_MATCH(WHILE, MESSAGE_TMPL_WHILE_EXEC_FAILED);

    while (calc_b_val(node->child.left)) {
        exec_stmt(node->child.right);
    }
}

template<class W>
void TemplateExecutor<W>::exec_if(const node_t *node)
{
    TRACE_FUNC;
    EXPECT_NODE_MATCH(IF, MESSAGE_TMPL_IF_EXEC_FAILED);

    if (calc_b_val(node->child.center)) {
        exec_stmt(node->child.left);
    } else if (node->child.right != NULL) {
        exec_stmt(node->child.right);
    }
}

template<class W>
void TemplateExecutor<W>::exec_print(const node_t *node)
{
    TRACE_FUNC;
    EXPECT_NODE_MATCH(PRINT, MESSAGE_TMPL_PRINT_EXEC_FAILED);

    exec_print_element(node->child.left);
}

template<class W>
void TemplateExecutor<W>::exec_print_element(const node_t *node)
{
    const variable_t *var;

    TRACE_FUNC;

    switch (node->type) {
    case Parser::BANK:
        return exec_print_bank(node);
    case TemplateParser::STRING:        // 文字列
        return print(node->s, node->l);
    case TemplateParser::HASH_REF:      // .
    case TemplateParser::ARRAY_REF:     // []
        var = get_variable(node);
        goto VAR;
    case TemplateParser::IDENTIFIER:    // 識別子
        var = variables_[node->id];
        goto VAR;
    VAR:
#ifndef FASTEST
        if (var == NULL) {
            THROW(MESSAGE_TMPL_UNINITIALIZED_VAR_REFERRED);
        }
#endif
        if ((var->type == Variable::SCALAR) &&
            (var->s->type == Variable::STRING)) {
            if (var->s->l == 0) {
                return print(var->s->s);
            } else {
                return print(var->s->s , var->s->l);
            }
        } else {
            return print(calc_i_val(var));
        }
    default:
        return print(calc_i_val(node));
    }
}

template<class W>
void TemplateExecutor<W>::exec_print_bank(const node_t *node)
{
    TRACE_FUNC;
    EXPECT_NODE_MATCH(BANK, MESSAGE_TMPL_PRINT_EXEC_FAILED);

    exec_print_element(node->child.left);

    if (node->child.center == NULL) {
        return;
    }
    exec_print_element(node->child.center);

    if (node->child.right == NULL) {
        return;
    }
    exec_print_element(node->child.right);
}


template<class W>
bool TemplateExecutor<W>::calc_b_val(const node_t *node)
{
    TRACE_FUNC;

    switch (node->type) {
    case Parser::EQUAL:
        return calc_i_val(node->child.left) == calc_i_val(node->child.right);
    case Parser::NOTEQUAL:
        return calc_i_val(node->child.left) != calc_i_val(node->child.right);
    case Parser::GREATER_THAN:
        return calc_i_val(node->child.left) > calc_i_val(node->child.right);
    case Parser::LESS_THAN:
        return calc_i_val(node->child.left) < calc_i_val(node->child.right);
    default:
#ifndef FASTEST
        THROW(MESSAGE_BUG_FOUND);
#else
        return false;
#endif
    }
}

template<class W>
int TemplateExecutor<W>::calc_i_val(const node_t *node)
{
    TRACE_FUNC;

    variable_t *var;
    int diff;
    bool is_return_before;
    int value;

    switch (node->type) {
    case Parser::IDENTIFIER:            // IDENTIFIER
        var = variables_[node->id];
#ifndef FASTEST
        if (var == NULL) {
            THROW(MESSAGE_TMPL_UNINITIALIZED_VAR_REFERRED);
        }
#endif
        return calc_i_val(var);
    case Parser::INTEGER:               // INTEGR
        return node->i;
    case Parser::HASH_REF:              // .
        return calc_i_val(get_variable(node));
    case Parser::ASSIGN:                // =
        return calc_assign(node);
    case Parser::PLUS_ASSIGN:           // +=
        diff = calc_i_val(node->child.right);
        goto ASSIGN2;
    case Parser::MINUS_ASSIGN:          // -=
        diff = -calc_i_val(node->child.right);
        goto ASSIGN2;
    ASSIGN2:
        return calc_assign_diff(node, diff);
    case Parser::PLUS_PLUS:             // ++
        diff = 1;
        goto MONADIC;
    case Parser::MINUS_MINUS:           // --
        diff = -1;
        goto MONADIC;
    MONADIC:
        if (node->child.left != NULL) {
            var = variables_[node->child.left->id];
            is_return_before = true;
        } else if (node->child.right != NULL) {
            var = variables_[node->child.right->id];
            is_return_before = false;
        } else {
#ifndef FASTEST
            THROW(MESSAGE_BUG_FOUND);
#else
            // ここには来ないはずだけど，コンパイラの警告をなくすため一応
            return 0;
#endif
        }
#ifndef FASTEST
        if (var == NULL) {
            THROW(MESSAGE_TMPL_UNINITIALIZED_VAR_REFERRED);
        }

        // ハッシュや配列の要素に対して直接適用できないようにする
        if (var->type != Variable::SCALAR) {
            THROW(MESSAGE_TMPL_SCALAR_TYPE_MISMATCH);
        }
        if (var->s->type != Variable::INTEGER) {
            THROW(MESSAGE_TMPL_INTEGER_TYPE_MISMATCH);
        }
#endif
        value = var->s->i;
        var->s->i = value + diff;;

        return is_return_before ? value : var->s->i;
    case Parser::RESIDUE:               // %
        return calc_i_val(node->child.left) % calc_i_val(node->child.right);
    case Parser::PLUS:                  // +
        return calc_i_val(node->child.left) + calc_i_val(node->child.right);
    case Parser::MINUS:                 // -
        return calc_i_val(node->child.left) - calc_i_val(node->child.right);
    default:
#ifndef FASTEST
        THROW(MESSAGE_BUG_FOUND);
#else
        return 0;
#endif
    }
}

template<class W>
int TemplateExecutor<W>::calc_i_val(const variable_t *var)
{
    TRACE_FUNC;

#ifndef FASTEST
    if (var->type != Variable::SCALAR) {
        THROW(MESSAGE_TMPL_SCALAR_TYPE_MISMATCH);
    }
#endif

    if (var->s->type == Variable::INTEGER) {
        return var->s->i;
    } else {
        return static_cast<int>(strlen(var->s->s));
    }
}

template<class W>
int TemplateExecutor<W>::calc_assign(const node_t *node)
{
    TRACE_FUNC;
    EXPECT_NODE_MATCH(ASSIGN, MESSAGE_TMPL_ASSIGN_CALC_FAILED);

#ifndef FASTEST
    if (UNLIKELY((node->child.left != NULL) &&
                 (node->child.left->type != Parser::IDENTIFIER))) {
        THROW(MESSAGE_TMPL_ASSIGN_TYPE_MISMATCH);
    }
#endif

    switch (node->child.right->type) {
    case Parser::IDENTIFIER:            // 識別子
    case Parser::ARRAY_REF:             // []
    case Parser::HASH_REF:              // .
        return calc_assign_var(node);
    case Parser::STRING:                // 文字列
        return calc_assign_str(node);
    default:                            // 数値
        return calc_assign_int(node);
    }
}

template<class W>
int TemplateExecutor<W>::calc_assign_var(const node_t *node)
{
    apr_size_t left_id;
    variable_t *left_var;
    const variable_t *right_var;

    TRACE_FUNC;

    left_id = node->child.left->id;
    left_var = variables_[left_id];
    right_var = get_variable(node->child.right);

#ifndef FASTEST
    if ((left_var != NULL) &&
        (left_var->type != right_var->type)) {
        THROW(MESSAGE_TMPL_ASSIGN_TYPE_MISMATCH);
    }
#endif
    variables_[left_id] = const_cast<variable_t *>(right_var);

    if (right_var->type == Variable::SCALAR) {
        return calc_i_val(right_var);
    } else {
        // スカラ以外の場合は 1 を返す
        return 1;
    }
}

template<class W>
int TemplateExecutor<W>::calc_assign_str(const node_t *node)
{
    apr_size_t left_id;
    variable_t *left_var;

    TRACE_FUNC;

    left_id = node->child.left->id;
    left_var = variables_[left_id];

    // 文字列のコピーは行わない
    if (left_var == NULL) {
        left_var = create_str_var(node->child.right->s);
        variables_[left_id] = left_var;
    } else {
#ifndef FASTEST
        if (left_var->type != Variable::SCALAR) {
            THROW(MESSAGE_TMPL_SCALAR_TYPE_MISMATCH);
        }
#endif
        left_var->s->type = Variable::STRING;
        left_var->s->s = node->child.right->s;
    }

    return calc_i_val(left_var);
}

template<class W>
int TemplateExecutor<W>::calc_assign_int(const node_t *node)
{
    apr_size_t left_id;
    variable_t *left_var;

    TRACE_FUNC;

    left_id = node->child.left->id;
    left_var = variables_[left_id];

    if (left_var == NULL) {
        left_var = create_int_var(calc_i_val(node->child.right));
        variables_[left_id] = left_var;

        return calc_i_val(left_var);
    } else {
#ifndef FASTEST
        if (left_var->type != Variable::SCALAR) {
            THROW(MESSAGE_TMPL_SCALAR_TYPE_MISMATCH);
        }
#endif
        left_var->s->type = Variable::INTEGER;
        left_var->s->i = calc_i_val(node->child.right);

        return left_var->s->i;
    }
}

template<class W>
int TemplateExecutor<W>::calc_assign_diff(const node_t *node,
                                          int diff)
{
    variable_t *left_var;

    TRACE_FUNC;

    left_var = variables_[node->child.left->id];

#ifndef FASTEST
    if (left_var == NULL) {
        THROW(MESSAGE_TMPL_UNINITIALIZED_VAR_REFERRED);
    }
    if (left_var->type != Variable::SCALAR) {
        THROW(MESSAGE_TMPL_SCALAR_TYPE_MISMATCH);
    }
    if (left_var->s->type != Variable::INTEGER) {
        THROW(MESSAGE_TMPL_INTEGER_TYPE_MISMATCH);
    }
#endif
    left_var->s->i = calc_i_val(left_var) + diff;

    return left_var->s->i;
}

template<class W>
const typename TemplateExecutor<W>::variable_t *
TemplateExecutor<W>::get_variable(const node_t *node)
{
    variable_t *var;
    variable_t *cont_var;

    TRACE_FUNC;

    var = NULL;
    switch (node->type) {
    case Parser::IDENTIFIER:            // IDENTIFIER
        var = variables_[node->id];
        break;
    case Parser::HASH_REF:              // .
        cont_var = variables_[node->child.left->id];
#ifndef FASTEST
        if (cont_var == NULL) {
            THROW(MESSAGE_TMPL_UNINITIALIZED_VAR_REFERRED);
        } else if (cont_var->type != Variable::HASH) {
            THROW(MESSAGE_TMPL_HASH_TYPE_MISMATCH);
        }
#endif
        var = get_variable(node->child.right->id);
        var->s = cont_var->s + node->child.right->id;
        break;
    case Parser::ARRAY_REF:             // []
        cont_var = variables_[node->child.left->id];
#ifndef FASTEST
        if (cont_var == NULL) {
            THROW(MESSAGE_TMPL_UNINITIALIZED_VAR_REFERRED);
        } else if (cont_var->type != Variable::ARRAY) {
            THROW(MESSAGE_TMPL_ARRAY_TYPE_MISMATCH);
        }
#endif
        // 多分，構文木が直感と違うので注意
        if ((node->child.right->type == Parser::INTEGER) ||
            (node->child.right->type == Parser::IDENTIFIER)) { // foo[int]
            var = cont_var->v + calc_i_val(node->child.right);
        } else if (node->child.right->type == Parser::HASH_REF) { // foo[var].baz
            cont_var = cont_var->v + calc_i_val(node->child.right->child.left);
#ifndef FASTEST
            if (cont_var->type != Variable::HASH) {
                THROW(MESSAGE_TMPL_HASH_TYPE_MISMATCH);
            }
#endif
            var = get_variable(node->child.right->child.right->id);
            var->s = cont_var->s + node->child.right->child.right->id;
        }
        break;
    default:
        break;
    }

#ifndef FASTEST
    if (var == NULL) {
        THROW(MESSAGE_TMPL_UNINITIALIZED_VAR_REFERRED);
    }
#endif

    return var;
}

template<class W>
TEMPLATE_EXECUTOR_INLINE typename TemplateExecutor<W>::variable_t *
TemplateExecutor<W>::get_variable(apr_size_t key)
{
    return var_pool_ + key;
}

template<class W>
TEMPLATE_EXECUTOR_INLINE typename TemplateExecutor<W>::variable_t *
TemplateExecutor<W>::create_int_var(int i)
{
    variable_t *var;

    var = create_variable();
    var->type = Variable::SCALAR;
    var->s = create_scalar();
    var->s->type = Variable::INTEGER;
    var->s->i = i;

    return var;
}

template<class W>
TEMPLATE_EXECUTOR_INLINE typename TemplateExecutor<W>::variable_t *
TemplateExecutor<W>::create_str_var(const char *s)
{
    variable_t *var;

    var = create_variable();
    var->type = Variable::SCALAR;
    var->s = create_scalar();
    var->s->type = Variable::STRING;
    var->s->s = s;

    return var;
}

template<class W>
TEMPLATE_EXECUTOR_INLINE typename TemplateExecutor<W>::variable_t *
TemplateExecutor<W>::create_variable()
{
    variable_t *var;

    APR_PALLOC(var, variable_t *, pool_, sizeof(variable_t));
    var->type = Variable::SCALAR;

    return var;
}

template<class W>
TEMPLATE_EXECUTOR_INLINE typename TemplateExecutor<W>::scalar_t *
TemplateExecutor<W>::create_scalar()
{
    scalar_t *scr;

    APR_PALLOC(scr, scalar_t *, pool_, sizeof(scalar_t));

    return scr;
}

template<class W>
TEMPLATE_EXECUTOR_INLINE void TemplateExecutor<W>::print(const char *str,
                                                         apr_size_t length)
{
    writer_.write(str, length);
}

template<class W>
TEMPLATE_EXECUTOR_INLINE void TemplateExecutor<W>::print(const char *str)
{
    writer_.write(str);
}

template<class W>
TEMPLATE_EXECUTOR_INLINE void TemplateExecutor<W>::print(int i)
{
    writer_.write(i);
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_TemplateExecutor
#include "UploaderTemplate.h"
#include "UploadItemList.h"
#include "UploadItemListReader.h"
#include "UploadItemIterator.h"
#include "UploadItemVariableCreator.h"
#include "ThumbnailList.h"
#include "TemplateVariableCreator.h"
#include "CGIResponseWriter.h"
#include "File.h"

#include "TestRunner.h"

typedef TemplateExecutor<CGIResponseWriter> TemplateExecutorImpl;

static const apr_size_t ITEM_LIST_SIZE  = 10;
static const apr_uint64_t MAX_FILE_SIZE = 1000000;
static const apr_size_t MAX_LIST_SIZE   = 1000;

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name << " <DATA_DIR_PATH> <TMPL_FILE_PATH>" << endl;
}

void run_exec(apr_pool_t *pool, UploadItemList *item_list,
              const char *tmpl_file_path, apr_size_t dump_level)
{
    show_test_name("exec");

    UploaderTemplate up_template(pool, tmpl_file_path);
    up_template.load();

    TemplateVariableCreator var_creator(pool, up_template.get_ids());
    UploadItemVariableCreator item_var_creator(up_template.get_keys());

    UploadItemIterator item_iter(pool, item_list, 0, ITEM_LIST_SIZE);

    var_creator.create("ITEM_LIST", item_var_creator.create(pool, &item_iter));

    if (dump_level > 2) {
        executor_is_trace = true;
    }

    CGIResponseWriter writer(pool);
    TemplateExecutorImpl executor(pool, writer);

    executor.exec(up_template.get_node_tree(), var_creator.get_variables(),
                  up_template.get_key_count());
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    const char *data_dir_path;
    const char *tmpl_file_path;
    apr_size_t dump_level;
    apr_shm_t *item_list_shm;
    apr_shm_t *thumbnail_list_shm;
    UploadItemList *item_list;
    apr_time_t mtime;
    ThumbnailList *thumbnail_list;

    if (argc < 3) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    data_dir_path = argv[1];
    tmpl_file_path = argv[2];
    if (argc >= 5) {
        dump_level = atoi(argv[3]);
    } else {
        dump_level = 0;
    }

    if (!File::is_exist(pool, data_dir_path)) {
        THROW(MESSAGE_DATA_DIR_NOT_FOUND);
    }
    if (!File::is_exist(pool, tmpl_file_path)) {
        THROW(MESSAGE_TMPL_FILE_NOT_FOUND);
    }

    item_list_shm = create_shm(pool,
                               UploadItemList::get_memory_size(MAX_LIST_SIZE));
    thumbnail_list_shm = create_shm(pool,
                                    ThumbnailList::get_memory_size(MAX_LIST_SIZE));

    thumbnail_list = ThumbnailList::get_instance(thumbnail_list_shm,
                                                 MAX_LIST_SIZE);
    item_list = UploadItemListReader::read(pool, data_dir_path, data_dir_path,
                                           MAX_FILE_SIZE, MAX_LIST_SIZE,
                                           item_list_shm, thumbnail_list,
                                           &mtime);

    show_item("data dir", data_dir_path);
    show_item("template file", tmpl_file_path);
    show_item("dump level", dump_level);
    show_line();

    run_exec(pool, item_list, tmpl_file_path, dump_level);
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
