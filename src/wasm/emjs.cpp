#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <emscripten/emscripten.h>
#include "wasm/extract.hpp"
#include "emjs.h"
#include <emscripten/val.h>
// Based on emscripten-browser-file package by Armchair Software, licensed under MIT
// https://github.com/Armchair-Software/emscripten-browser-file

namespace emjs{

using upload_handler = void(*)(std::string const&, std::string const&, std::string_view buffer, void*);

EM_JS(void, upload, (char const *accept_types, upload_handler callback, void *callback_data), {
  /// Prompt the browser to open the file selector dialogue, and pass the file to the given handler
  /// Accept-types are in the format ".png,.jpeg,.jpg" as per https://developer.mozilla.org/en-US/docs/Web/HTML/Attributes/accept
  /// Upload handler callback signature is:
  ///   void my_handler(std::string const filename, std::string const &mime_type, std::string_view buffer, void *callback_data = nullptr);
  globalThis.open_file = function(e) {
    const file_reader = new FileReader();
    file_reader.addEventListener("loadend",function(){
      const uint8Arr = new Uint8Array(event.target.result);
      const num_bytes = uint8Arr.length * uint8Arr.BYTES_PER_ELEMENT;
      const data_ptr = Module._malloc(num_bytes);
      const data_on_heap = new Uint8Array(Module.HEAPU8.buffer, data_ptr, num_bytes);
      data_on_heap.set(uint8Arr);
      var callback_data = callback_data || 0;
      const res = Module.ccall('load_file_return', 'number', ['string', 'string', 'number', 'number', 'number', 'number'], [event.target.filename, event.target.mime_type, data_on_heap.byteOffset, uint8Arr.length, callback, callback_data]);
      Module._free(data_ptr);
    });
    file_reader.filename = e.target.files[0].name;
    file_reader.mime_type = e.target.files[0].type;
    file_reader.readAsArrayBuffer(e.target.files[0]);
  };

  var file_selector = document.createElement('input');
  file_selector.setAttribute('type', 'file');
  file_selector.setAttribute('onchange', 'open_file(event)');
  file_selector.setAttribute('accept', UTF8ToString(accept_types));
  file_selector.click();
});

void upload_wrap(std::string const &accept_types, upload_handler callback, void *callback_data) {
  /// C++ wrapper for javascript upload call
  upload(accept_types.c_str(), callback, callback_data);
}

EM_JS(void, download, (char const *filename, char const *mime_type, void const *buffer, size_t buffer_size), {
  /// Offer a buffer in memory as a file to download, specifying download filename and mime type
  var a = document.createElement('a');
  a.download = UTF8ToString(filename);
  a.href = URL.createObjectURL(new Blob([new Uint8Array(Module.HEAPU8.buffer, buffer, buffer_size)], {type: UTF8ToString(mime_type)}));
  a.click();
});

void download_wrap(std::string const &filename, std::string const &mime_type, std::string_view buffer) {
  /// C++ wrapper for javascript download call, accepting a string_view
  download(filename.c_str(), mime_type.c_str(), buffer.data(), buffer.size());
}

EM_JS(void, down, (char const *filename), {
		/// Offer a buffer in memory as a file to download, specifying download filename and mime type
		var a = document.createElement('a');
		a.download = UTF8ToString(filename);
		a.href = URL.createObjectURL(new Blob([FS.readFile(UTF8ToString(filename)).buffer], {type: "application/octet-stream"}));
		a.click();
});

void down_wrap(std::string const &filename) {
  /// C++ wrapper for javascript download call, accepting a string_view
  down(filename.c_str());
}

EM_JS(void, update_file_list, (char const *json), {
  var tree_data = JSON.parse(UTF8ToString(json));
  createTree(tree_data);
});

EM_JS(void, ui_innerhtml_int, (const char *id, const char *value), {
	var elem = document.getElementById(UTF8ToString(id));
	elem.innerHTML=UTF8ToString(value);
});

EM_JS(void, ui_setattr_int, (const char *id, const char *attr, const char *value), {
	var elem = document.getElementById(UTF8ToString(id));
	elem.setAttribute(UTF8ToString(attr),UTF8ToString(value));
  // console.log("setattr="+UTF8ToString(attr)+", val="+UTF8ToString(value));
});

EM_JS(void, ui_remattr_int, (const char *id, const char *attr), {
	var elem = document.getElementById(UTF8ToString(id));
	elem.removeAttribute(UTF8ToString(attr));
});

EM_JS(void, ui_show_error_int, (), {
  showErrorModal();
});


void ui_innerhtml(const char *id, const char *value) {
  ui_innerhtml_int(id, value);
}

void ui_setattr(const char *id, const char *attr, std::string const &value) {
  ui_setattr_int(id, attr, value.c_str());
}

void ui_remattr(const char *id, const char *attr) {
  ui_remattr_int(id, attr);
}

void ui_progbar_update(int value) {
  static int last_value = 0;
  if (value != last_value) {
    const std::string s = std::to_string(value) + "%";
    ui_setattr("progress-bar", "style", "width: " + s + ";");
    ui_innerhtml("progress-bar", s.c_str());
    last_value = value;
    emscripten_sleep(1);
  }
}

void ui_show_error() {
  ui_show_error_int();
}

EM_JS(void, open_int, (const char *name, const char *modes), {
  var fileStream;
  if(!fileStream){
    fileStream = streamSaver.createWriteStream(UTF8ToString(name));
    Module.writer = fileStream.getWriter();
  }
  Module.writer.ready.then(() => {
  console.log("zipstream: open, ready");
  });
});

void open(const char *name, const char *modes){
  open_int(name, modes);
}

EM_ASYNC_JS(size_t, write_int, (const void *ptr, size_t size, size_t n), {
  let buff = new Uint8Array(Module.HEAPU8.buffer, ptr, size*n);

  await Module.writer.write(buff); //.then(() => {
  console.log("zipstream: write "+(size*n));
});

size_t write(const void *ptr, size_t size, size_t n){
  static char buff[64*1024*1024];
  static int bpos = 0;
  if((bpos+size*n > sizeof(buff)) || (ptr == NULL)) {
    std::cout << "writing " << bpos << "bytes\n";
    write_int(buff, 1, bpos);
    bpos = 0;
  }

  memcpy(buff+bpos, ptr, size*n);
  bpos += size*n;

  return size*n;
}

EM_JS(void, close_int, (void), {
  Module.writer.close();
  console.log("zipstream: close")
});

void close(){
  close_int();
}

namespace {

extern "C" {
ssize_t emjs_write(void* buf, size_t len)
{
  return emjs::write(buf, 1, len);
}

EMSCRIPTEN_KEEPALIVE int load_file_return(char const *filename, char const *mime_type, char *buffer, size_t buffer_size, upload_handler callback, void *callback_data) {
  /// Load a file - this function is called from javascript when the file upload is activated
  callback(filename, mime_type, {buffer, buffer_size}, callback_data);
  return 1;
}

EMSCRIPTEN_KEEPALIVE char const * load_exe(char const *filename) {
  static std::string result;
  result = wasm::extractor::get().load_exe(filename);
  return result.c_str();
}

EMSCRIPTEN_KEEPALIVE char const * list_files() {
  static std::string result;
  result = wasm::extractor::get().list_files();
  return result.c_str();
}

EMSCRIPTEN_KEEPALIVE char const * extract(char const *list_json) {
  static std::string result;
  result = wasm::extractor::get().extract(list_json);
  return result.c_str();
}

}

}

}
