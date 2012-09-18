#!perl
#########################

use Test::More tests => 11;
use Test::Exception;
use Time::HiRes qw(sleep); # sleep() with fractional values
BEGIN { use_ok('MPRACEX::ABB'); };

#########################

my $abb = MPRACEX::ABB->new(board_number => 0);

my $dgen;
my $fifo = $abb->fifo;
isa_ok($fifo, 'MPRACEX::ABB::FIFO');

lives_ok { $dgen = $abb->data_generator } 'Create DataGenerator object';
isa_ok($dgen, 'MPRACEX::ABB::DataGenerator');

lives_ok { $dgen->stop } 'Survive stopping of the DataGenerator';

is($dgen->busy, 0, 'DataGenerator not busy after stopping');

my $status = $fifo->status;

sleep(0.25);

is($fifo->status, $status, 'FIFO status did not change after stopping dgen');

$fifo->clear;

$status = $fifo->status;

is($status, 1, 'FIFO empty after clearing');

$dgen->store_pattern({
	loop => 1,
	pattern => [ 0x2323, 0xF000, 0xDEAD, 0xFACE, 0x0, 0x1, 0xC0, 0xD0 ],
});

sleep(0.1);

my $new_status = $fifo->status;
is($dgen->busy, 1, 'DataGenerator is busy');
ok($new_status > $status, 'FIFO status increased, data generator seems to work');

$dgen->stop;

is($dgen->busy, 0, 'DataGenerator no longer busy after stopping');
