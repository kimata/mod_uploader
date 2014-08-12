#!/usr/bin/env perl

use Jcode;

###############################################################################
# 設定

# 次の二行を環境に合わせて書き換えてください．
# デフォルトでは http://192.168.2.1:8080/up/ に設置した場合の設定になっ
# ています．
use constant HOST       => '192.168.2.1:8080'; # ホスト名
use constant PATH       => '/up'; # パス
###############################################################################

my $post_data;

while (<>) {
    $post_data .= $_;
}

$post_data = jcode($post_data)->euc;
$post_data =~ s/\r\n/\n/g;
$post_data =~ s/\n/\r\n/g;

print <<"__TEXT__";
POST @{[PATH]}/mail_upload/ HTTP/1.1
Host: @{[HOST]}
Connection: Close
Content-Length: @{[length($post_data) + 1]}

@{[$post_data]}
__TEXT__


