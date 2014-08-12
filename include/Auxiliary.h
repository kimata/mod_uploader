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
 * $Id: Auxiliary.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef AUXILIARY_H
#define AUXILIARY_H

#include "Environment.h"

#include <cstdlib>

#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 255
#endif

#include "apr_pools.h"
#include "apr_time.h"
#include "apr_shm.h"

/**
 * 文字列を URL エンコードします．
 *
 * @param[in] pool プール
 * @param[in] str 文字列
 * @return URL エンコードされた文字列
 */
const char *rfc2396_encode(apr_pool_t *pool, const char* str);
/**
 * アトミック関数を初期化します．
 *
 * @param[in] pool プール
 */
void init_atomic(apr_pool_t *pool);
/**
 * 共有メモリを作成します．
 *
 * @param[in] pool プール
 * @param[in] size サイズ
 * @param[in] file_path ファイルのパス
 * @return 共有メモリ
 */
apr_shm_t *create_shm(apr_pool_t *pool, apr_size_t size,
                      const char *file_path=NULL);
/**
 * 子プロセス用に共有メモリを初期化します．
 *
 * @param[in] pool プール
 * @param[in] shm 共有メモリ
 * @param[in] file_path ファイルのパス
 */
void attach_shm(apr_pool_t *pool, apr_shm_t *shm,
                const char *file_path=NULL);
/**
 * APR の API でエラーが発生しているか調べます．
 * エラーが発生している場合，その内容を標準エラーに出力します．
 *
 * @param[in] pool プール
 * @param[in] status ステータス
 */
void check_apr_error(apr_pool_t *pool, apr_status_t status);
/**
 * APR の API で最後に発生したエラー内容を取得します．
 *
 * @param[in] pool プール
 * @return エラー内容
 */
const char *get_last_error_message(apr_pool_t *pool);
/**
 * APR の API で発生したエラー内容を返します．
 *
 * @param[in] pool プール
 * @param[in] status ステータス
 * @return エラー内容
 */
const char *get_error_message(apr_pool_t *pool, apr_status_t status);
/**
 * 現在時間(秒)を返します．
 */
double get_time_sec();
/**
 * 消費したユーザ時間(秒)を返します．
 */
double get_usage_sec();

apr_uint32_t mask32(apr_uint32_t i);
apr_uint32_t clip32(apr_uint32_t i);
apr_size_t bit_width(apr_size_t x);

/**
 * 数値文字列を apr_size_t 型の数値に変換します．
 *
 * @param[in] s 文字列
 * @return 数値
 */
apr_size_t atosize(const char *s);
/**
 * ファイルの名前から拡張子を求めます．
 *
 * @param[in] file_name ファイルの名前
 * @return ファイルの拡張子
 */
const char *get_file_ext(const char *file_name);
/**
 * ファイルの MIME タイプから画像の拡張子を求めます．
 *
 * @param[in] file_mime ファイルの MIME タイプ
 * @return ファイルの拡張子
 */
const char *get_mime_image_ext(const char *file_mime);
/**
 * ファイルの名前から MIME タイプを求めます．
 *
 * @param[in] file_name ファイルの名前
 * @return ファイルの MIME タイプ
 */
const char *get_image_mime(const char *file_name);
/**
 * パスのファイル部分を返します，
 *
 * @param[in] path パス
 * @return パスのファイル部分
 */
const char *basename_ex(const char *path);
/**
 * パスのディレクトリ部分を返します，
 *
 * @param[in] pool プール
 * @param[in] path パス
 * @return パスのディレクトリ部分
 */
const char *dirname_ex(apr_pool_t *pool, const char *path);
/**
 * HTML のエスケープを行います，
 *
 * @param[in] pool プール
 * @param[in] str 文字列
 * @param[in] str_lenght 文字列の長さ
 * @param[out] escaped_str エスケープされた文字列
 * @param[out] escaped_lenght エスケープされた文字列の長さ
 */
void escape_html(apr_pool_t *pool,
                 const char *str, apr_size_t str_length,
                 const char **escaped_str, apr_size_t *escaped_length);
/**
 * HTML のエスケープを行います，
 *
 * @param[in] pool プール
 * @param[in] str 文字列
 * @return エスケープされた文字列
 */
const char *escape_html(apr_pool_t *pool, const char *str);
/**
 * サイズを文字列で表します．
 *
 * @param[in] pool プール
 * @param[in] size サイズ
 * @return サイズ文字列
 */
const char *size_str(apr_pool_t *pool, apr_uint64_t size);
#ifndef HAVE_STRNCHR
/**
 * 文字列の中から指定された文字の位置を探します，
 *
 * @param[in] s 文字列
 * @param[in] length 文字列の長さ
 * @param[in] c 探索する文字
 * @return 文字が見つかった位置
 */
const char *strnchr(const char *s, size_t length, int c);
#endif
#if !defined(HAVE_MEMMEM) || defined(WIN32)
/**
 * メモリの中から指定されたパターンの位置を探します，
 *
 * @param[in] haystack メモリ
 * @param[in] haystacklen メモリのサイズ
 * @param[in] needle 探索するパターン
 * @param[in] needlelenc 探索するパターンの長さ
 * @return パターンが見つかった位置
 */
void *memmem(const void *haystack, size_t haystacklen,
             const void *needle, size_t needlelen);
#endif
#if !defined(HAVE_STRNDUP) || defined(WIN32)
/**
 * 文字列を複製します．
 *
 * @param[in] s 文字列
 * @param[in] n 文字列の長さ
 * @return 複製した文字列
 */
char *strndup(const char *s, size_t n);
#endif
/**
 * diff コマンドを実行して二つのファイルを比較します．
 *
 * @param[in] a_file_path ファイルのパス
 * @param[in] b_file_path ファイルのパス
 * @return diff コマンドの終了コード
 */
int diff(apr_pool_t *pool, const char *a_file_path, const char *b_file_path);

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
