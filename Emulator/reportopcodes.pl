#!/usr/bin/perl
%ops = ();
while (<>) {
  chomp;
  #print "[$_]\n";
  if ($_ =~ /^const int O_(\S+)\s*=\s*0x(\S+);/) {
    $ops{$1} = hex($2);
    #print "-- Storing op $1 as $2 ($ops{$1})\n";
  }
}
@sortedkeys = sort { $ops{$a} <=> $ops{$b} } keys (%ops);
foreach (@sortedkeys) {
  print "$_: $ops{$_}\n";
}
