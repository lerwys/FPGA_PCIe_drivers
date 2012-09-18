#!perl
#########################

use Test::More tests => 5;
use Test::Exception;
use Data::Dumper;
BEGIN { use_ok('MPRACEX::ABB'); };

#########################

my $abb = MPRACEX::ABB->new(board_number => 0);

isa_ok($abb, 'MPRACEX::ABB');

diag("Design ID = " . $abb->design_id);

my $design_id = $abb->design_id;

my $design_version = ($design_id & 0xFF000000) >> 24;
my $design_major_revision = ($design_id & 0x00FF0000) >> 16;
my $author = ($design_id & 0x0000F000) >> 12;
my $design_minor_revision = ($design_id & 0x00000FFF);

is($design_version, 0x1, 'Design version is 0x1');
is($design_major_revision, 0x3, 'Design major revision is 0x3');
is($author, 0x2, 'Design author is 0x2');

diag("ABB supports: " . Dumper($abb->capabilities));
