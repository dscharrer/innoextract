#ifndef INNOEXTRACT_WASM_H
#define INNOEXTRACT_WASM_H

#include <boost/filesystem.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <set>
#include <string>
#include <vector>

#include "crypto/checksum.hpp"
#include "crypto/hasher.hpp"
#include "loader/offsets.hpp"
#include "setup/file.hpp"
#include "setup/info.hpp"
#include "stream/file.hpp"
#include "util/fstream.hpp"

#include "wasm/fdzipstream/fdzipstream.h"

namespace fs = boost::filesystem;

namespace wasm {

class processed_file {
 public:
  processed_file(const setup::file_entry* entry, const std::string& path)
      : entry_(entry), path_(path) {}

  bool is_multipart() const { return !entry_->additional_locations.empty(); }
  const std::string& path() const { return path_; }
  const setup::file_entry& entry() const { return *entry_; }

 private:
  const setup::file_entry* entry_;
  std::string path_;
};

class file_output : private boost::noncopyable {
  fs::path path_;
  const processed_file* file_;
  util::fstream stream_;

  crypto::hasher checksum_;
  boost::uint64_t checksum_position_;

  boost::uint64_t position_;
  boost::uint64_t total_written_;

  bool write_;
  ZIPstream* zip_ = nullptr;
	bool file_open_;
	ZIPentry* zip_entry_;

 public:
  explicit file_output(const fs::path& dir, const processed_file* f, bool write, ZIPstream* zip);
  bool write(char* data, size_t n);
  void seek(boost::uint64_t new_position);
  void close();
  const fs::path& path() const { return path_; }
  const processed_file* file() const { return file_; }
  bool is_complete() const;
  bool has_checksum() const;
  bool calculate_checksum();
  crypto::checksum checksum();
  void settime(time_t t);
};

class Context {
 public:
  static Context& get();
  std::string LoadExe(std::string exe_file);
  std::string ListFiles();
  std::string Extract(std::string list_json);

 private:
  Context();
  fs::path installer_;
  util::ifstream ifs_;
  loader::offsets offsets_;
  setup::info info_;
  std::set<std::string> dirs_;
  std::vector<processed_file> all_files_;
  uint64_t bytes_extracted_;
  uint64_t total_size_;
  typedef boost::ptr_map<const processed_file*, file_output> multi_part_outputs;
  multi_part_outputs multi_outputs_;
  ZIPstream* zip_ = nullptr;
  void add_dirs(std::set<std::string>& vec, const std::string& path) const;
  uint64_t get_size() const;
  uint64_t copy_data(const stream::file_reader::pointer& source,
                     const std::vector<file_output*>& outputs);
  void verify_close_outputs(const std::vector<file_output*>& outputs,
                            const setup::data_entry& data);
  void save_zip();
  static const char* error_obj(const std::string& msg);
};

}  // namespace wasm

#endif  // INNOEXTRACT_WASM_H