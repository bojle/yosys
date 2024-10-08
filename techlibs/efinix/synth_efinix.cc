/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2019  Miodrag Milanovic <micko@yosyshq.com>
 *  Copyright (C) 2019  Claire Xenia Wolf <claire@yosyshq.com>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "kernel/register.h"
#include "kernel/celltypes.h"
#include "kernel/rtlil.h"
#include "kernel/log.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct SynthEfinixPass : public ScriptPass
{
	SynthEfinixPass() : ScriptPass("synth_efinix", "synthesis for Efinix FPGAs") { }

	void help() override
	{
		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    synth_efinix [options]\n");
		log("\n");
		log("This command runs synthesis for Efinix FPGAs.\n");
		log("\n");
		log("    -top <module>\n");
		log("        use the specified module as top module\n");
		log("\n");
		log("    -edif <file>\n");
		log("        write the design to the specified EDIF file. writing of an output file\n");
		log("        is omitted if this parameter is not specified.\n");
		log("\n");
		log("    -json <file>\n");
		log("        write the design to the specified JSON file. writing of an output file\n");
		log("        is omitted if this parameter is not specified.\n");
		log("\n");
		log("    -run <from_label>:<to_label>\n");
		log("        only run the commands between the labels (see below). an empty\n");
		log("        from label is synonymous to 'begin', and empty to label is\n");
		log("        synonymous to the end of the command list.\n");
		log("\n");
		log("    -noflatten\n");
		log("        do not flatten design before synthesis\n");
		log("\n");
		log("    -retime\n");
		log("        run 'abc' with '-dff -D 1' options\n");
		log("\n");
		log("    -nobram\n");
		log("        do not use EFX_RAM_5K cells in output netlist\n");
		log("\n");
		log("\n");
		log("The following commands are executed by this synthesis command:\n");
		help_script();
		log("\n");
	}

	string top_opt, edif_file, json_file, blif_file, sverilog_file;
	bool flatten, retime, nobram;

	void clear_flags() override
	{
		top_opt = "-auto-top";
		edif_file = "";
		json_file = "";
    blif_file = "";
    sverilog_file = "";
		flatten = true;
		retime = false;
		nobram = false;
	}

	void execute(std::vector<std::string> args, RTLIL::Design *design) override
	{
		string run_from, run_to;
		clear_flags();

		size_t argidx;
		for (argidx = 1; argidx < args.size(); argidx++)
		{
			if (args[argidx] == "-top" && argidx+1 < args.size()) {
				top_opt = "-top " + args[++argidx];
				continue;
			}
			if (args[argidx] == "-edif" && argidx+1 < args.size()) {
				edif_file = args[++argidx];
				continue;
			}
			if (args[argidx] == "-json" && argidx+1 < args.size()) {
				json_file = args[++argidx];
				continue;
			}
			if (args[argidx] == "-blif" && argidx+1 < args.size()) {
				blif_file = args[++argidx];
				continue;
			}
			if (args[argidx] == "-sverilog" && argidx+1 < args.size()) {
				sverilog_file = args[++argidx];
				continue;
			}
			if (args[argidx] == "-run" && argidx+1 < args.size()) {
				size_t pos = args[argidx+1].find(':');
				if (pos == std::string::npos)
					break;
				run_from = args[++argidx].substr(0, pos);
				run_to = args[argidx].substr(pos+1);
				continue;
			}
			if (args[argidx] == "-noflatten") {
				flatten = false;
				continue;
			}
			if (args[argidx] == "-retime") {
				retime = true;
				continue;
			}
			if (args[argidx] == "-nobram") {
				nobram = true;
				continue;
			}
			break;
		}
		extra_args(args, argidx, design);

		if (!design->full_selection())
			log_cmd_error("This command only operates on fully selected designs!\n");

		log_header(design, "Executing SYNTH_EFINIX pass.\n");
		log_push();

		run_script(design, run_from, run_to);

		log_pop();
	}

	void script() override
	{
		if (check_label("begin"))
		{
			run("read_verilog -lib +/efinix/cells_sim.v");
			run(stringf("hierarchy -check %s", help_mode ? "-top <top>" : top_opt.c_str()));
		}

		if (flatten && check_label("flatten", "(unless -noflatten)"))
		{
      run("efx_mult_bypass");
			run("proc");
			run("flatten");
			run("tribuf -logic");
			run("deminout");
			run("techmap -map +/efinix/mul_map.v");
		}

		if (check_label("coarse"))
		{
			run("synth -run coarse");
		}

		if (check_label("map_ram"))
		{
			std::string args = "";
			if (help_mode)
				args += " [-no-auto-block]";
			else {
				if (nobram)
					args += " -no-auto-block";
			}
			run("memory_libmap -lib +/efinix/brams.txt" + args, "(-no-auto-block if -nobram)");
			run("techmap -map +/efinix/brams_map.v");
		}

		if (check_label("map_ffram"))
		{
			run("opt -fast -mux_undef -undriven -fine");
			run("memory_map");
			run("opt -undriven -fine");
		}

		if (check_label("map_gates"))
		{
			run("techmap -map +/techmap.v -map +/efinix/arith_map.v");
			run("opt -fast");
			if (retime || help_mode)
				run("abc -dff -D 1", "(only if -retime)");
		}

		if (check_label("map_ffs"))
		{
			run("dfflegalize -cell $_DFFE_????_ 0 -cell $_SDFFE_????_ 0 -cell $_SDFFCE_????_ 0 -cell $_DLATCH_?_ x");
			run("techmap -D NO_LUT -map +/efinix/cells_map.v");
			run("opt_expr -mux_undef");
			run("simplemap");
		}

		if (check_label("map_luts"))
		{
			run("abc -lut 4");
			run("clean");
		}

		if (check_label("map_cells"))
		{
			run("techmap -map +/efinix/cells_map.v");
			run("clean");
		}

		if (check_label("map_gbuf"))
		{
			run("clkbufmap -buf $__EFX_GBUF O:I");
			run("techmap -map +/efinix/gbuf_map.v");
			run("efinix_fixcarry");
			run("clean");
		}

		if (check_label("check"))
		{
			run("hierarchy -check");
			run("stat");
			run("check -noinit");
			run("blackbox =A:whitebox");
		}

		if (check_label("edif"))
		{
			if (!edif_file.empty() || help_mode)
				run(stringf("write_edif %s", help_mode ? "<file-name>" : edif_file.c_str()));
		}

		if (check_label("json"))
		{
			if (!json_file.empty() || help_mode)
				run(stringf("write_json %s", help_mode ? "<file-name>" : json_file.c_str()));
		}

		if (check_label("blif"))
		{
			if (!blif_file.empty() || help_mode)
				run(stringf("write_blif %s", help_mode ? "<file-name>" : blif_file.c_str()));
		}

		if (check_label("sverilog"))
		{
			if (!sverilog_file.empty() || help_mode)
				run(stringf("write_verilog %s", help_mode ? "<file-name>" : sverilog_file.c_str()));
		}
	}
} SynthEfinixPass;

PRIVATE_NAMESPACE_END
