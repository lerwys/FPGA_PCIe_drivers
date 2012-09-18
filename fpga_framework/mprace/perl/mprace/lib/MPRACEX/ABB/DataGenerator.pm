package MPRACEX::ABB::DataGenerator;

use Moose;
use mprace;
use Math::BaseCnv;
use v5.10;

has '_board' => (
	isa => 'Board',
	is => 'rw',
);

has '_abb' => (
	isa => 'MPRACEX::ABB',
	is => 'ro',
);

# After building the object, check if the data generator is present on card and
# throw an exception if not.
sub BUILD {
	my $self = shift;

	die "Data Generator not present\n"
		unless ($self->_abb->capabilities ~~ "data_generator");
}

# Returns 1 if the data generator is currently producing data and 0 if it's not
sub busy {
	my $self = shift;

	return int(($self->_board->getReg((0x00A8 >> 2)) & 0x2) != 0);
}

sub write_daq_data {
	my $self = shift;
	my $arg = shift;

	my $data = 0;
	$data |= (1 << 17) if ($arg->{start_of_frame});
	$data |= (1 << 16) if ($arg->{end_of_frame});
	$data |= ($arg->{data} & 0xFFFF);

	#print "Writing daq data 0x" . cnv($data, 10, 16) . " (bits = " . cnv($data, 10, 2) . ") to " . ((0xC0000 >> 2) + $arg->{offset}) . "\n";
	$self->_board->write((0xC0000 >> 2) + $arg->{offset}, $data);
}

sub write_descriptor {
	my $self = shift;
	my $arg = shift;

	my $descriptor = 0;
	my $traffic_class = 1; # TODO

	$descriptor |= (1 << 31) if ($arg->{enable});
	$descriptor |= (1 << 30) if ($arg->{stop});

	# 29:28 = lowest two bits of the descriptor_class
	$descriptor |= (($traffic_class & 0x3) << 28);

	# 27:16 = next address (in 64-bit mode!)
	$descriptor |= (($arg->{next_address} & 0xFFF) << 16);

	# 15:00 = delay count
	$descriptor |= $arg->{delay_count};

	#print "Writing descriptor 0x" . cnv($descriptor, 10, 16) . " (bits = " . cnv($descriptor, 10, 2) . ") to " . ((0xC0000 >> 2) + $arg->{offset}) . "\n";
	$self->_board->write((0xC0000 >> 2) + $arg->{offset}, $descriptor);
}

# Reset the data generator so that it parses the first descriptor. Note that
# this does not stop the data generator, it only sets it back to the first
# descriptor where it will continue to process as before.
sub reset {
	my $self = shift;

	$self->_board->setReg((0x00A8 >> 2), (0x0A));
}

sub stop {
	my $self = shift;
	my $board = $self->_board;

	$self->write_daq_data({
		offset => 0x1,
		start_of_frame => 0,
		end_of_frame => 1,
		data => 0x0,
	});

	$self->write_descriptor({
		offset => 0x0,
		enable => 0,
		stop => 1,
		traffic_class => 'daq',
		next_address => 1,
		delay_count => 0,
	});

	$self->reset;
}

sub store_pattern {
	my $self = shift;
	my $arg = shift;
	my $offset = 0x2;
	my @pattern = @{$arg->{pattern}};

	die "Pattern not aligned, must contain a multiple of 4 elements" if ((@pattern % 4) != 0);

	$self->stop;

	$self->write_daq_data({
		offset => 0x1,
		start_of_frame => 1,
		end_of_frame => 0,
		data => $pattern[0],
	});

	for (my $c = 1; $c < @pattern; $c++) {
		my $is_last_descriptor = ($c == (@pattern - 1));
		my $next_address = (($arg->{loop} and $is_last_descriptor) ? 0 : ($offset / 2) + 1);

		$self->write_descriptor({
			offset => $offset,
			enable => 0,
			stop => (not $arg->{loop} and $is_last_descriptor),
			traffic_class => 'daq',
			next_address => $next_address,
			delay_count => 0,
		});

		$self->write_daq_data({
			offset => $offset + 1,
			start_of_frame => (($c % 4) == 0),
			end_of_frame => ((($c + 1) % 4) == 0),
			data => $pattern[$c],
		});

		$offset += 2;
	}

	$self->reset;

	$self->write_descriptor({
		offset => 0,
		enable => 1,
		stop => 0,
		traffic_class => 'daq',
		next_address => 1,
		delay_count => 0,
	});

	$self->reset;
}

1
