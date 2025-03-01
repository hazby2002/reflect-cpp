#ifndef RFL_PARSING_PARSER_RENAME_HPP_
#define RFL_PARSING_PARSER_RENAME_HPP_

#include <type_traits>

#include "../Rename.hpp"
#include "../Result.hpp"
#include "../always_false.hpp"
#include "../internal/StringLiteral.hpp"
#include "Parser_base.hpp"

namespace rfl {
namespace parsing {

template <class R, class W, class T, internal::StringLiteral _name>
  requires AreReaderAndWriter<R, W, Rename<_name, T>>
struct Parser<R, W, Rename<_name, T>> {
  using InputVarType = typename R::InputVarType;
  using OutputVarType = typename W::OutputVarType;

  static Result<Rename<_name, T>> read(const R& _r,
                                       const InputVarType& _var) noexcept {
    const auto to_rename = [](auto&& _t) {
      return Rename<_name, T>(std::move(_t));
    };
    return Parser<R, W, std::remove_cvref_t<T>>::read(_r, _var).transform(
        to_rename);
  }

  static OutputVarType write(const W& _w,
                             const Rename<_name, T>& _rename) noexcept {
    return Parser<R, W, std::remove_cvref_t<T>>::write(_w, _rename.value());
  }
};

}  // namespace parsing
}  // namespace rfl

#endif
