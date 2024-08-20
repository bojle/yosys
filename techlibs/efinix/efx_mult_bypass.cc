/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2024  Shreeyash Pandey <shreeyash.pandey@vicharak.in>
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

#include "kernel/sigtools.h"
#include "kernel/yosys.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct EfxMultBypass : public Pass {
  EfxMultBypass()
      : Pass("efx_mult_bypass",
             "skip generating a hard multiplier for multiplications"
             " with a efx_mult_bypass set") {}
  void help() override {
    log("\n");
    log("    efx_mult_bypass \n");
    log("\n");
    log("skip generating a hard multiplier for multiplications"
        " with a efx_mult_bypass set\n");
    log("\n");
  }
  void execute(std::vector<std::string> args, RTLIL::Design *design) override {
    const IdString bypass_attr_name = IdString("\\efx_mult_bypass");
    log_header(design, "Executing EFX_MULT_BYPASS pass \n");

    for (RTLIL::Module *mm : design->modules()) {
      for (RTLIL::Cell *cc : mm->cells()) {
        if (cc->type == "$mul") {
          if (cc->has_attribute(bypass_attr_name) &&
              cc->get_bool_attribute(bypass_attr_name)) {
            log("bypassing EFX_MULT generation for cell %s", log_id(cc->name));
            // keep $mul as the type
          } else {
            cc->type = IdString("$__efx_mult");
          }
        }
      }
    }
  }
} EfxMultBypass;

PRIVATE_NAMESPACE_END
