package WWW::SourceforgeJP;

use strict;
use vars qw($VERSION);

$VERSION = 0.01;

# use Encode;
use WWW::Mechanize;

use constant LIST_RELEASE_LABEL         => 'パッケージ/リリース/ファイル';
use constant LIST_RELEASE_LINK_LABEL    => 'リリース一覧';
use constant EDIT_RELEASE_LABEL         => 'リリースの編集';
use constant EDIT_RELEASE_LINK_LABEL    => 'ファイルリリース管理';
use constant CREATE_RELEASE_LABEL       => 'リリースの新規作成';

# binmode STDOUT, ":encoding(euc-jp)";
# binmode STDERR, ":encoding(euc-jp)";

sub new {
    my $class = shift;
    my $project = shift;

    my $member = {
        project => $project,
        mech => WWW::Mechanize->new(),
    };

    $member->{'mech'}->agent_alias('Windows IE 6');

    bless $member, $class;
}

sub login {
    my $self = shift;
    my $user = shift;
    my $pass = shift;

    $self->{'mech'}->get('https://sourceforge.jp/account/login.php');
    $self->{'mech'}->form_number(2);
    $self->{'mech'}->field('form_loginname', $user);
    $self->{'mech'}->field('form_pw', $pass);
    $self->{'mech'}->click();

    if ($self->{'mech'}->content() =~ /\QInvalid\E/) {
        die 'ログインに失敗しました．';
    }
}

sub create_release {
    my $self = shift;
    my $package = shift;
    my $version = shift;
    my $file_path = shift;
    my $file_name  = (split(/[\/\\]/, $file_path))[-1];
    my @links;

    @links = $self->get_release_links();

    if (grep($_->text() eq $file_name, @links)) {
        die '既に同じ名前のリリースが存在します．';
    }

    $self->_mv_create_release_page($package);

    $self->{'mech'}->form_number(2);
    $self->{'mech'}->field('release_name', $version);
    $self->{'mech'}->field('userfiles[]', $file_path);
    $self->{'mech'}->click();

    @links = $self->get_release_links();

    unless (grep($_->text() eq $file_name, @links)) {
        die 'リリースの作成に失敗しました．';
    }
}

sub get_release_links {
    my $self = shift;
    my @links;

    $self->_mv_list_release_page();

    @links = grep($_->url() =~ m|/projects/[^/]+/downloads|,
                  $self->{'mech'}->find_all_links());

    map { $_->[0] }
        sort { $b->[1] <=> $a->[1] }
            map { [$_, ($_->text() =~ /([\d.]+)/)[0]] } @links;
}


sub _mv_list_release_page {
    my $self = shift;
    my $link;

    $self->{'mech'}->get("/projects/@{[$self->{'project'}]}/admin");

    $link = $self->{'mech'}->find_link(text => LIST_RELEASE_LINK_LABEL);
    $self->{'mech'}->get($link->url());

    if ($self->{'mech'}->content() !~ /\Q@{[LIST_RELEASE_LABEL]}[0]\E/) {
        die 'リリースの一覧ページへの移動に失敗しました．';
    }
}

sub _mv_edit_release_page {
    my $self = shift;
    my $link;

    $self->{'mech'}->get("/projects/@{[$self->{'project'}]}/admin");
    $link = $self->{'mech'}->find_link(text => EDIT_RELEASE_LINK_LABEL);
    $self->{'mech'}->get($link->url());

    if ($self->{'mech'}->content() !~ /\Q@{[EDIT_RELEASE_LABEL]}[0]\E/) {
        die 'リリースの編集ページへの移動に失敗しました．';
    }
}

sub _mv_create_release_page {
    my $self = shift;
    my $package = shift;
    my $link;
    my $url;

    $self->_mv_edit_release_page();
    $link = $self->{'mech'}->find_link(text => $package, n => 2) ||
        $self->{'mech'}->find_link(text => $package, n => 1);

    if (! defined $link) {
        die 'パッケージ (' . $package . ') を見つけられませんでした．';
    }

    $url = $link->url();
    $url =~ s/show_edit_package/show_edit_release/;

    $self->{'mech'}->get($url);

    if ($self->{'mech'}->content() !~ /\Q@{[CREATE_RELEASE_LABEL]}[0]\E/) {
        die 'リリースの新規作成ページへの移動に失敗しました．';
    }
}

1;

# Local Variables:
# mode: cperl
# coding: utf-8-unix
# End:
