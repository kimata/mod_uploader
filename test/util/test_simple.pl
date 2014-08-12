#!/usr/bin/env perl

# アップロードと削除が正常に行えることを確認します．

use strict;

use File::Basename;
use Test::More;

BEGIN {
    push(@INC, dirname($0));
}

use UploaderTester;

use constant BASE_URL       => 'http://localhost:8080/up/';
use constant TEST_FILE      => dirname($0) . '/../data/image.png';
use constant TEST_COMMENT   => 'test';
use constant TEST_DL_PASS   => 'D';
use constant TEST_RM_PASS   => 'R';

use constant UPLOAD_COUNT   => 10;

my $tester;
my @item_ids;

plan tests => (UPLOAD_COUNT*2) + 1;

$tester = UploaderTester->new(BASE_URL);

# upload
foreach (1..UPLOAD_COUNT) {
    unlike($tester->upload(TEST_FILE, TEST_COMMENT, TEST_DL_PASS, TEST_RM_PASS),
           qr/Error/i,
           "Upload with file, comment, dl_pass, rm_pass");
    push(@item_ids, $tester->get_latest_item_id());
}

# id
do {
    my %tmp;
    @item_ids = grep { !$tmp{$_}++ } @item_ids;
    is(@item_ids, UPLOAD_COUNT, "Duplication of item id");
};

# download
foreach (reverse(1..UPLOAD_COUNT)) {
    my $i = int(rand($_ - 1));

    unlike($tester->remove($item_ids[$i], TEST_RM_PASS),
           qr/Error/i,
           "Remove item[$i] = $item_ids[$i]");

    splice(@item_ids, $i, 1);
}

# Local Variables:
# mode: cperl
# coding: euc-japan-unix
# End:
