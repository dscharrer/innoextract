
#include "stream/file.hpp"

#include <boost/make_shared.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/restrict.hpp>

#include "setup/data.hpp"
#include "setup/version.hpp"
#include "stream/checksum.hpp"
#include "stream/exefilter.hpp"

namespace io = boost::iostreams;

namespace stream {

file_reader::pointer file_reader::get(base_type & base, const setup::data_entry & location,
                                      const inno_version & version, crypto::checksum * checksum) {
	
	boost::shared_ptr<io::filtering_istream> result = boost::make_shared<io::filtering_istream>();
	
	result->push(stream::checksum_filter(checksum, location.checksum.type), 8192);
	
	if(location.options & setup::data_entry::CallInstructionOptimized) {
		if(version < INNO_VERSION(5, 2, 0)) {
			result->push(inno_exe_decoder_4108(), 8192);
		} else {
			result->push(inno_exe_decoder_5200(version >= INNO_VERSION(5, 3, 9)), 8192);
		}
	}
	
	result->push(io::restrict(base, 0, int64_t(location.file_size)));
	
	return result;
}

} // namespace stream
