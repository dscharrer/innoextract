/*
 * Copyright (C) 2011-2020 Daniel Scharrer
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*!
 * \file
 *
 * Debug output functions.
 */
#ifndef INNOEXTRACT_CLI_DEBUG_HPP
#define INNOEXTRACT_CLI_DEBUG_HPP

#include <iosfwd>

#include "configure.hpp"

#include "cli/extract.hpp"

#ifdef DEBUG

namespace loader { struct offsets; }
namespace setup { struct info; }

void print_offsets(const loader::offsets & offsets);
void print_info(const setup::info & info);

void dump_headers(std::istream & is, const loader::offsets & offsets, const extract_options & o);

#endif // DEBUG

#endif // INNOEXTRACT_CLI_DEBUG_HPP
