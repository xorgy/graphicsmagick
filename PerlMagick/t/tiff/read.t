#!/usr/local/bin/perl
#
# Test reading TIFF images
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
BEGIN { $| = 1; $test=1; print "1..5\n"; }
END {print "not ok $test\n" unless $loaded;}

use Graphics::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/tiff' || die 'Cd failed';

#
# 1) Test Reading Monochrome
# 
print("Monochrome ...\n");
testRead ( 'input_mono.tiff',
  '2b48bcf7d93cc31a8deb3026d872f1bb0a300b0f4e177423e5301adc638179e4' );

#
# 2) Test reading PseudoColor
#
++$test;
print("PseudoColor ...\n");
testRead( 'input_256.tiff',
  'a570df6d4c214d08bccaaee856f617883fee4f96494af6ef5e25c76d86ea0e0a' );

#
# 3) Test Reading TrueColor (8-bit)
# 
++$test;
print("TrueColor (8-bit) image ...\n");
testRead( 'input_truecolor.tiff',
  '326599218925e7aa27c735d482332744cccf41f460a1ba2d276a2d172f2b3de0' );

#
# 4) Test Reading TrueColor (16-bit)
#
++$test;
print("TrueColor (16-bit) image ...\n");
testRead( 'input_truecolor_16.tiff',
  '1030dc73bdc72c846831d10b661539e7a9e9c24a170f24ffdffaeae1a8aa769d',
  '72fd047e4f5942b4edc96e8b16444580e9eb1f265017dd6bb11795a842e58d58' );

#
# 5) Test Reading 8-bit TrueColor Tiled (32x32 tiles)
# 
++$test;
print("TrueColor (8-bit) tiled image ...\n");
testRead( 'input_truecolor_tiled32x32.tiff',
  '326599218925e7aa27c735d482332744cccf41f460a1ba2d276a2d172f2b3de0' );
