package MPRACEX::ABB::FIFO;

use Moose;
use mprace;
use Time::HiRes qw(nanosleep);

has '_board' => (
	isa => 'Board',
	is => 'rw',
);

sub status {
	my $self = shift;

	my $s = $self->_board->getReg(0x24);
	print "(actually: status = $s)\n";
	return $s;
	# TODO: ignore the lower three bits
}

sub clear {
	my $self = shift;
	my $board = $self->_board;

	# Immediately return if the FIFO is already empty
	return if $self->status == 1;

	$board->setReg(0x24, 0x0A);

	# Sleep 500 nSec to be sure the status register will be updated.
	# The FIFO clear itself will take about 3.5 mSec, so sleeping
	# here does not really slow us down.
	nanosleep(500);

	# FIFO status will be 0 while clearing, so wait until we're done
	while ($self->status > 1 or $self->status == 0) {
		print "waiting, status = " . $self->status . "\n";
	}
	print "After my loop\n";
}

1
