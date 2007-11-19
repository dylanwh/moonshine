#!/usr/bin/perl
use strict;
use warnings;
use File::Basename;

local $/ = undef;

my @packages;
while (my $content = <ARGV>) {
	my $module = basename(dirname($ARGV)) . "." . basename($ARGV, '.lua');
	$module =~ s/^lua.//;
	push @packages, {
		module => $module,
		content => $content,
	};
}

@packages = sort { $a->{module} cmp $b->{module} } @packages;

my $n = @packages + 1;

print <<HEADER;
typedef struct {
	char *module;
	char *content;
} Package;

static const Package packages[$n] = {
HEADER

foreach my $pkg (@packages) {
	print qq|\t{"$pkg->{module}",\n|;
	foreach my $line (split(/\n/, $pkg->{content})) {
		$line =~ s/\\/\\\\/g;
		$line =~ s/\"/\\"/g;
		$line =~ s/\?\?/?""?""/g;
		print qq|\t\t"$line\\n"\n|
	}
	print "\t},\n";
}

print <<FOOTER;
	{ 0, 0 }
};
FOOTER
