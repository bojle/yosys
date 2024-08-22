/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2012 Vicharak Computers PVT LTD <shreeyash.pandey@vicharak.in>
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

#include "kernel/celltypes.h"
#include "kernel/log.h"
#include "kernel/register.h"
#include "kernel/rtlil.h"
#include "kernel/sigtools.h"
#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

/*

Sections in a vdb file

  Header
  Module Info (IMAO)
  Module port info
  Primary Inputs
  Primary Outputs
  Module instantiation (parameters + portref)
  Footer
 *
*/

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

using uchar = unsigned char;
using u16 = uint16_t;

const std::vector<uchar> vdb_header{0x03, 0x1f, 0x6c, 0x12};

const std::vector<uchar> vdb_version{0xa0, 0x00, 0x00, 0x0b};

const std::vector<uchar> vdb_encryp_algo{0x08, 0x03, 0x1f, 0x7a, 0xe4, 0x40};

const std::vector<uchar> vdb_timestamp{0x00, 0x00, 0x00, 0x00,
                                       0x66, 0xc4, 0x3d, 0x7a};

const std::vector<uchar> idk{0x01, 0x00, 0x20, 0x20, 0x00, 0x02};

const std::vector<uchar> preamble_header{0x40, 0x31, 0x00, 0xab, 0x00};

/* encode/decode scheme */
std::map<uchar, uchar> gbl_map{
    {'[', 0x3c}, {']', 0x35}, {'_', 0x40}, {'o', 0x60}, {'[', 0xbd},
    {']', 0xbc}, {'d', 0x4a}, {'a', 0x41}, {'c', 0x48}, {'b', 0x43},
    {'0', 0x3e}, {'I', 0x95}, {'1', 0x3d}, {'2', 0x3f}, {'3', 0x44},
    {'4', 0xaa}, {'9', 0xa5}, {'8', 0xa6}, {'7', 0xa4}, {'6', 0xab},
    {'5', 0xa9}, {'T', 0xba}, {'R', 0xb3}, {'U', 0xb9}, {'E', 0x99},
    {'C', 0x98}, {'X', 0xb6}, {'A', 0x91}, {'B', 0x93}, {'S', 0xb8},
    {'P', 0xb2}, {'M', 0x9d}, {'Y', 0xb5}, {'N', 0x9f}, {'L', 0x9e},
    {'D', 0x9a}, {'Q', 0xb1}, {'J', 0x97}, {'K', 0x9c}, {'G', 0x94},
    {'H', 0x96}, {'z', 0x67}, {'y', 0x65}, {'x', 0x66}, {'w', 0x64},
    {'v', 0x6b}, {'u', 0x69}, {'t', 0x6a}, {'s', 0x68}, {'r', 0x63},
    {'q', 0x61}, {'p', 0x62}, {'n', 0x4f}, {'m', 0x4d}, {'l', 0x4e},
    {'k', 0x4c}, {'j', 0x47}, {'i', 0x45}, {'h', 0x46}, {'g', 0x44},
    {'f', 0x4b}, {'e', 0x49}, {'.', 0x8f}, {'/', 0xa0}, {'Z', 0xb7},
    {'W', 0xb4}, {'V', 0xbb}, {'F', 0x9b},

    // -- shplit  --

    {'D', 0x13}, {'A', 0x10}, {'_', 0x3d}, {'X', 0x3b}, {'F', 0x1a},
    {'E', 0x18}, {'H', 0x1b}, {'T', 0x33}, {'I', 0x14}, {'W', 0x39},
    {'R', 0x32}, {'B', 0x12}, {'U', 0x38}, {'N', 0x1e}, {'a', 0x80},
    {'1', 0x20}, {'b', 0x8d}, {'l', 0x87}, {'0', 0x0f}, {'m', 0x8c},
    {'2', 0x22}, {'3', 0x21}, {'4', 0x23}, {'5', 0x28}, {'6', 0x29},
    {'O', 0x1d}, {'P', 0x1f}, {'S', 0x31}, {'M', 0x1c}, {'Y', 0x34},
    {'C', 0x11}, {'L', 0x17}, {'G', 0x19}, {'b', 0x82}, {'V', 0x3a},
    {'J', 0x16}, {'Z', 0x36}, {'d', 0x83}, {'K', 0x15}, {'Q', 0x30},
    {'c', 0x81}, {'e', 0x88}, {'g', 0x89}, {'i', 0x84}, {'k', 0x85},
    {'v', 0xaa}, {'n', 0x8e}, {'6', 0x2a}, {'~', 0xae}, {'f', 0x8a},
    {'$', 0x03},
};


