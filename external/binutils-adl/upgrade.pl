#!/usr/bin/env perl

#
# This script is designed for upgrading the binutils tree.  For every C file or
# header, it copies over the same file from a binutils source tree.  The idea is
# that this gives you a start, you then figure out what edits or extra files are
# required.
#
use File::Find;

if (!@ARGV) {
	print "usage:  $0 <upgrade-source-path>\n";
	exit 1;
}

my $upath = $ARGV[0];

if (! -d $upath ) {
	print "Upgrade path '$upath' is not an existing directory.\n";
	exit 1;
}

find (\&handle_file,".");

sub handle_file {
	if ( $_ eq ".svn") {
	 	$File::Find::prune = 1;
	} 
	elsif ( /\.[hc]$/ ) {
		#print "File: $_, $File::Find::name\n";
		my $src = "$upath/$File::Find::name";
		if ( ! -f $src ) {
			unlink $_;
			print "Source file $_ does not exist in new tree (removing from existing tree).\n";
		} else {
			my $cmd = "cp $src $_";
			#print "Command to run:  $cmd\n";
 			system $cmd and die "Could not copy $src to $File::Find::name:  $!";
			#print "Updating $_\n";
		}
	}
}
