#!/usr/bin/perl -w
#
#	    File: GenerateMIGOutput.pl
#	Abstract: This post-processes the CMIODPA Server/Client .h & .cpp MIG output.
#
#				This is needed because MIG does not like the scoping operator "::", so the .defs file uses "_SCOPE_" in its place, so the "_SCOPE_" must be replaced with the "::" operator
#				in the generated output.
#				(The "_SCOPE_" text cannot be universally replaced in the .cpp, since it is used in some #defines)
#	
#	 Version: 1.2
#	
#	Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
#	Inc. ("Apple") in consideration of your agreement to the following
#	terms, and your use, installation, modification or redistribution of
#	this Apple software constitutes acceptance of these terms.  If you do
#	not agree with these terms, please do not use, install, modify or
#	redistribute this Apple software.
#	
#	In consideration of your agreement to abide by the following terms, and
#	subject to these terms, Apple grants you a personal, non-exclusive
#	license, under Apple's copyrights in this original Apple software (the
#	"Apple Software"), to use, reproduce, modify and redistribute the Apple
#	Software, with or without modifications, in source and/or binary forms;
#	provided that if you redistribute the Apple Software in its entirety and
#	without modifications, you must retain this notice and the following
#	text and disclaimers in all such redistributions of the Apple Software.
#	Neither the name, trademarks, service marks or logos of Apple Inc. may
#	be used to endorse or promote products derived from the Apple Software
#	without specific prior written permission from Apple.  Except as
#	expressly stated in this notice, no other rights or licenses, express or
#	implied, are granted by Apple herein, including but not limited to any
#	patent rights that may be infringed by your derivative works or by other
#	works in which the Apple Software may be incorporated.
#	
#	The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
#	MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
#	THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
#	FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
#	OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
#	
#	IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
#	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#	INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
#	MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
#	AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
#	STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
#	POSSIBILITY OF SUCH DAMAGE.
#	
#	Copyright (C) 2012 Apple Inc. All Rights Reserved.
#	

$target = $ARGV[0];
$INPUT_FILE_BASE =  $ARGV[1];

if ($target eq "server")
{
	$outputSource = $INPUT_FILE_BASE."Server.cpp";
	$outputHeader = $INPUT_FILE_BASE."Server.h";

	# Open the {INPUT_FILE_BASE}Server.cpp and slurp all of its contents into the @lines array.
	open (MIGFILE,$outputSource) or die "Can't open MIG .cpp for reading $!";
	@lines = <MIGFILE>;
	close MIGFILE;

	# Selectively replace the "_SCOPE_" text with the "::" operator.  It cannot be universally replaced because it appears in 
	# some #defines.
	foreach (@lines)
	{
		# Find the portion of a #define statement where "_SCOPE_" must be replaced.  It should only be done where a type is being
		# specifed.  A double open parantheses "((" marks the location at which it is safe to replace all subsequent "_SCOPE_"
		# occurences on the line.
		while ($_ =~ s/(.*\(\()(.*)_SCOPE_/$1$2::/)
		{
		}

		# Find where "_SCOPE_" is used in a typedef of a structure.  This can be down by looking for
		#	2 tabs - any text - _SCOPE_ - any text followed by a space
		while ($_ =~ s/(\t\t.*)_SCOPE_(.* )/$1::$2/)
		{
		}

		# Find where "_SCOPE_" is used in a cast for a unbounded array.  This can be down by looking for
		#	space - ( - any text - _SCOPE_ - any text followed by a )
		while ($_ =~ s/( \(.*)_SCOPE_(.*\))/$1::$2/)
		{
		}
	}

	# Write the modified {INPUT_FILE_BASE}Server.cpp back to disk.
	open (MIGFILE, "> $outputSource") or die "Can't open MIG .cpp for writing $!";
	print MIGFILE @lines;
	close MIGFILE;



	#
	# {INPUT_FILE_BASE}Server.h
	#

	# Open the {INPUT_FILE_BASE}Server.h and slurp all of its contents into the @lines array.
	open (MIGFILE, $outputHeader) or die "Can't open MIG .h for reading $!";
	@lines = <MIGFILE>;
	close MIGFILE;

	# Replace all occurences of "_SCOPE_" with "::"
	foreach (@lines)
	{
		s/_SCOPE_/::/g
	}

	# Write the modified {INPUT_FILE_BASE}Server.h back to disk.
	open (MIGFILE, "> $outputHeader") or die "Can't open MIG .h for writing $!";
	print MIGFILE @lines;
	close MIGFILE;
}


if ($target eq "client")
{
	$outputSource = $INPUT_FILE_BASE."Client.cpp";
	$outputHeader = $INPUT_FILE_BASE."Client.h";

	# Open the {INPUT_FILE_BASE}Client.cpp and slurp all of its contents into the @lines array.
	open (MIGFILE, $outputSource) or die "Can't open MIG .cpp for reading $!";
	@lines = <MIGFILE>;
	close MIGFILE;

	# Replace all occurences of "_SCOPE_" with "::"
	foreach (@lines)
	{
		# Find the portion of a #define statement where "_SCOPE_" must be replaced.  It should only be done where a type is being
		# specifed.  A double open parantheses "((" marks the location at which it is safe to replace all subsequent "_SCOPE_"
		# occurences on the line.
		while ($_ =~ s/(.*\(\()(.*)_SCOPE_/$1$2::/)
		{
		}

		# Find where "_SCOPE_" is used in a typedef of a structure.  This can be down by looking for
		#	2 tabs - any text - _SCOPE_ - any text followed by a space
		while ($_ =~ s/(\t\t.*)_SCOPE_(.* )/$1::$2/)
		{
		}

		# Find where "_SCOPE_" is used in a parameter list.  This can be down by looking for
		#	1 tabs - any text - _SCOPE_ - any text that does NOT include an open parantheses followed by a space
		while ($_ =~ s/(\t.*)_SCOPE_([^\(]* )/$1::$2/)
		{
		}

		# Find where "_SCOPE_" is used in a cast for a unbounded array.  This can be down by looking for
		#	1 tabs - any text - = ( - any text - _SCOPE_ - any text
		while ($_ =~ s/(\t.* = \(.*)_SCOPE_(.*)/$1::$2/)
		{
		}
	}

	# Write the modified {INPUT_FILE_BASE}Client.cpp back to disk.
	open (MIGFILE, "> $outputSource") or die "Can't open MIG .h for writing $!";
	print MIGFILE @lines;
	close MIGFILE;



	#
	# {INPUT_FILE_BASE}Client.h
	#

	# Open the {INPUT_FILE_BASE}Client.h and slurp all of its contents into the @lines array.
	open (MIGFILE, $outputHeader) or die "Can't open MIG .h for reading $!";
	@lines = <MIGFILE>;
	close MIGFILE;

	# Replace all occurences of "_SCOPE_" with "::"
	foreach (@lines)
	{
		s/_SCOPE_/::/g
	}

	# Write the modified {INPUT_FILE_BASE}Client.h back to disk.
	open (MIGFILE, "> $outputHeader") or die "Can't open MIG .h for writing $!";
	print MIGFILE @lines;
	close MIGFILE;
}