using SSMap = std::map<std::string, std::string>;

std::map<std::string, SSMap> prim_info_map {
  {"EFX_LUT4", SSMap{{"cellname", "EFX_LUT4"}}}
};

uchar maplu(uchar key) {
  auto itr = gbl_map.find(key);
  if (itr != gbl_map.end()) {
    return itr->second;
  }
  log_error("fix this bruh, go look for an encoding for key: %c\n", key);
  return 0;
}

std::vector<uchar> map_encode(std::string s) {
  std::string ss = RTLIL::unescape_id(s);
  std::vector<uchar> ret;
  ret.push_back(ss.size());
  for (auto itr = ss.rbegin(); itr != ss.rend(); ++itr) {
    ret.push_back(maplu(*itr));
  }
  return ret;
}

std::vector<uchar> map_encode(RTLIL::IdString id_s) {
  std::string ss = RTLIL::unescape_id(id_s);
  return map_encode(ss);
}

template <typename T> void revld(std::ostream &f, const std::vector<T> &v) {
  for (auto itr = v.rbegin(); itr != v.rend(); ++itr) {
    f << *itr;
  }
}


std::vector<uchar> zeros(int x) {
  assert(x >= 0);
  std::vector<uchar> ret(x, 0);
  return ret;
}

template <typename T> void ld(std::ostream &f, const std::vector<T> &v) {
  for (auto itr = v.begin(); itr != v.end(); ++itr) {
    f << *itr;
  }
}

void ld(std::ostream &f, uchar v) {
  f << v;
}

void revld(std::ostream &f, u16 v) {
  uchar byte0 = v & 0x00ff;
  uchar byte1 = (v & 0xff00) >> 8;
  log("byte 0 %02x\n", byte0);
  log("byte 1 %02x\n", byte1);
  ld(f, byte0);
  ld(f, byte1);
}


struct VdbBackend : public Backend {
  VdbBackend() : Backend("vdb", "write design to vdb netlist file") {}
  void help() override {
    //   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
    log("\n");
    log("    write_vdb [options] [filename]\n");
    log("\n");
  }

  void emit_header(std::ostream *&f, std::string top_module_name) {
    revld(*f, vdb_header);
    revld(*f, vdb_version);
    ld(*f, zeros(4));
    revld(*f, vdb_encryp_algo);
    revld(*f, vdb_timestamp);
    ld(*f, idk);
    ld(*f, preamble_header);
    ld(*f, map_encode(std::string("efxphysicallib")));
    ld(*f, map_encode(top_module_name));
    ld(*f, map_encode(top_module_name));
  }

  void emit_module_info(std::ostream *&f, RTLIL::Design *design) {
    /* load all black box modules */
    for (auto module : design->modules()) {
      for (auto cc : module->cells()) {
        for (auto itr : cc->attributes) {
          log("cell %s, attr %s = %d\n", log_id(cc->name), log_id(itr.first),
              itr.second.as_int());
        }
        log("encoding module %s of type %s\n", log_id(cc->name),
            log_id(cc->type));
        if (cc->type != IdString("$scopeinfo")) {
          ld(*f, map_encode(cc->type));
          ld(*f, zeros(10));
          log("type %s \n", log_id(cc->type));
        }
      }
    }
    for (auto module : design->modules()) {
      for (auto cc : module->cells()) {
        for (auto itr : cc->attributes) {
          log("cell %s, attr %s = %d\n", log_id(cc->name), log_id(itr.first),
              itr.second.as_int());
        }
        log("encoding module %s of type %s\n", log_id(cc->name),
            log_id(cc->type));
        if (cc->type == IdString("$scopeinfo")) {
          ld(*f, map_encode(cc->name));
          ld(*f, zeros(10));
          log("type %s \n", log_id(cc->type));
        }
      }
    }
  }

