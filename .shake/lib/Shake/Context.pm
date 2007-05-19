# vim: set ts=4 sw=4 noexpandtab si ai sta tw=100:
# This module is copyrighted, see end of file for details.
package Shake::Context;
use strict;
use warnings;
#use Shake::Base;
#use base 'Shake::Base';
use File::Temp ();

our $VERSION = 0.03;

sub new {
	my ($class) = @_;
	return bless { vars => {} }, $class;
}

sub set {
	my ($self, $k, $v) = @_;
	$self->{vars}{$k} = $v;
}
sub get {
	my ($self, $k) = @_;
	$self->{vars}{$k};
}

sub tempfile {
	my ($self, @args) = @_;
	my $t = $self->{prefix} || 'unknown';
	$t =~ s![\\/.]!_!g;
	my $file = new File::Temp (TEMPLATE => "shake_$t-XXXXXX", @args);
	push @{ $self->{files} }, $file;
	return $file;
}

sub tempfilled {
	my ($self, $content, @args) = @_;
	my $file = $self->tempfile(@args);
	print $file $content;
	close $file;
	return $file->filename;
}

sub tempname {
	my ($self, @args) = @_;
	my $file = $self->tempfile(@args);
	close $file;
	unlink $file;
	return "$file";
}

sub tempexe { shift->tempname($^O eq 'MSWin32' ? (SUFFIX => '.exe') : () ) }

1;
