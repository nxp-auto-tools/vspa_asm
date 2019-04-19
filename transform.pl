$vcpu3 = "assembler/as-vcpu3_spec.cc";
$ippu3 = "assembler/as-ippu3_spec.cc";

my %index_map;
my %modifier_map;

# Check if a line corresponds to a modifier function's definition.
sub check_modifier_definition
{
	my $line = shift;
	return $line =~ m/^static bfd_uint64_t\ _sym\d+_modifier\ \(const\ expressionS\ \*operands,\ unsigned\ __cia\ ATTRIBUTE_UNUSED\)/;
}

# Process modifier in order to extract information related to the operands
# whose values are used to compute the encoded value.
sub process_modifier
{
	my $line = shift;
	my ($name, $body) = $line =~ /.*(_sym\d+_modifier).* \{(.*)\}/;

	my @indexes = $body =~ /operands\[(\d+)\]\.X_add_number/g;

	if (scalar(@indexes) == 0)
	{
		return;
	}

	my %hash = map { $_, 1 } @indexes;
	my @unique = sort { $a <=> $b } keys %hash;

	my $modifier_signature = join("_", @unique);

	if (not exists($index_map{$modifier_signature}))
	{
		$index_map{$modifier_signature} = \@unique;
	}

	$modifier_map{$name} = $modifier_signature;
}

# Emit mapping between modifiers and indexes of the operands whose values  are
# used to compute the encoded value.
sub emit_modifier_maps
{
	my $core = shift;
	my $i = 0;

	print "\n";

	my %size_map = ();
	for my $index (sort keys %index_map)
	{
		my $list = $index_map{$index};
		my $name = "idx_list_" . $i;
		print "const size_t " . $name . "[] = { " . join(", ", @$list)
			. " };\n";
		$index_map{$index} = $name;
		$size_map{$index} = scalar(@$list);
		$i = $i + 1;
	}

	print "\n";
	print "size_t num_" . $core . "_modifiers = " . scalar(keys %modifier_map)
		. ";\n";
	print "struct modifier_info " . $core . "_modifiers[] = {\n";
	for my $modifier (sort keys %modifier_map)
	{
		my $index = $modifier_map{$modifier};
		print "\t{ " . $modifier . ", " . $size_map{$index} . ", " 
			. $index_map{$index} . " },\n"; 
	}

	print "\t{ NULL, 0, NULL, },\n";
	print "};\n";
}

sub transform_file
{
	my $file = shift;
	my $core = shift;
	my $ippu;

	if ($core eq 'vcpu')
	{
		$ippu = 0;
	}
	else
	{
		$ippu = 1;
	}

	open (FILE, "<$file") or die "Can't open $file for reading!\n";

	my @lines = <FILE>;
	if ($lines[0] =~ /^\/\/\ Pre\-processed/)
	{
		return;
	}

	open(STDOUT, ">$file") or die "Can't open $file for writing!\n";

	# these functions are prefixed
	my @replace_list = (
			"dt_debug",
			"adl_get_instr_ops",
			"adl_get_instr_names",
      "adl_size_lookup",
			"md_begin",
			"md_show_usage",
			"md_parse_option",
			"md_apply_fix",
			"md_assemble",
			"ppc_target_format",
			"ppc_arch",
			"ppc_mach"
		);

	# add the namespace since the "using namespace adl;" is removed to avoid the conflicts related to multiple 
	my %replace_map = (
			"InstrBundle" => "adl::InstrBundle",
			"InstrInfo" => "adl::InstrInfo",
      "le_intbv" => "adl::le_intbv",
      "le_sintbv" => "adl::le_sintbv"
		);

	print "// Pre-processed\n";	

	# fix the MS header error
	if ($ippu)
	{
		print "#define _Enable_shared static _Enable_shared\n\n";
	}

	%index_map = ();
	%modifier_map = ();
	foreach my $line (@lines)
	{
		if (check_modifier_definition($line))
		{
			process_modifier($line);
		}

		# skip some lines that are not needed by the assembler
		if (($ippu 
			and ($line =~ m/std::string\sadl_asm_version/
				or $line =~ m/\s*const\s+char\s+(adl_parallel_separator_chars|adl_symbol_chars|comment_chars|line_comment_chars|line_separator_chars)/
				or $line =~ m/#include\s+"helpers/))
			or $line =~ m/using\s+namespace\s+adl/)
		{
			next;
		}

		# add static attributes on some definitions to avoid conflicts when linking the generated files
		if ($line =~ m/^(enum_field|struct\s+adl_prefix_field|struct\s+adl_operand|struct\s+enum_fields|adl_instr_attrs)/)
		{
			$line = "static " . $line;
		}

		# replace some patterns
		foreach my $replace (@replace_list)
		{
			my $with = $core . "_" . $replace;
			$line =~ s/$replace/$with/g;
		}

		foreach my $replace (keys %replace_map)
		{
			my $with = $replace_map{$replace};
			$line =~ s/$replace/$with/g;
		}

		print $line;
	}

	print "#include \"$core-assembler.inc\"\n";

	emit_modifier_maps($core);

	close STDOUT;
}

transform_file($vcpu3, 'vcpu');
transform_file($ippu3, 'ippu');
