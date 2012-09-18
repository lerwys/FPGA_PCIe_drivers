#!perl
#########################

use Test::More tests => 4;
use Test::Exception;
BEGIN { use_ok('MPRACEX::ABB'); };

#########################

my $abb = MPRACEX::ABB->new(board_number => 0);
my $fifo = $abb->fifo;
isa_ok($fifo, 'MPRACEX::ABB::FIFO');

my $status = $fifo->status;

diag("FIFO status is $status");

ok(int($status) == $status, 'Status is numeric');

# TODO: write stuff to fifo
# see if fifo status increases
# read stuff from fifo
# see if fifo status decreases

$fifo->clear;
$status = $fifo->status;

is($fifo->status, 1, "FIFO status 1 after clearing");
