#!/usr/bin/env perl
###############################################################################
# Copyright (C) 2006 Tetsuya Kimata <kimata@acapulco.dyndns.org>
#
# All rights reserved.
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any
# damages arising from the use of this software.
#
# Permission is granted to anyone to use this software for any
# purpose, including commercial applications, and to alter it and
# redistribute it freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must
#    not claim that you wrote the original software. If you use this
#    software in a product, an acknowledgment in the product
#    documentation would be appreciated but is not required.
#
# 2. Altered source versions must be plainly marked as such, and must
#    not be misrepresented as being the original software.
#
# 3. This notice may not be removed or altered from any source
#    distribution.
#
# $Id: win_test.pl 1930 2006-11-04 15:38:59Z svn $
###############################################################################

use strict;

use Expect;
use Term::ReadLine;
use Term::ReadPassword;

use constant CMD_TIMEOUT_SEC    => 5;
use constant MAKE_TIMEOUT_SEC   => 3600;
use constant WINDOWS_HOSTNAME   => 'brazil';
use constant VSVARS_PATH        => 'C:/Develop/IDE/VisualStudio/Common7/Tools/vsvars32.bat';
use constant ICLVARS_PATH       => 'C:/Develop/Language/C++/Intel/Compiler/C++/9.1/IA32/Bin/iclvars.bat';
use constant DISK_DEVICE        => 'Z:';
use constant DISK_PATH          => '\\\\columbia\\kimata';
use constant MOD_UPLOADER_PATH  => 'Z:/prog/Apache/Uploader2';
use constant APACHE20_PATH      => 'C:/Server/Apache-2.0';
use constant APACHE22_PATH      => 'C:/Server/Apache-2.2';


sub get_pass {
    return read_password(qq|pass: |);
}

sub create_exp {
    my $exp = Expect->new();
    $exp->raw_pty(0);
    return $exp;
}

sub ssh_login {
    my $exp = shift;
    my $pass = shift;

    $exp->spawn('cocot', qw(-t EUC-JP -p CP932 ssh), WINDOWS_HOSTNAME) or die $!;
    $exp->expect(CMD_TIMEOUT_SEC, 'password:');
    $exp->send($pass . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, 'successfully');
}

sub mount_disk {
    my $exp = shift;
    my $user = shift;
    my $pass = shift;

    $exp->send('net use ' . DISK_DEVICE . ' ' . quotemeta(DISK_PATH) .
               ' /USER:' . $user . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, '$ ');
}

sub init_tree {
    my $exp = shift;

    $exp->send('base_dir=`mktemp -d $TEMP/mod_uploader.XXXXXX`' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, '$ ');

    $exp->send('cp -pr ' . MOD_UPLOADER_PATH . '/work/mod_uploader/* ' .
               '$base_dir' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, '$ ');

    $exp->send('cp -pr ' . APACHE20_PATH . '/Apache2/bin/*.dll $base_dir/src' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, '$ ');

    $exp->send('cp -pr ' . APACHE22_PATH . '/bin/*.dll $base_dir/src' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, '$ ');
}

sub cd_src_dir {
    my $exp = shift;

    $exp->send('cd $base_dir/src' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, '$ ');
}

sub set_env {
    my $exp = shift;
    my $cc = shift;

    $exp->send('cmd' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, '>');

    $exp->send(VSVARS_PATH . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, '>');

    if ($cc eq 'icc') {
        $exp->send(ICLVARS_PATH . "\n");
        $exp->expect(CMD_TIMEOUT_SEC, '>');
    }
}

sub exit_cmd {
    my $exp = shift;

    $exp->send('exit' . "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, '$ ');
}

sub make_check {
    my $exp = shift;
    my $cc = shift;

    $exp->send('make -f GNUmakefile.win32 clean CC=' . $cc . "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, 'src>');

    $exp->send('make -f GNUmakefile.win32 CC=' . $cc . "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, 'src>');

    $exp->send('make -f GNUmakefile.win32 clean-obj CC=' . $cc . "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, 'src>');

    $exp->send('make -f GNUmakefile.win32 test-run CC=' . $cc. "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, 'src>');
}

sub make_test {
    my $exp = shift;
    my $cc = shift;
    my $test = shift;

    $exp->send('make -f GNUmakefile.win32 ' . $test . '.run CC=' . $cc. "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, 'src>');
}

my $user = $ENV{'USER'};
my $cc   = shift @ARGV || 'cl';
my $test = shift @ARGV;
my $repeat = shift @ARGV || 1;
my $pass = $ENV{'WIN_PASS'} || get_pass();
my $exp = create_exp();

my $temp_dir = `mktemp -d -t mod_uploader.XXXXXX 2>/dev/null`;
chomp($temp_dir);

# NOTE:
# Samba 経由だと遅いので，ツリーを Windows のローカルドライブに
# コピーしてからコンパイルを行う．

# ツリーの準備
`rm -rf work; ln -s $temp_dir work`;
`svn export . work/mod_uploader`;
`cd work/mod_uploader; ./configure`;

# ログイン
ssh_login($exp, $pass);
# ディスクのマウント
mount_disk($exp, $user, $pass);
# ツリーの準備
init_tree($exp);

# ディレクトリの移動
cd_src_dir($exp);
# 環境変数のセット
set_env($exp, $cc);
if (defined $test) {
    foreach (1..$repeat) {
        make_test($exp, $cc, $test);
    }
} else {
    make_check($exp, $cc);
}
exit_cmd($exp);

`rm -rf work $temp_dir`;

# Local Variables:
# mode: cperl
# coding: euc-japan-unix
# End:
