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
# $Id: win_build.pl 1928 2006-11-04 14:47:16Z svn $
###############################################################################

use strict;

use Expect;
use Term::ReadLine;
use Term::ReadPassword;

use constant CMD_TIMEOUT_SEC    => 10;
use constant MAKE_TIMEOUT_SEC   => 3600;
use constant HOSTNAME           => 'debian';
use constant MOD_UPLOADER_PATH  => '$HOME/prog/Apache/Uploader2';
use constant BUILD_DIR          => 'build';

sub create_exp {
    my $exp = Expect->new();
    $exp->raw_pty(0);
    return $exp;
}

sub ssh_login {
    my $exp = shift;
    my $pass = shift;

    $exp->spawn('ssh', HOSTNAME) or die $!;
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');
}

sub init_tree {
    my $exp = shift;

    $exp->send('cd `mktemp -d`' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');

    $exp->send('svn update ' . MOD_UPLOADER_PATH . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');

    $exp->send('svn export ' . MOD_UPLOADER_PATH . ' ' . BUILD_DIR . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');

    $exp->send('cd ' . BUILD_DIR . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');
}

sub build_deb {
    my $exp = shift;

    $exp->send('version=`grep AC_INIT configure.ac|' .
               'perl -pe \'s/^[^,]+\D+([\d.]+)\D.*$/\1/\'`-1;' .
               'dch -v $version "mod_uploader $version release."' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');

    $exp->send('debuild --no-tgz-check -us -uc' . "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, HOSTNAME . '%');
}

sub copy_deb {
    my $exp = shift;

    $exp->send('rm -rf '. MOD_UPLOADER_PATH . '/*.deb' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');

    $exp->send('cp ../*.deb '. MOD_UPLOADER_PATH . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');

    system("scp '@{[HOSTNAME]}:@{[MOD_UPLOADER_PATH]}/*.deb' .");
}

my $exp = create_exp();

# ログイン
ssh_login($exp);
# ツリーの準備
init_tree($exp);
# パッケージの作成
build_deb($exp);
# パッケージのコピー
copy_deb($exp);

# Local Variables:
# mode: cperl
# coding: euc-japan-unix
# End:
