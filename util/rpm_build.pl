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
use constant HOSTNAME           => 'centos';
use constant MOD_UPLOADER_PATH  => '$HOME/prog/Apache/Uploader2';

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

sub clean {
    my $exp = shift;

    $exp->send('sudo rm -rf /usr/src/redhat/RPMS/**/mod_uploader-*.rpm' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');

    $exp->send('sudo rm -rf /usr/src/redhat/SRPMS/**/mod_uploader-*.rpm' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');
}

sub init_tree {
    my $exp = shift;

    $exp->send('cd ' . MOD_UPLOADER_PATH . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');

    $exp->send('svn update' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');

    $exp->send('./configure' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');

    $exp->send('rm -rf *.tgz' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');

    $exp->send('make -f GNUmakefile.dist' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');

    $exp->send('sudo mv mod_uploader-*.tgz /usr/src/redhat/SOURCES/' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');

    $exp->send('sudo rm -rf /usr/src/redhat/RPMS/**/mod_uploader-*.rpm' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');
}

sub build_rpm {
    my $exp = shift;

    $exp->send('sudo chown -R root:root *' . "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, HOSTNAME . '%');

    $exp->send('sudo rpmbuild -ba util/mod_uploader.spec' . "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, HOSTNAME . '%');

    $exp->send('sudo chown -R kimata:kimata *' . "\n");
    $exp->expect(MAKE_TIMEOUT_SEC, HOSTNAME . '%');
}

sub copy_rpm
    {
    my $exp = shift;

    $exp->send('rm -rf '. MOD_UPLOADER_PATH . '/*.rpm' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');

    $exp->send('cp /usr/src/redhat/RPMS/**/mod_uploader-*.rpm .' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');

    $exp->send('cp /usr/src/redhat/SRPMS/**/mod_uploader-*.rpm .' . "\n");
    $exp->expect(CMD_TIMEOUT_SEC, HOSTNAME . '%');

    system("scp '@{[HOSTNAME]}:@{[MOD_UPLOADER_PATH]}/*.rpm' .");
}

my $exp = create_exp();

# ログイン
ssh_login($exp);
# 古い RPM の削除
clean($exp);
# ツリーの準備
init_tree($exp);
# パッケージの作成
build_rpm($exp);
# パッケージのコピー
copy_rpm($exp);

# Local Variables:
# mode: cperl
# coding: euc-japan-unix
# End:
