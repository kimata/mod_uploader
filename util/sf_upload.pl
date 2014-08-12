#!/usr/bin/env perl

use strict;

use File::Basename;
use Getopt::Long;
use Term::ReadPassword;
use WWW::SourceforgeJP;

use constant PROJECT    => 'mod-uploader';
use constant USER       => 'kimata';
use constant SF_URL     => 'http://sourceforge.jp';

my $pass;
my %args = ();
my $sf;

GetOptions(\%args, "package=s", "mode=s", "version=s", "file=s");

$pass = $ENV{'SF_PASS'} || read_password('sourceforge pass: ', 0, 1);

$sf = WWW::SourceforgeJP->new(PROJECT);
$sf->login(USER, $pass);

if ($args{'mode'} eq 'release') {
    die '--file が指定されていません．' unless (defined $args{'file'});
    die 'ファイル (' . $args{'file'} . ') が存在しません．' unless (-e $args{'file'});

    $sf->create_release($args{'package'}, $args{'version'}, $args{'file'});
} else {
    my @links = $sf->get_release_links();
    my $src_link = (grep($_->text() =~ /^mod_uploader-.*\.tgz$/, @links))[0];
    my $rpm_link = (grep($_->text() =~ /^mod_uploader-.*\.x86_64\.rpm$/, @links))[0];
    my $srpm_link = (grep($_->text() =~ /^mod_uploader-.*\.src\.rpm$/, @links))[0];
    my $deb_link = (grep($_->text() =~ /^libapache2-mod-uploader_.*\.deb$/, @links))[0];
    my $win32_bin_link = (grep($_->text() =~ /^mod_uploader-.*\.msi$/, @links))[0];

    while (<>) {
        if (/\* `mod_uploader-\d+\.\d+\.\d+\.tgz/) {
            print << "__HTML__";
* `@{[$src_link->text()]}[0] <@{[SF_URL]}@{[$src_link->url()]}[0]>`_
__HTML__
        } elsif (/\* `mod_uploader-\d+\.\d+\.\d+-1\.x86_64\.rpm/) {
            print << "__HTML__";
* `@{[$rpm_link->text()]}[0] <@{[SF_URL]}@{[$rpm_link->url()]}[0]>`_  (for CentOS, FedoraCore 64bit)
__HTML__
        } elsif (/\* `mod_uploader-\d+\.\d+\.\d+-1\.src\.rpm/) {
            print << "__HTML__";
* `@{[$srpm_link->text()]}[0] <@{[SF_URL]}@{[$srpm_link->url()]}[0]>`_  (for CentOS, FedoraCore)
__HTML__
        } elsif (/\* `libapache2-mod-uploader_\d+\.\d+\.\d+-1_amd64\.deb/) {
            print << "__HTML__";
* `@{[$deb_link->text()]}[0] <@{[SF_URL]}@{[$deb_link->url()]}[0]>`_  (for Debian, Ubuntu 64bit)
__HTML__
        } elsif (/\* `mod_uploader-\d+\.\d+\.\d+\.msi/) {
            print << "__HTML__";
* `@{[$win32_bin_link->text()]}[0] <@{[SF_URL]}@{[$win32_bin_link->url()]}[0]>`_
__HTML__
        } else {
            s/mod_uploader-.*\.msi/$win32_bin_link->text()/eg;
            print;
        }
    }
}

# Local Variables:
# mode: cperl
# coding: euc-japan-unix
# End:
