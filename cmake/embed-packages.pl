#!/usr/bin/perl
use strict;
use warnings;
use File::Basename;

local $/ = undef;

my @packages;
while (my $file = <ARGV>) {
	push @packages, {
		content => $file,
		file => basename($ARGV, '.lua'),
	};
}

@packages = sort { $a->{file} cmp $b->{file} } @packages;

my $n = @packages;

print <<HEADER;
typedef struct {
	char *filename;
	char *content;
} Package;

static const Package packages[$n] = {
HEADER

foreach my $pkg (@packages) {
	print qq|\t{"$pkg->{file}",\n|;
	foreach my $line (split(/\n/, $pkg->{content})) {
		$line =~ s/\\/\\\\/g;
		$line =~ s/\"/\\"/g;
		print qq|\t\t"$line\\n"\n|
	}
	print "\t},\n";
}

print <<FOOTER;
};
FOOTER
