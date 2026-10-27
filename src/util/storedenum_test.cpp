// SPDX-License-Identifier: Zlib
// Regression tests for stored flag-set padding. See LICENSE for distribution terms.
#include "util/test.hpp"
#include "util/storedenum.hpp"
#include <sstream>

struct padding_flags {
 FLAGS(options, First, Second);
};
FLAGS_OVERLOADS(padding_flags::options)
namespace {

INNOEXTRACT_TEST(stored_flag_padding,
 std::string modern(8, '\0');
 modern[0] = 3;
 modern += char(0x7f);
 std::istringstream input(modern);
 stored_flag_reader<padding_flags::options> reader(input);
 reader.add(padding_flags::First);
 reader.add(padding_flags::Second);
 padding_flags::options value = reader.finalize(8);
 test("modern flag bits", value.has(padding_flags::First) && value.has(padding_flags::Second));
 test("modern eight-byte boundary", input.get() == 0x7f);

 std::istringstream legacy(std::string(4, '\0') + char(0x7f));
 stored_flag_reader<padding_flags::options> old_reader(legacy);
 for(size_t i = 0; i < 24; ++i) { old_reader.add(padding_flags::First); }
 old_reader.finalize();
 test("legacy three-byte set padding", legacy.get() == 0x7f);

 std::istringstream win16(std::string(3, '\0') + char(0x7f));
 stored_flag_reader<padding_flags::options> short_reader(win16, 16);
 for(size_t i = 0; i < 24; ++i) { short_reader.add(padding_flags::First); }
 short_reader.finalize();
 test("16-bit set remains unpadded", win16.get() == 0x7f);

 bool rejected = false;
 try {
  std::istringstream truncated(std::string(4, '\0'));
  truncated.exceptions(std::ios_base::badbit | std::ios_base::failbit);
  stored_flag_reader<padding_flags::options> bad_reader(truncated);
  bad_reader.add(padding_flags::First);
  bad_reader.finalize(8);
 } catch(const std::ios_base::failure &) { rejected = true; }
 test("truncated modern padding rejected", rejected);
)
}
