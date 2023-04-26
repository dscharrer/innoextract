#include "extract.hpp"

// #include <zip.h>

#include <iostream>
#include <nlohmann/json.hpp>

#include "cli/goggalaxy.hpp"
#include "emjs.h"
#include "setup/data.hpp"
#include "setup/directory.hpp"
#include "setup/filename.hpp"
#include "setup/language.hpp"
#include "stream/slice.hpp"
#include "util/load.hpp"
#include "util/log.hpp"
#include "util/time.hpp"


using json = nlohmann::ordered_json;

namespace wasm {

file_output::file_output(const fs::path& dir, const processed_file* f, bool write, Nonzip &zip)
    : path_(dir / f->path()),
      file_(f),
      checksum_(f->entry().checksum.type),
      checksum_position_(f->entry().checksum.type == crypto::None ? boost::uint64_t(-1) : 0),
      position_(0),
      total_written_(0),
      write_(write),
      zip_(zip),
      zip_open_(false) {
  if (write_) {
    try {
      std::ios_base::openmode flags =
          std::ios_base::out | std::ios_base::binary | std::ios_base::trunc;
      if (file_->is_multipart()) {
        flags |= std::ios_base::in;
      }
      // stream_.open(path_, flags);
      // if (!stream_.is_open()) {
      if(zip_.getStatus() != NONZIP_STATUS_READY) {
        throw std::exception();
      }
    } catch (...) {
      throw std::runtime_error("Could not open output file \"" + path_.string() + '"');
    }
  }
}

bool file_output::write(const char* data, size_t n) {
  // if (write_) {
  //   stream_.write(data, std::streamsize(n));
  // }

  int r;
  if(!zip_open_) {
		r = zip_.addFile(path_.c_str(), data, n, &zipindex_);
		zip_open_ = true;
	} else {
    r = zip_.appendFile(data, n);
  }


  if (checksum_position_ == position_) {
    checksum_.update(data, n);
    checksum_position_ += n;
  }

  position_ += n;
  total_written_ += n;

  // return !write_ || !stream_.fail();
  return r==n;
}

void file_output::seek(boost::uint64_t new_position) {
  if (new_position == position_) {
    return;
  }

  // if (!write_) {
  //   position_ = new_position;
  //   return;
  // }

  // const boost::uint64_t max =
  //     boost::uint64_t(std::numeric_limits<util::fstream::off_type>::max() / 4);

  // if (new_position <= max) {
  //   stream_.seekp(util::fstream::off_type(new_position), std::ios_base::beg);
  // } else {
  //   util::fstream::off_type sign = (new_position > position_) ? 1 : -1;
  //   boost::uint64_t diff =
  //       (new_position > position_) ? new_position - position_ : position_ - new_position;
  //   while (diff > 0) {
  //     stream_.seekp(sign * util::fstream::off_type(std::min(diff, max)), std::ios_base::cur);
  //     diff -= std::min(diff, max);
  //   }
  // }

  // position_ = new_position;
  printf("seek() stub! pos=%llu, newpos=%llu\n", position_, new_position);
}

void file_output::close() {
  // if (write_) {
  //   stream_.close();
  // }
  return;
}


void file_output::settime(time_t t){
  if(zip_open_)
    zip_.setTime(zipindex_, t);
}

bool file_output::is_complete() const { return total_written_ == file_->entry().size; }

bool file_output::has_checksum() const { return checksum_position_ == file_->entry().size; }

bool file_output::calculate_checksum() {
  if (has_checksum()) {
    return true;
  }

  if (!write_) {
    return false;
  }

  const boost::uint64_t max =
      boost::uint64_t(std::numeric_limits<util::fstream::off_type>::max() / 4);

  boost::uint64_t diff = checksum_position_;
  stream_.seekg(util::fstream::off_type(std::min(diff, max)), std::ios_base::beg);
  diff -= std::min(diff, max);
  while (diff > 0) {
    stream_.seekg(util::fstream::off_type(std::min(diff, max)), std::ios_base::cur);
    diff -= std::min(diff, max);
  }

  while (!stream_.eof()) {
    char buffer[8192];
    std::streamsize n = stream_.read(buffer, sizeof(buffer)).gcount();
    checksum_.update(buffer, size_t(n));
    checksum_position_ += boost::uint64_t(n);
  }

  if (!has_checksum()) {
    return false;
  }

  return true;
}

crypto::checksum file_output::checksum() { return checksum_.finalize(); }

Context::Context() : zip_("innoout.zip") { color::init(color::disable, color::disable); }

Context& Context::get() {
  static Context ctx;
  return ctx;
}

std::string Context::LoadExe(std::string exe_file) {
  installer_ = exe_file;
  json obj;
  emjs::ui_progbar_update(0);
  try {
    if (ifs_.is_open()) {
      ifs_.close();
    }
    log_info << "Opening \"" << exe_file << '"';
    ifs_.open(exe_file, std::ios_base::in | std::ios_base::binary);
    if (!ifs_.is_open()) {
      throw std::exception();
    }
  } catch (...) {
    return error_obj("Could not open file \"" + exe_file + '"');
  }
  offsets_.load(ifs_);
  setup::info::entry_types entries = 0;
  entries |= setup::info::Files;
  entries |= setup::info::Directories;
  entries |= setup::info::DataEntries;
  entries |= setup::info::Languages;
  ifs_.seekg(offsets_.header_offset);
  try {
    info_.load(ifs_, entries, 0);
  } catch (const setup::version_error&) {
    if (offsets_.found_magic) {
      if (offsets_.header_offset == 0) {
        return error_obj("Could not determine location of setup headers!");
      } else {
        return error_obj("Could not determine setup data version!");
      }
    }
    return error_obj("Not a supported Inno Setup installer!");
  } catch (const std::exception& e) {
    return error_obj(e.what());
  }

  gog::parse_galaxy_files(info_, 0);

  obj["name"] = info_.header.app_versioned_name.empty() ? info_.header.app_name
                                                        : info_.header.app_versioned_name;
  obj["copyrights"] = info_.header.app_copyright;
  obj["langs"] = json::array();
  obj["size"] = get_size() / 1024 / 1024;
  obj["files_num"] = info_.files.size();

  for (const setup::language_entry& language : info_.languages) {
    json lang_obj;
    lang_obj["name"] = language.name;
    lang_obj["lang_name"] = language.language_name;
    obj["langs"].push_back(lang_obj);
  }
  return obj.dump();
}

uint64_t Context::get_size() const {
  uint64_t max_size = 0;
  for (size_t i = 0; i < info_.data_entries.size(); i++) {
    const setup::data_entry& location = info_.data_entries[i];
    max_size += location.uncompressed_size;
  }
  return max_size;
}

std::string Context::ListFiles() {
  setup::filename_map filenames;
  dirs_.clear();
  all_files_.clear();
  all_files_.reserve(info_.files.size());
  json main_obj;
  json main_dir;
  main_dir["text"] = info_.header.app_name;
  main_dir["mainDir"] = true;
  main_dir["nodes"] = json::array();
  std::map<std::string, json::object_t*> json_dirs;
  filenames.set_expand(true);

  for (const setup::directory_entry& directory : info_.directories) {
    std::string path = filenames.convert(directory.name);
    if (path.empty()) continue;
    dirs_.insert(path);
    add_dirs(dirs_, path);
  }

  for (const setup::file_entry& file : info_.files) {
    if (file.location >= info_.data_entries.size()) {
      continue;  // Ignore external files (copy commands)
    }
    std::string path = filenames.convert(file.destination);
    if (path.empty()) continue;
    add_dirs(dirs_, path);
    all_files_.push_back(processed_file(&file, path));
  }

  // create JSON objects for directories
  for (const auto& p : dirs_) {
    size_t pos = p.find_last_of(setup::path_sep);
    if (pos == std::string::npos) {
      json_dirs[p] = main_dir["nodes"].emplace_back(json{{"text", p}}).get_ptr<json::object_t*>();
    } else {
      json::object_t* parent = json_dirs[p.substr(0, pos)];
      if (!parent->count("nodes")) {
        parent->emplace("nodes", json::array());
      }
      json_dirs[p] = parent->at("nodes")
                         .emplace_back(json{{"text", p.substr(pos + 1)}})
                         .get_ptr<json::object_t*>();
    }
  }

  uint32_t idx = 0;
  for (const auto& f : all_files_) {
    const std::string& path = f.path();
    json file_obj;
    size_t pos = path.find_last_of(setup::path_sep);

    file_obj["text"] = path.substr(pos + 1);
    file_obj["icon"] = "bi bi-file-earmark-fill";
    file_obj["fileId"] = idx++;
    file_obj["tags"] = json::array();
    file_obj["tags"].push_back(f.entry().languages);

    if (pos != std::string::npos) {
      json::object_t* parent = json_dirs[path.substr(0, pos)];
      if (!parent->count("nodes")) {
        parent->emplace("nodes", json::array());
      }
      parent->at("nodes").push_back(file_obj);
    } else {
      main_dir["nodes"].push_back(file_obj);
    }
  }
  main_obj.emplace_back(main_dir);
  return main_obj.dump();
}

std::string Context::Extract(std::string list_json) {
  const std::string& output_dir = info_.header.app_name;
  auto input = json::parse(list_json);
  std::vector<const processed_file*> selected_files;
  selected_files.reserve(all_files_.size());

  std::sort(input.begin(), input.end());
  log_info << "Unpacking " << input.size() << " files have been started.";
  for (const auto& i : input) {
    selected_files.push_back(&all_files_[i]);
  }

  // cleaning MEMFS
  if (fs::exists(output_dir)) {
    fs::remove_all(output_dir);
  }

  // creating empty directories - ignoring user input
  // writing directly to the ZIP will resolve that in the future
  fs::create_directory(output_dir);

  for (const auto& dir : dirs_) {
    fs::create_directory(output_dir + "/" + dir);
  }

  typedef std::pair<const processed_file*, uint64_t> output_location;
  std::vector<std::vector<output_location> > files_for_location;
  files_for_location.resize(info_.data_entries.size());

  for (const processed_file* file_ptr : selected_files) {
    files_for_location[file_ptr->entry().location].push_back(output_location(file_ptr, 0));
    uint64_t offset = info_.data_entries[file_ptr->entry().location].uncompressed_size;
    uint32_t sort_slice = info_.data_entries[file_ptr->entry().location].chunk.first_slice;
    uint32_t sort_offset = info_.data_entries[file_ptr->entry().location].chunk.sort_offset;
    for (uint32_t location : file_ptr->entry().additional_locations) {
      setup::data_entry& data = info_.data_entries[location];
      files_for_location[location].push_back(output_location(file_ptr, offset));
      offset += data.uncompressed_size;
      if (data.chunk.first_slice > sort_slice ||
          (data.chunk.first_slice == sort_slice && data.chunk.sort_offset > sort_offset)) {
        sort_slice = data.chunk.first_slice;
        sort_offset = data.chunk.sort_offset;
      } else if (data.chunk.first_slice == sort_slice &&
                 data.chunk.sort_offset == data.chunk.offset) {
        data.chunk.sort_offset = ++sort_offset;
      } else {
        // Could not reorder chunk - no point in trying to reordder the
        // remaining chunks
        sort_slice = -1;
      }
    }
  }

  total_size_ = 0;
  uint64_t files = 0;

  typedef std::map<stream::file, size_t> Files;
  typedef std::map<stream::chunk, Files> Chunks;
  Chunks chunks;
  for (size_t i = 0; i < info_.data_entries.size(); i++) {
    if (!files_for_location[i].empty()) {
      setup::data_entry& location = info_.data_entries[i];
      chunks[location.chunk][location.file] = i;
      total_size_ += location.uncompressed_size;
    }
  }

  log_info << "Total size: " << total_size_ << " bytes";

  try {
    std::unique_ptr<stream::slice_reader> slice_reader;
    if (offsets_.data_offset) {
      slice_reader.reset(new stream::slice_reader(&ifs_, offsets_.data_offset));
    } else {
      fs::path dir = installer_.parent_path();
      std::string basename = installer_.stem().string();
      std::string basename2 = info_.header.base_filename;
      // Prevent access to unexpected files
      std::replace(basename2.begin(), basename2.end(), '/', '_');
      std::replace(basename2.begin(), basename2.end(), '\\', '_');
      // Older Inno Setup versions used the basename stored in the headers,
      // change our default accordingly
      if (info_.version < INNO_VERSION(4, 1, 7) && !basename2.empty()) {
        std::swap(basename2, basename);
      }
      slice_reader.reset(
          new stream::slice_reader(dir, basename, basename2, info_.header.slices_per_disk));
    }

    bytes_extracted_ = 0;
    multi_outputs_.clear();

    for (const Chunks::value_type& chunk : chunks) {  //[first = chunk, second = [file, location]]
      stream::chunk_reader::pointer chunk_source;
      if (chunk.first.encryption == stream::Plaintext) {
        chunk_source = stream::chunk_reader::get(*slice_reader, chunk.first, "");
      }
      uint64_t offset = 0;
      for (const Files::value_type& location : chunk.second) {  // 1 chunk => n files
        const stream::file& file = location.first;
        const std::vector<output_location>& output_locations = files_for_location[location.second];
        if (file.offset > offset) {
          if (chunk_source.get()) {
            util::discard(*chunk_source, file.offset - offset);
          }
        }
        if (chunk_source.get() && file.offset < offset) {
          std::ostringstream oss;
          oss << "Bad offset while extracting files: file start (" << file.offset
              << ") is before end of previous file (" << offset << ")!";
          throw std::runtime_error(oss.str());
        }
        offset = file.offset + file.size;

        if (!chunk_source.get()) {
          continue;  // Not extracting/testing this file
        }

        crypto::checksum checksum;

        // Open input file
        stream::file_reader::pointer file_source;
        file_source = stream::file_reader::get(*chunk_source, file, &checksum);

        std::vector<file_output*> outputs;
        for (const output_location& output_loc : output_locations) {  // 1 file => n output files
          const processed_file* fileinfo = output_loc.first;

          // Re-use existing file output for multi-part files
          file_output* output = NULL;
          if (fileinfo->is_multipart()) {
            multi_part_outputs::iterator it = multi_outputs_.find(fileinfo);
            if (it != multi_outputs_.end()) {
              output = it->second;
            }
          }

          if (!output) {
            output = new file_output(output_dir, fileinfo, true, zip_);
            if (fileinfo->is_multipart()) {
              multi_outputs_.insert(fileinfo, output);
            }
          }

          outputs.push_back(output);
          output->seek(output_loc.second);
        }

        uint64_t output_size = copy_data(file_source, outputs);
        const setup::data_entry& data = info_.data_entries[location.second];

        if (output_size != data.uncompressed_size) {
          log_warning << "Unexpected output file size: " << output_size
                      << " != " << data.uncompressed_size;
        }

        verify_close_outputs(outputs, data);

        if (checksum != file.checksum) {
          log_warning << "Checksum mismatch:\n"
                      << " ├─ actual:   " << checksum << '\n'
                      << " └─ expected: " << file.checksum;
        }
      }
    }
  } catch (const std::exception& e) {
    return error_obj(e.what());
  }

  if (!multi_outputs_.empty()) {
    log_warning << "Incomplete multi-part files";
  }

  log_info << "Done. Creating ZIP file.";
  save_zip();

  return json::object().dump();
}

uint64_t Context::copy_data(const stream::file_reader::pointer& source,
                            const std::vector<file_output*>& outputs) {
  uint64_t output_size = 0;
  while (!source->eof()) {
    char buffer[8192 * 10];
    std::streamsize buffer_size = std::streamsize(boost::size(buffer));
    std::streamsize n = source->read(buffer, buffer_size).gcount();
    if (n > 0) {
      for (file_output* output : outputs) {
        bool success = output->write(buffer, size_t(n));
        if (!success) {
          throw std::runtime_error("Error writing file \"" + output->path().string() + '"');
        }
      }
      bytes_extracted_ += n;
      output_size += n;

      emjs::ui_progbar_update(float(bytes_extracted_) / total_size_ * 100);
    }
  }

  return output_size;
}

void Context::verify_close_outputs(const std::vector<file_output*>& outputs,
                                   const setup::data_entry& data) {
  for (file_output* output : outputs) {
    if (output->file()->is_multipart() && !output->is_complete()) {
      continue;
    }
    if (output->file()->entry().checksum.type != crypto::None && output->calculate_checksum()) {
      crypto::checksum output_checksum = output->checksum();
      if (output_checksum != output->file()->entry().checksum) {
        log_warning << "Output checksum mismatch for " << output->file()->path() << ":\n"
                    << " ├─ actual:   " << output_checksum << '\n'
                    << " └─ expected: " << output->file()->entry().checksum;
      }
    }
    output->close();
    output->settime(data.timestamp);
    log_info << " - File " << output->path() << " unpacked.";
    // if (!util::set_file_time(output->path(), data.timestamp, data.timestamp_nsec)) {
    //   log_warning << "Error setting timestamp on file " << output->path();
    // }

    if (output->file()->is_multipart()) {
      multi_outputs_.erase(output->file());
    }
  }
}

void Context::save_zip() {
  // const std::string& output_dir = info_.header.app_name;
  // std::string zname = output_dir + ".zip";

  // // remove ZIP file if exists
  // if (fs::exists(zname)) {
  //   fs::remove(zname);
  // }

  // int ze = 0;
  // zip_error_t zerr;
  // zip_t* zip = zip_open(zname.c_str(), ZIP_CREATE, &ze);
  // if (ze) printf("ZIP err: %d: %s\n", ze, zip_strerror(zip));
  // zip_int64_t fi;
  // zip_dir_add(zip, output_dir.c_str(), 0);
  // for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(output_dir)) {
  //   std::string path = dir_entry.path().string();
  //   if (fs::is_directory(dir_entry)) {
  //     zip_dir_add(zip, path.c_str(), 0);
  //   } else {
  //     zip_source_t* zf = zip_source_file_create(path.c_str(), 0, 0, &zerr);
  //     fi = zip_file_add(zip, path.c_str(), zf, 0);
  //     zip_set_file_compression(zip, fi, ZIP_CM_STORE, 0);
  //   }
  // }
  // zip_close(zip);
  // emjs::down_wrap(zname);
  zip_.close();
}

void Context::add_dirs(std::set<std::string>& vec, const std::string& path) const {
  size_t pos = path.find_last_of(setup::path_sep);
  if (pos == std::string::npos) {
    return;
  }
  std::string dir = path.substr(0, pos);
  vec.insert(dir);
  add_dirs(vec, dir);
}

const char* Context::error_obj(const std::string& msg) {
  static std::string result;
  static json error_obj;
  error_obj["error"] = msg;
  log_error << msg;
  result = error_obj.dump();
  return result.c_str();
}

}  // namespace wasm