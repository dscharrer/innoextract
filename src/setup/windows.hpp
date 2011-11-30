
#ifndef INNOEXTRACT_SETUP_WINDOWS_HPP
#define INNOEXTRACT_SETUP_WINDOWS_HPP

#include <iosfwd>

namespace setup {

struct version;

struct windows_version {
	
	struct data {
		
		unsigned major;
		unsigned minor;
		unsigned build;
		
		inline bool operator==(const data & o) const {
			return (build == o.build && major == o.major && minor == o.minor);
		}
		
		inline bool operator!=(const data & o) const {
			return !(*this == o);
		}
		
		void load(std::istream & is, const version & version);
		
	};
	
	data win_version;
	data nt_version;
	
	struct service_pack {
		
		unsigned major;
		unsigned minor;
	
		inline bool operator==(const service_pack & o) const {
			return (major == o.major && minor == o.minor);
		}
		
		inline bool operator!=(const service_pack & o) const {
			return !(*this == o);
		}
		
	};
	
	service_pack nt_service_pack;
	
	void load(std::istream & is, const version & version);
	
	inline bool operator==(const windows_version & o) const {
		return (win_version == o.win_version
		        && nt_version == o.nt_version
		        && nt_service_pack == o.nt_service_pack);
	}
	
	inline bool operator!=(const windows_version & o) const {
		return !(*this == o);
	}
	
	static const windows_version none;
	
};

struct windows_version_range {
	
	windows_version begin;
	windows_version end;
	
	void load(std::istream & is, const version & version);
	
};

std::ostream & operator<<(std::ostream & os, const windows_version::data & svd);
std::ostream & operator<<(std::ostream & os, const windows_version & svd);

} // namespace setup

#endif // INNOEXTRACT_SETUP_WINDOWS_HPP
