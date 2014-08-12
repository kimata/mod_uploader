#!/usr/bin/env perl

sub encode {
    my $data = shift;

    $data =~ s/(.)/'\\x' . unpack('H2', $1)/eg;

    return $data;
}

while (my $line = <>) {
    if ($line =~ /^#define /) {
        $line =~ s/"(.+)"$/'"' . encode($1) . '"'/e;
    }
    print $line;
}
