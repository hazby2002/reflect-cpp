#ifndef RFL_JSON_WRITE_HPP_
#define RFL_JSON_WRITE_HPP_

#include <yyjson.h>

#include <ostream>
#include <sstream>
#include <string>

#include "Parser.hpp"

namespace rfl {
namespace json {

/// Returns a JSON string.
template <class T>
std::string write(const T& _obj) {
  auto w = Writer(yyjson_mut_doc_new(NULL));
  const auto json_obj = Parser<T>::write(w, _obj);
  yyjson_mut_doc_set_root(w.doc_, json_obj.val_);
  const char* json_c_str = yyjson_mut_write(w.doc_, 0, NULL);
  const auto json_str = std::string(json_c_str);
  free((void*)json_c_str);
  yyjson_mut_doc_free(w.doc_);
  return json_str;
}

/// Writes a JSON into an ostream.
template <class T>
std::ostream& write(const T& _obj, std::ostream& _stream) {
  auto w = Writer(yyjson_mut_doc_new(NULL));
  const auto json_obj = Parser<T>::write(w, _obj);
  yyjson_mut_doc_set_root(w.doc_, json_obj.val_);
  const char* json_c_str = yyjson_mut_write(w.doc_, 0, NULL);
  _stream << json_c_str;
  free((void*)json_c_str);
  yyjson_mut_doc_free(w.doc_);
  return _stream;
}

}  // namespace json
}  // namespace rfl

#endif  // JSON_PARSER_HPP_
