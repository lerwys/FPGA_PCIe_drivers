package MPRACEX::ABB;

use Moose;
use mprace;
use MPRACEX::ABB::FIFO;
use MPRACEX::ABB::DataGenerator;

has 'board_number' => (
	isa => 'Int',
	is => 'ro',
	required => 1,
);

has 'design_id' => (
	isa => 'Int',
	is => 'ro',
	init_arg => undef,
	lazy_build => 1,
);

has '_board' => (
	isa => 'Board',
	is => 'rw',
	init_arg => undef,
	lazy_build => 1
);

has 'fifo' => (
	isa => 'MPRACEX::ABB::FIFO',
	is => 'ro',
	init_arg => undef,
	lazy_build => 1,
);

has 'data_generator' => (
	isa => 'MPRACEX::ABB::DataGenerator',
	is => 'ro',
	init_arg => undef,
	lazy_build => 1,
);

has 'capabilities' => (
	isa => 'ArrayRef',
	is => 'ro',
	init_arg => undef,
	lazy_build => 1,
);

sub _build__board {
	my $self = shift;

	return Board->new('ABB', $self->board_number);
}

sub _build_design_id {
	my $self = shift;

	return $self->_board->getReg(0);
}

sub _build_fifo {
	my $self = shift;

	return MPRACEX::ABB::FIFO->new(_board => $self->_board);
}

sub _build_data_generator {
	my $self = shift;

	return MPRACEX::ABB::DataGenerator->new(_board => $self->_board, _abb => $self);
}

sub _build_capabilities {
	my $self = shift;
	my @caps;

	my $bits = $self->_board->getReg(0x08);

	push @caps, 'data_generator' if ($bits & 0x0020);

	return [ @caps ];
}

1
