#########################

use Test::More tests => 4;
use Test::Exception;
BEGIN { use_ok('mprace') };

#########################

my $abb = Board->new('ABB', 0);
isa_ok($abb, 'Board');

is($abb->dmabuffer(29), 4);

diag("Design ID = " . $abb->getReg(0));

# Creating a Board with unknown card type
throws_ok { Board->new('doesntwork', 0) } qr/Unknown board type/, 'Unknown board type';
