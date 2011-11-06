
#ifndef INNOEXTRACT_STREAM_LZMAFILTER_HPP
#define INNOEXTRACT_STREAM_LZMAFILTER_HPP

#include <stddef.h>
#include <boost/iostreams/filter/symmetric.hpp>
#include <boost/noncopyable.hpp>
#include <iosfwd>

struct lzma_error : public std::ios_base::failure {
	
	inline lzma_error(std::string msg, int code) : failure(msg), error_code(code) { }
	
	inline int error() const { return error_code; }
	
private:
	
	int error_code;
};

class lzma_decompressor_impl_base : public boost::noncopyable {
	
public:
	
	typedef char char_type;
	
	inline ~lzma_decompressor_impl_base() { close(); }
	
	bool filter(const char * & begin_in, const char * end_in,
	            char * & begin_out, char * end_out, bool flush);
	
	void close();
	
protected:
	
	//! Abstract base class, subclasses need to intialize stream.
	inline lzma_decompressor_impl_base() : stream(NULL) { }
	
	void * stream;
	
};

class inno_lzma1_decompressor_impl : public lzma_decompressor_impl_base {
	
public:
	
	inline inno_lzma1_decompressor_impl() : nread(0) { }
	
	bool filter(const char * & begin_in, const char * end_in,
	            char * & begin_out, char * end_out, bool flush);
	
	inline void close() { lzma_decompressor_impl_base::close(), nread = 0; }
	
private:
	
	size_t nread; //! Number of bytes read into header.
	char header[5];
	
};

class inno_lzma2_decompressor_impl : public lzma_decompressor_impl_base {
	
public:
	
	bool filter(const char * & begin_in, const char * end_in,
	            char * & begin_out, char * end_out, bool flush);
	
};

template <class Impl, class Allocator = std::allocator<typename Impl::char_type> >
class lzma_decompressor : public boost::iostreams::symmetric_filter<Impl, Allocator> {
	
public:
	
	explicit lzma_decompressor(int buffer_size = boost::iostreams::default_device_buffer_size)
		: boost::iostreams::symmetric_filter<Impl, Allocator>(buffer_size) { }
	
};

/*!
 * A filter that decompressess LZMA1 streams found in Inno Setup installers,
 * to be used with boost::iostreams.
 *
 * The LZMA1 streams used by Inno Setup differ slightly from the LZMA Alone file format:
 * The stream header only stores the properties (lc, lp, pb) and the dictionary size and
 * is missing the uncompressed size field. The fiels that are present are encoded identically.
 */
typedef lzma_decompressor<inno_lzma1_decompressor_impl> inno_lzma1_decompressor;

/*!
 * A filter that decompressess LZMA2 streams found in Inno Setup installers,
 * to be used with boost::iostreams.
 *
 * Inno Setup uses raw LZMA2 streams. (preceded only by the dictionary size encoded as one byte)
 */
typedef lzma_decompressor<inno_lzma2_decompressor_impl> inno_lzma2_decompressor;

#endif // INNOEXTRACT_STREAM_LZMAFILTER_HPP
