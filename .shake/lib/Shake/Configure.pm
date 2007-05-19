package Shake::Configure;
use Exporter;
use base 'Exporter';
use Shake::Context;

our @EXPORT = qw( configure program );

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


1;
