
#ifndef INNOEXTRACT_CLI_DEBUG_HPP
#define INNOEXTRACT_CLI_DEBUG_HPP

#include <iosfwd>

namespace loader { struct offsets; }
namespace setup { struct info; }

void print_offsets(const loader::offsets & offsets);
void print_info(const setup::info & info);

#endif // INNOEXTRACT_CLI_DEBUG_HPP
