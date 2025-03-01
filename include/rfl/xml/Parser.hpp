#ifndef RFL_XML_PARSER_HPP_
#define RFL_XML_PARSER_HPP_

#include <type_traits>

#include "is_required_for_xml.hpp"
#include "../internal/is_attribute.hpp"
#include "../parsing/NamedTupleParser.hpp"
#include "../parsing/Parser.hpp"
#include "Reader.hpp"
#include "Writer.hpp"
#include "is_required_for_xml.hpp"

namespace rfl {
namespace parsing {

/// XML is very special. It doesn't have proper support for arrays, which means
/// that we just need to ignore empty containers. Therefore, we need to a
/// template specialization for the NamedTuple parser to accommodate for it.
template <class... FieldTypes>
requires AreReaderAndWriter<xml::Reader, xml::Writer, NamedTuple<FieldTypes...>>
struct Parser<xml::Reader, xml::Writer, NamedTuple<FieldTypes...>>
    : public NamedTupleParser<xml::Reader, xml::Writer,
                              /*_ignore_empty_containers=*/true,
                              FieldTypes...> {
};

}  // namespace parsing
}  // namespace rfl

namespace rfl {
namespace xml {

template <class T>
using Parser = parsing::Parser<Reader, Writer, T>;

}
}  // namespace rfl

#endif
