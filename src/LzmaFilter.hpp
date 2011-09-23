
#include <boost/iostreams/filter/symmetric.hpp>

class lzma_error {
	
public:
	
	int code;
	
	inline lzma_error(int _code) : code(_code) { }
	
};

class inno_lzma_decompressor_impl {
	
public:
	
	typedef char char_type;
	
	inno_lzma_decompressor_impl();
	
	~inno_lzma_decompressor_impl();
	
	bool filter(const char * & begin_in, const char * end_in,
	            char * & begin_out, char * end_out, bool flush);
	
	void close();
	
private:
	
	void operator=(inno_lzma_decompressor_impl);
	
	size_t nread;
	char buf[5];
	void * stream;
	bool eof;
	
};

template<typename Alloc = std::allocator<char> >
struct basic_inno_lzma_decompressor
	: public boost::iostreams::symmetric_filter<inno_lzma_decompressor_impl, Alloc> {
	
private:
	
	typedef inno_lzma_decompressor_impl impl_type;
	typedef boost::iostreams::symmetric_filter<impl_type, Alloc> base_type;
	
public:
	
	typedef typename base_type::char_type char_type;
	typedef typename base_type::category category;
	
	basic_inno_lzma_decompressor(int buffer_size = boost::iostreams::default_device_buffer_size);
	
};

typedef basic_inno_lzma_decompressor<> inno_lzma_decompressor;


// Implementation

template<typename Alloc>
basic_inno_lzma_decompressor<Alloc>::basic_inno_lzma_decompressor(int buffer_size) 
	: base_type(buffer_size) { }
