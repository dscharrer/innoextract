/*
 * Copyright (C) 2018 Daniel Scharrer
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
 * GOG.com-specific extensions.
 */
#ifndef INNOEXTRACT_CLI_GOGGALAXY_HPP
#define INNOEXTRACT_CLI_GOGGALAXY_HPP

namespace setup { struct info; }

namespace gog {

/*!
 * For some GOG installers, some application files are shipped in GOG Galaxy format:
 * Thse files are split into one or more parts and then individually compressed.
 * The parts are decompressed and reassembled by pre-/post-install scripts.
 * This function parses the arguments to those scripts so that we can re-assemble them ourselves.
 *
 * The first part of a multi-part file has a before_install script that configures the output filename
 * as well as the number of parts in the file and a checksum for the whole file.
 *
 * Each part (including the first) has an after_install script with a checksum for the decompressed
 * part as well as compressed and decompressed sizes.
 *
 * Additionally, language constrained are also parsed from check scripts and added to the language list.
 */
void parse_galaxy_files(setup::info & info, bool force);

} // namespace gog

#endif // INNOEXTRACT_CLI_GOGGALAXY_HPP

