#!/usr/bin/perl
use strict;
use warnings;
use lib '.shake/lib';
use Shake::Configure;

my $ctx = configure(
	['PKGCONFIG' => program('pkg-config')],
	['SLANG_LDFLAGS' => sub { '-lslang' }],                # FIXME
	['SLANG_CFLAGS'  => sub { '-I/usr/include/slang-2' }], # FIXME
	['LUA'       => sub {
		my $ctx = shift;
		my $pkgconfig = $ctx->get('PKGCONFIG') or die "Need pkg-config to find lua.\n";
		my @luas = qw( lua lua51 lua50 );
		foreach my $lua (@luas) {
			if (system($pkgconfig, '--exists', $lua) == 0) {
				return $lua;
			}
		}
		return undef;
	}],
);

generate($ctx, 'config.mk');
