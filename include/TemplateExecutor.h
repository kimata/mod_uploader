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
 * $Id: TemplateExecutor.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef TEMPLATE_EXECUTOR_H
#define TEMPLATE_EXECUTOR_H

#include "Environment.h"

#include "TemplateParser.h"
#include "TemplateVariable.h"
#include "Uncopyable.h"

/**
 * @brief テンプレートを実行するクラス
 */
template<class W>
class TemplateExecutor: public Uncopyable
{
public:
    TemplateExecutor(apr_pool_t *pool, W& writer);

    void exec(const TemplateParser::node_t *node,
              TemplateVariable::variable_t **variables, apr_size_t key_count);

private:
    // エラーメッセージが見にくくなるので，テンプレート引数は W にする
    typedef W                           ResponseWriterClass;

    typedef TemplateParser              Parser;
    typedef TemplateVariable            Variable;
    typedef Parser::node_t              node_t;
    typedef Variable::variable_t        variable_t;
    typedef Variable::scalar_t          scalar_t;
    typedef Variable::variable_type_t   variable_type_t;

    void prepare_exec(apr_size_t key_count);

    void exec_stmt_list(const node_t *node);
    void exec_stmt(const node_t *node);
    void exec_foreach(const node_t *node);
    void exec_while(const node_t *node);
    void exec_if(const node_t *node);
    void exec_print(const node_t *node);
    void exec_print_element(const node_t *node);
    void exec_print_bank(const node_t *node);
    bool calc_b_val(const node_t *node);
    int calc_i_val(const node_t *node);
    int calc_i_val(const variable_t *var);
    int calc_assign(const node_t *node);
    int calc_assign_var(const node_t *node);
    int calc_assign_str(const node_t *node);
    int calc_assign_int(const node_t *node);
    int calc_assign_diff(const node_t *node, int diff);

    // ハッシュや配列は変更不可にしたいので const を返す
    const variable_t *get_variable(const node_t *node);

    variable_t *get_variable(apr_size_t key);
    variable_t *create_int_var(int i);
    variable_t *create_str_var(const char *s);
    variable_t *create_variable();
    scalar_t *create_scalar();

    void print(const char *str, apr_size_t length);
    void print(const char *str);
    void print(int i);

    apr_pool_t *pool_;
    ResponseWriterClass writer_;

    variable_t **variables_;
    variable_t *var_pool_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
