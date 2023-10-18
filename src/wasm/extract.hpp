#ifndef INNOEXTRACT_WASM_H
#define INNOEXTRACT_WASM_H

#include <boost/filesystem.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <mutex>
#include <nlohmann/json.hpp>
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

  bool is_multipart() const {
    return !entry_->additional_locations.empty();
  }
  const std::string& path() const {
    return path_;
  }
  const setup::file_entry& entry() const {
    return *entry_;
  }

private:
  const setup::file_entry* entry_;
  std::string path_;
};

class file_output : private boost::noncopyable {
public:
  file_output(const fs::path& dir, const processed_file* f, bool write, ZIPstream* zip);

  bool write(char* data, size_t n);
  void seek(boost::uint64_t new_position);
  void close();
  const fs::path& path() const {
    return path_;
  }
  const processed_file* file() const {
    return file_;
  }
  bool is_complete() const;
  bool has_checksum() const;
  bool calculate_checksum();
  crypto::checksum checksum();
  void settime(time_t t);

private:
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
};

enum class LanguageFilterOptions {
  SelectedLanguageAndAgnostic,
  All,
  SelectedLanguage,
  LanguageAgnostic
};

enum class CollisionResolutionOptions { Overwrite, Rename, RenameAll, Error };

struct ExtractionSettings {
  bool debug_messages_enabled;
  bool exclude_temporary_files_enabled;
  bool logs_to_file;
  LanguageFilterOptions language_filter_options;
  CollisionResolutionOptions collision_resolution_options;
};

class extractor {
public:
  static extractor& get();

  void set_options(const std::string& options_json);
  bool options_differ(const std::string& options_json) const;

  std::string load_exe(const std::string& exe_path);
  std::string list_files();
  std::string extract(const std::string& list_json);
  void set_abort(bool state);

private:
  using json = nlohmann::ordered_json;
  using multi_part_outputs = boost::ptr_map<const processed_file*, file_output>;
  using output_location = std::pair<const processed_file*, uint64_t>;

  using files_t = std::map<stream::file, size_t>;
  using chunks_t = std::map<stream::chunk, files_t>;

  static void init_singleton();

  static const char* error_obj(const std::string& msg);

  static extractor* singleton_instance;
  static std::once_flag init_instance_flag;
  bool aborted;

  extractor();
  extractor(const extractor&) = delete;
  extractor& operator=(const extractor&) = delete;

  void open_installer_stream();
  void load_installer_data();
  std::string dump_installer_info() const;

  void clear_files_list();
  void fetch_files();
  json create_main_dir_obj(std::map<std::string, json::object_t*>& dir_objs) const;
  std::string dump_dirs_info(json& main_dir_obj,
                             std::map<std::string, json::object_t*>& dir_objs) const;
  std::vector<processed_file>
  resolve_collisions(const std::vector<const processed_file*>& selected_files,
                     const std::string& default_language);

  void print_extraction_options() const;
  std::vector<const processed_file*>
  fetch_selected_files(const nlohmann::ordered_json& input) const;
  void clean_memfs(const std::string& output_dir) const;
  void create_empty_dirs(const nlohmann::ordered_json& input) const;
  std::vector<std::vector<output_location>>
  get_files_for_location(const std::vector<processed_file>& resolved_files,
                         const std::string& lang);
  chunks_t
  create_chunks(const std::vector<std::vector<extractor::output_location>>& files_for_location);
  void open_zip_stream(const std::string& output_dir);

  std::unique_ptr<stream::slice_reader> create_slice_reader();
  std::vector<file_output*> open_outputs(const std::vector<output_location>& output_locations,
                                         const std::string& output_dir);
  uint64_t seek_input_stream(stream::chunk_reader::type* chunk_source, const stream::file& file,
                             uint64_t current_offset);
  uint64_t copy_data(const stream::file_reader::pointer& source,
                     const std::vector<file_output*>& outputs);

  uint64_t get_size() const;
  void verify_close_outputs(const std::vector<file_output*>& outputs,
                            const setup::data_entry& data);
  void save_zip();
  void abort_zip();

  fs::path installer_path_{};
  util::ifstream installer_ifs_{};
  loader::offsets installer_offsets_{};
  setup::info installer_info_{};

  std::set<std::string> dirs_{};
  std::vector<processed_file> all_files_{};

  uint64_t bytes_extracted_{};
  uint64_t total_size_{};

  multi_part_outputs multi_outputs_{};

  ZIPstream* output_zip_stream_{};

  ExtractionSettings extraction_settings_{};
};

} // namespace wasm

#endif // INNOEXTRACT_WASM_H