  void emit_module_port_info(std::ostream *&f, RTLIL::Design *design,
                             IdString top_module_name) {
    ld(*f, preamble_header);
    ld(*f, 0x00);
    revld(*f, vdb_timestamp);
    ld(*f, (uchar)0xff);
    ld(*f, zeros(8));
    ld(*f, map_encode(top_module_name));
    ld(*f, zeros(10));

    ld(*f, preamble_header);
    ld(*f, 0x00);
    revld(*f, vdb_timestamp);
    ld(*f, (uchar)0xff);

    for (auto module : design->modules()) {
      log("mod -> %s\n", log_id(module->name));
      for (auto cc : module->cells()) {
        log("\tcc -> %s\n", log_id(cc->type));
        uchar sz = cc->connections().size();
        ld(*f, sz);
        ld(*f, 0x00);
        for (auto itr : cc->connections()) {
          log("\t\tconnection ports %s type %d\n", log_id(itr.first), cc->input(itr.first));
          ld(*f, map_encode(itr.first));
          ld(*f, zeros(10));
          std::vector<uchar> ze(10, 0);
          if (cc->input(itr.first)) {
            ze[0] = 0x01;
            ld(*f, ze);
          } else if (cc->output(itr.first)) {
            ze[0] = 0x02;
            ld(*f, ze);
          } else {
            log_error("cant handle cell %s being neither input nor output\n", log_id(itr.first));
          }
        }
      }
    }

    RTLIL::Module *mod = design->module(top_module_name);
    uchar sz = mod->wires().size();
    ld(*f, sz);
    ld(*f, 0x00);
    for (auto ww : mod->wires()) {
      log("\twires ports %s type %d\n", log_id(ww->name), ww->port_input);
      ld(*f, map_encode(ww->name));
      ld(*f, zeros(12));
      std::vector<uchar> ze(10, 0);
      if (ww->port_input) {
        ze[0] = 0x01;
        ld(*f, ze);
      } else if (ww->port_output) {
        ze[0] = 0x02;
        ld(*f, ze);
      } else {
        log_error("cant handle cell %s being neither input nor output\n", log_id(top_module_name));
      }

    }
  }

  void emit_primary_inputs(std::ostream *&f, RTLIL::Design *design, IdString top_module_name) {
    RTLIL::Module *mod = design->module(top_module_name);
    for (auto name : mod->ports) {
      if (mod->wire(name)->port_input) {
        log("primary input port %s\n", log_id(name));
      } else if (mod->wire(name)->port_output) {
        log("primary output port %s\n", log_id(name));
      } else {
        log_error("cant handle cell %s being neither input nor output\n", log_id(name));
      }
    }
  }

  void init_wire_map(std::map<RTLIL::IdString, u16> &wmap, RTLIL::Design *design) {
    u16 cnt = 0x00;
    for (auto module : design->modules()) {
      for (auto ww : module->wires()) {
        log("init_wire_map %s %d\n", log_id(ww->name), cnt);
        wmap.insert({ww->name, cnt});
        cnt++;
      }
    }
  }

  void execute(std::ostream *&f, std::string filename,
               std::vector<std::string> args, RTLIL::Design *design) override {
    std::string top_module_name;

    size_t argidx = 1;
    extra_args(f, filename, args, argidx);

    if (top_module_name.empty()) {
      for (auto module : design->modules()) {
        if (module->get_bool_attribute(ID::top)) {
          top_module_name = module->name.str();
        }
      }
    }

    std::map<RTLIL::IdString, u16> wire_map;
    init_wire_map(wire_map, design);

    emit_header(f, top_module_name);
    ld(*f, std::vector<uchar>{0x02, 0x00});
    emit_module_info(f, design);
    emit_module_port_info(f, design, IdString(top_module_name));
    emit_primary_inputs(f, design, IdString(top_module_name));
  }
} VdbBackend;

PRIVATE_NAMESPACE_END
