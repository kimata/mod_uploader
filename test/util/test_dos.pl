#!/usr/bin/env perl

# 同時に複数のアップロードが行われても正常動作することを確認します．

use strict;

use threads;
use File::Basename;
use Test::More;

BEGIN {
    push(@INC, dirname($0));
}

use UploaderTester;

use constant BASE_URL       => 'http://columbia:8080/up/';
use constant SMALL_FILE     => dirname($0) . '/../data/small.dat';
use constant LARGE_FILE     => dirname($0) . '/../data/large.dat';
use constant TEST_COMMENT   => 'test';
use constant TEST_DL_PASS   => 'D';
use constant TEST_RM_PASS   => 'R';

use constant VIEW_WORKER    => 10;
use constant SMALL_COUNT    => 40;
use constant LARGE_COUNT    => 10;

plan tests => SMALL_COUNT*2 + LARGE_COUNT*2;

my $stop_view : shared = 0;
my @threads;

sub upload {
    my $file = shift;
    my $tester;

    $tester = UploaderTester->new(BASE_URL);
    unlike($tester->upload($file, TEST_COMMENT, TEST_DL_PASS, TEST_RM_PASS),
           qr/Error/i,
           "upload start");

    return 0;
}

sub view {
    while (1) {
        my $tester;

        $tester = UploaderTester->new(BASE_URL);
        eval {
            $tester->get_item($tester->get_latest_item_id());
        };

        if ($stop_view) {
            return 0;
        }
    }
}

my @conf = (
    {
        file  => SMALL_FILE,
        count => SMALL_COUNT,
    },
    {
        file  => LARGE_FILE,
        count => LARGE_COUNT,
    },
);

# view
foreach (1..VIEW_WORKER) {
    threads->new(\&view);
}

# upload
foreach my $conf (@conf) {
    foreach (1..$conf->{count}) {
        push(@threads, threads->new(\&upload, $conf->{file}));
    }

    while (my $thread = pop(@threads)) {
        ok($thread->join() == 0, "upload finish");
    }
}

# finish
$stop_view = 1;
foreach my $thread (threads->list()) {
    $thread->join();
}

# Local Variables:
# mode: cperl
# coding: euc-japan-unix
# End:
