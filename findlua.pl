#!/usr/bin/perl
use strict;
use warnings;

my $pkgconfig = shift || 'pkg-config';

my @luas = qw( lua5.1 lua51 lua50 lua );
foreach my $lua (@luas) {
	if (system($pkgconfig, '--exists', $lua) == 0) {
		print "$lua\n";
		exit 0;
	}
}

exit 1;
