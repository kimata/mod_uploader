#!/usr/bin/env perl

package UploaderTester;
use strict;
use vars qw($VERSION);

$VERSION = 0.01;

use WWW::Mechanize;

sub new {
    my $class = shift;
    my $base_url = shift;

    my $member =  {
        base_url => $base_url,
        mech => WWW::Mechanize->new(),
    };

    $member->{'mech'}->proxy('http', '');
    $member->{'mech'}->agent_alias('Windows IE 6');

    bless $member, $class;
}

sub upload {
    my $self = shift;
    my $file_path = shift;
    my $comment = shift;
    my $download_pass = shift;
    my $remove_pass = shift;

    $self->{'mech'}->get($self->{'base_url'});
    $self->{'mech'}->form_number(1);
    $self->{'mech'}->set_fields(
        file => $file_path,
        comment => $comment,
        download_pass => $download_pass,
        remove_pass => $remove_pass,
    );

    $self->{'mech'}->submit();

    return $self->{'mech'}->content();
}

sub remove {
    my $self = shift;
    my $item_id = shift;
    my $remove_pass = shift;

    $self->{'mech'}->get($self->{'base_url'});
    $self->{'mech'}->form_number(2);
    $self->{'mech'}->set_fields(
        id => $item_id,
        remove_pass => $remove_pass,
    );

    $self->{'mech'}->submit();

    return $self->{'mech'}->content();
}

sub get_latest_item_id {
    my $self = shift;

    $self->{'mech'}->get($self->{'base_url'});

    foreach (split(/\n/, $self->{'mech'}->content())) {
        m|/download/(\d+)| or next;

        return $1;
    }

    die 'get_latest_item_id failed';
}

sub get_item {
    my $self = shift;
    my $id = shift;

    $self->{'mech'}->get($self->{'base_url'} . 'download/' . $id);
}

1;

# Local Variables:
# mode: cperl
# coding: euc-japan-unix
# End:
