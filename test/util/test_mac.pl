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
# $Id: mac_test.pl 2031 2006-11-23 13:49:32Z svn $
###############################################################################

use strict;

use Expect;
use Term::ReadLine;

use constant CMD_TIMEOUT_SEC    => 10;
use constant MAKE_TIMEOUT_SEC   => 3600;
use constant MAC_HOSTNAME       => 'blendy';
use constant MOD_UPLOADER_PATH  => 'Documents/prog/Apache/Uploader2';

sub create_exp {
    my $exp = Expect->new();
    $exp->raw_pty(0);
    return $exp;
}

sub ssh_login {
    my $exp = shift;

    $exp->spawn('cocot', qw(-t EUC-JP -p UTF-8 ssh), MAC_HOSTNAME) or die $!;
    $exp->expect(CMD_TIMEOUT_SEC, MAC_HOSTNAME . '%');
}

sub cd_top_dir {
    my $exp = shift;

    $exp->send('cd ' . MOD_UPLOADER_PATH . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, MAC_HOSTNAME . '%');
}

sub svn_update {
    my $exp = shift;

    $exp->send('eval `ssh-agent`; ssh-add ~/.ssh/svn.id_rsa; svn update' . "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, MAC_HOSTNAME . '%');
}

sub make_config {
    my $exp = shift;

    $exp->send('./configure' . "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, '@' . MAC_HOSTNAME . '%');

    $exp->send('export MACOSX_DEPLOYMENT_TARGET=10.4' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, MAC_HOSTNAME . '%');
}

sub make_check {
    my $exp = shift;

    $exp->send('make clean WITH_COLOR=1' . "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, MAC_HOSTNAME . '%');

    $exp->send('make WITH_COLOR=1' . "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, MAC_HOSTNAME . '%');

    $exp->send('sudo make -f GNUmakefile.apache install WITH_COLOR=1' . "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, MAC_HOSTNAME . '%');

    $exp->send('make -C src clean-obj WITH_COLOR=1' . "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, MAC_HOSTNAME . '%');

    $exp->send('make -C src test-run WITH_COLOR=1' . "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, MAC_HOSTNAME . '%');
}

my $user = $ENV{'USER'};
my $exp = create_exp();

# ログイン
ssh_login($exp);
# ディレクトリの移動
cd_top_dir($exp);
# アップデート
svn_update($exp);
# 設定
make_config($exp);
# コンパイル
make_check($exp);

# Local Variables:
# mode: cperl
# coding: euc-japan-unix
# End:
