#include <string>
#include <sstream>
#include <iomanip>
#include <emscripten.h>
#include "emjs.h"
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

void upload(std::string const &accept_types, upload_handler callback, void *callback_data) {
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

void download(std::string const &filename, std::string const &mime_type, std::string_view buffer) {
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

void down(std::string const &filename) {
  /// C++ wrapper for javascript download call, accepting a string_view
  down(filename.c_str());
}

EM_JS(void, get_file, (char const *filename), {
  let ok = false;
  for (let i = 0; i < global_file_list.length; i++) {
    var file = global_file_list[i];
    if (file.name == UTF8ToString(filename)) {
      const file_reader = new FileReader();
      file_reader.addEventListener("loadend",function(){
        const uint8Arr = new Uint8Array(event.target.result);
        const num_bytes = uint8Arr.length * uint8Arr.BYTES_PER_ELEMENT;
        FS.writeFile(event.target.filename, uint8Arr);
        Module.ccall('get_file_done');
      });
      file_reader.filename = file.name;
      file_reader.mime_type = file.type;
      file_reader.readAsArrayBuffer(file);
      ok = true;
    }
  }
  if (!ok) {
    Module.ccall('get_file_done');
  }
});

static volatile bool file_loadend;

void get_file(std::string const &filename) {
  /// C++ wrapper for javascript download call, accepting a string_view
  file_loadend = false;
  get_file(filename.c_str());
  while (!file_loadend) {
    emscripten_sleep(100);
  }
}

EM_JS(void, update_file_list, (char const *json), {
  var tree_data = JSON.parse(UTF8ToString(json));
  createTree(tree_data);
});

void update_file_list(std::string const &json) {
  update_file_list(json.c_str());
}

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

void ui_progbar_update(float value) {
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << value << "%";
  ui_setattr("progress-bar", "style", "width: "+stream.str()+";");
  ui_innerhtml("progress-bar", stream.str().c_str());
  emscripten_sleep(1);
}

void ui_show_error() {
  ui_show_error_int();
}

namespace {

extern "C" {

EMSCRIPTEN_KEEPALIVE int load_file_return(char const *filename, char const *mime_type, char *buffer, size_t buffer_size, upload_handler callback, void *callback_data) {
  /// Load a file - this function is called from javascript when the file upload is activated
  callback(filename, mime_type, {buffer, buffer_size}, callback_data);
  return 1;

}

EMSCRIPTEN_KEEPALIVE void get_file_done() {
  file_loadend = true;
}

extern volatile int ie_state;
EMSCRIPTEN_KEEPALIVE void ui_extract() {
	ie_state=2;
}

}

}

}
