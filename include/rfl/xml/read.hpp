#ifndef RFL_XML_READ_HPP_
#define RFL_XML_READ_HPP_

#include <istream>
#include <pugixml.hpp>
#include <string>

#include "../internal/get_type_name.hpp"
#include "../internal/remove_namespaces.hpp"
#include "../wrap_in_rfl_array_t.hpp"
#include "Parser.hpp"
#include "Reader.hpp"

namespace rfl {
namespace xml {

using InputVarType = typename Reader::InputVarType;

/// Parses an object from a XML var.
template <class T>
Result<wrap_in_rfl_array_t<T>> read(const InputVarType& _var) {
  const auto r = Reader();
  return Parser<T>::read(r, _var);
}

/// Parses an object from XML using reflection.
template <class T>
Result<wrap_in_rfl_array_t<T>> read(const std::string& _xml_str) {
  pugi::xml_document doc;
  const auto result = doc.load_string(_xml_str.c_str());
  if (!result) {
    return Error("XML string could not be parsed: " +
                 std::string(result.description()));
  }
  const auto var = InputVarType(doc.first_child());
  return read<T>(var);
}

/// Parses an object from a stringstream.
template <class T>
Result<wrap_in_rfl_array_t<T>> read(std::istream& _stream) {
  const auto xml_str = std::string(std::istreambuf_iterator<char>(_stream),
                                   std::istreambuf_iterator<char>());
  return read<T>(xml_str);
}

}  // namespace xml
}  // namespace rfl

#endif
