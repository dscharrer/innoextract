#ifndef INNOEXTRACT_WASM_JSFILE_H
#define INNOEXTRACT_WASM_JSFILE_H

#include <boost/filesystem/path.hpp>
#include <istream>

namespace fs = boost::filesystem;

namespace wasm {

class JSFile;

class JSFileBuf : public std::streambuf {
 public:
  JSFileBuf(JSFile& file);

 private:
  JSFile& file_;
  std::streampos pos_;
  virtual std::streamsize xsgetn(char* s, std::streamsize n) override;
  virtual std::streamsize showmanyc() override;
  virtual std::streampos seekpos(std::streampos sp, std::ios_base::openmode mode) override;
  virtual std::streampos seekoff(off_type off, std::ios_base::seekdir way,
                                 std::ios_base::openmode mode) override;
  virtual int_type uflow() override;
};

class JSFile : public std::istream {
 public:
  friend class JSFileBuf;
  JSFile();
  JSFile(const std::string& path, std::ios_base::openmode mode = ios_base::in);
  JSFile(const fs::path& path, std::ios_base::openmode mode = ios_base::in);
  virtual ~JSFile();
  void open(const std::string& path, std::ios_base::openmode mode = ios_base::in);
  void open(const fs::path& path, std::ios_base::openmode mode = ios_base::in);
  bool is_open() const;
  size_t size() const;
  void close();

 private:
  std::string path_;
  int js_index_ = -1;
  size_t size_ = 0;
};

extern "C" {
int file_exist(char const* filename);
int file_size(int file_idx);
int read_bytes(int file_idx, void* ptr, uint64_t pos, uint64_t length);
}
}  // namespace wasm

#endif