#!/usr/bin/perl
use strict;
use warnings;
use lib '.shake/lib';
use Shake::Configure;


configure(
	['PKGCONFIG' => program('pkg-config')],
	['LUA'       => \&lua],
);

sub lua {
	my $ctx = shift;
	my $pkgconfig = $ctx->get('PKGCONFIG') or die "Need pkg-config to find lua.\n";
	my @luas = qw( lua lua51 lua50 );
	foreach my $lua (@luas) {
		if (system($pkgconfig, '--exists', $lua) == 0) {
			return $lua;
		}
	}
	return undef;
}
