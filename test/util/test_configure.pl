#!/usr/bin/env perl

# 様々な configure オプションでコンパイルできることを確認します．

use strict;
use warnings;

use Storable;
use Term::ANSIColor;

use constant VERBOSE        => 0;
use constant AP13_APXS      => '/home/kimata/local/apache13/bin/apxs';
use constant CACHE          => '.test_configure.cache';
use constant CONFIGURE      => './configure';
use constant CONFIG_OPTIONS => [
    { command               => '--enable-atomic-builtins' },
    { command               => '--enable-empty-comment' },
    { command               => '--enable-empty-password' },
    { command               => '--enable-remove-unpopular' },
    { command               => '--enable-thumbnail' },
    { command               => '--enable-numname' },
    { command               => '--enable-fastest' },
    { command               => '--enable-gzip' },
    { command               => '--enable-debug' },
    {
        command             => '--enable-apache13',
        with                => '--with-apxs=' . AP13_APXS,
    },
    {
        command             => '--with-writer',
        args                => [qw(basic mmap)],
    },
];

sub run {
    my $command = shift;

    if (VERBOSE) {
        return system "$command";
    } else {
        return system "$command >/dev/null 2>&1";
    }
}

sub test_option_combine {
    my $opt_list = shift;
    my $opt_index = shift;
    my $use_list = shift;
    my $func = shift;
    my $opt = $opt_list->[$opt_index];

    if (defined $opt) {
        test_option_combine($opt_list, $opt_index+1, $use_list, $func);

        if (defined $opt->{args}) {
            foreach my $arg (@{$opt->{args}}) {
                test_option_combine
                    ($opt_list, $opt_index+1,
                     [ sprintf("%s=%s", $opt->{command}, $arg),
                       defined $opt->{with} ? (@{$use_list}, $opt->{with})
                                            : @{$use_list} ],
                     $func);
            }
        } else {
             test_option_combine
                 ($opt_list, $opt_index+1,
                  [ $opt->{command},
                    defined $opt->{with} ? (@{$use_list}, $opt->{with})
                                         : @{$use_list} ],
                  $func);
         }
    } else {
        $func->($use_list);
    }
}

my $total = 0;
my $done = 0;
my $cache = {};

if (@ARGV) {
    unlink(CACHE);
} elsif (-e CACHE) {
    $cache = retrieve(CACHE);
}

test_option_combine(CONFIG_OPTIONS, 0, [], sub { $total++; });
test_option_combine
    (CONFIG_OPTIONS, 0, [],
     sub {
         my $use_list = shift;
         my $conf_opts = join(' ', sort @{$use_list});
         my $ret = 0;
         local $| = 1;

         $done++;
         printf "%4d / %4d ", $done, $total;

         if (!exists $cache->{$conf_opts}) {
             $ret |= run(CONFIGURE . ' ' . $conf_opts);
             $ret |= run('make -j2');
         }

         if ($ret == 0) {
             print colored('[OK]', 'bold green'),
                 ' ', join(', ', @{$use_list}), "\n";

             $cache->{$conf_opts} = 1;
             store $cache, CACHE . '.tmp';
             rename CACHE . '.tmp', CACHE;
         } else {
             print colored('[NG]', 'bold red'),
                 ' ', join(', ', @{$use_list}), "\n";
         }
     });
