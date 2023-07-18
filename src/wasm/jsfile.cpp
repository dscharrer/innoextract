#include "jsfile.hpp"
#include <emscripten.h>

namespace wasm {

EM_JS(int, file_exist, (char const* filename), {
  for (let i = 0; i < global_file_list.length; i++) {
    var file = global_file_list[i];
    if (file.name == UTF8ToString(filename)) {
      return i;
    }
  }
  return -1;
});

EM_JS(int, file_size, (int file_idx), {
  if (file_idx < global_file_list.length) {
    return global_file_list[file_idx].size;
  }
  return -1;
});

EM_ASYNC_JS(int, read_bytes, (int file_idx, void* ptr, uint64_t pos, uint64_t length), {
  if (file_idx < global_file_list.length) {
    let file = global_file_list[file_idx];
    let chunk = file.slice(Number(pos), Number(pos + length));
    const uint8Arr = new Uint8Array(await chunk.arrayBuffer());
    const num_bytes = uint8Arr.length * uint8Arr.BYTES_PER_ELEMENT;

    if (num_bytes > 0) {
      const data_on_heap = new Uint8Array(Module.HEAPU8.buffer, ptr, Number(length));
      data_on_heap.set(uint8Arr);
    }

    return num_bytes;
  }

  return -1;
});

JSFileBuf::JSFileBuf(JSFile& file) : file_(file), pos_(0) {}

std::streamsize JSFileBuf::xsgetn(char* s, std::streamsize n) {
  int read = read_bytes(file_.js_index_, s, static_cast<uint64_t>(pos_), static_cast<uint64_t>(n));

  if (read == -1) {
    return 0;
  }
  pos_ += read;
  return read;
}

JSFileBuf::int_type JSFileBuf::uflow() {
  char buff;
  int read = read_bytes(file_.js_index_, &buff, static_cast<uint64_t>(pos_), 1);
  if (read == 1) {
    pos_ += 1;
    return buff;
  }
  return traits_type::eof();
}

std::streamsize JSFileBuf::showmanyc() { return std::streamsize(file_.size() - pos_); }

std::streampos JSFileBuf::seekpos(std::streampos sp, std::ios_base::openmode mode) {
  pos_ = sp;
  return pos_;
}

std::streampos JSFileBuf::seekoff(off_type off, std::ios_base::seekdir way,
                                  std::ios_base::openmode mode) {
  switch (way) {
    case std::ios_base::beg:
      pos_ = off;
      break;
    case std::ios_base::cur:
      pos_ += off;
      break;
    case std::ios_base::end:
      pos_ = file_.size() - off;
      break;
  }
  return pos_;
}

JSFile::JSFile() : std::istream(new JSFileBuf(*this)) {}

JSFile::JSFile(const std::string& path, std::ios_base::openmode mode)
    : std::istream(new JSFileBuf(*this)) {
  open(path, mode);
}

JSFile::JSFile(const fs::path& path, std::ios_base::openmode mode)
    : std::istream(new JSFileBuf(*this)) {
  open(path, mode);
}

JSFile::~JSFile() { delete rdbuf(); }

void JSFile::open(const std::string& path, std::ios_base::openmode mode) {
  int file_idx = file_exist(path.c_str());
  if (file_idx >= 0) {
    path_ = path;
    js_index_ = file_idx;
    size_ = file_size(js_index_);
    if (mode & std::ios_base::ate) {
      seekg(0, std::ios_base::end);
    } else {
      seekg(0);
    }
  } else {
    setstate(badbit);
  }
}

void JSFile::open(const fs::path& path, std::ios_base::openmode mode) { open(path.string(), mode); }

void JSFile::close() { js_index_ = -1; }

bool JSFile::is_open() const { return js_index_ != -1; }

size_t JSFile::size() const { return size_; }
}  // namespace wasm