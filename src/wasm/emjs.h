#ifndef EMSCRIPTEN_UPLOAD_FILE_H_INCLUDED
#define EMSCRIPTEN_UPLOAD_FILE_H_INCLUDED

// Based on emscripten-browser-file package by Armchair Software, licensed under MIT
// https://github.com/Armchair-Software/emscripten-browser-file

#include <string>
#include <emscripten.h>

namespace emjs {

using upload_handler = void(*)(std::string const&, std::string const&, std::string_view buffer, void*);

void upload_wrap(std::string const &accept_types, upload_handler callback, void *callback_data = nullptr);
void download_wrap(std::string const &filename, std::string const &mime_type, std::string_view buffer);
void down_wrap(std::string const &filename);
void ui_innerhtml(const char *id, const char *value);
void ui_setattr(const char *id, const char *attr, std::string const &value);
void ui_remattr(const char *id, const char *attr);
void ui_progbar_update(float value);
void ui_show_error();

namespace {
extern "C" {
EMSCRIPTEN_KEEPALIVE int load_file_return(char const *filename, char const *mime_type, char *buffer, size_t buffer_size, upload_handler callback, void *callback_data);
EMSCRIPTEN_KEEPALIVE char const * load_exe(char const *filename);
EMSCRIPTEN_KEEPALIVE char const * list_files();
EMSCRIPTEN_KEEPALIVE char const * extract(char const *list_json);
}
}

}


#endif // EMSCRIPTEN_UPLOAD_FILE_H_INCLUDED
