package Shake::Configure;
use Exporter;
use base 'Exporter';
use Shake::Context;
use IO::File;

our @EXPORT = qw( configure program generate );

sub configure {
	my $context = new Shake::Context;

	while (my $pair = shift @_) {
		my ($k, $f) = @$pair;
		print "check: $k = ";
		$context->{prefix} = $k;
		my $v = $f->($context);
		$context->set($k => $v);
		print "$v\n";
	}
	return $context;
}

sub program ($) {
	my $prog = shift;
	my @path = $^O ne 'MSWin32' ? split(/:/, $ENV{PATH}) : split(/;/, $ENV{PATH});
	return sub { 
		foreach my $dir (@path) {
			my $path = File::Spec->join($dir, $prog);
			if ($^O ne 'MSWin32') {
				if (-x $path) {
					return $path;
				}
			} else {
				foreach my $suf (qw( bat exe cmd )) {
					if (-e "$path.$suf") {
						return "$path.$suf";
					}
				}
			}
		}

		return undef;
	}
}

sub generate {
	my ($ctx, @filenames) = @_;
	foreach my $filename (@filenames) {
		print "Generating $filename\n";
		my $in  = new IO::File("$filename.in", 'r') or die "Can't open $filename.in for reading";
		my $out = new IO::File($filename, 'w')      or die "Can't open $filename for writing";
		while (my $line = $in->getline) {
			$line =~ s/@(\S+)@/$ctx->get($1) || ''/ge;
			$out->print($line);
		}
	}
}

1;
