#ifndef RFL_PARSING_PARSER_BOX_HPP_
#define RFL_PARSING_PARSER_BOX_HPP_

#include <type_traits>

#include "../Box.hpp"
#include "../Result.hpp"
#include "Parser_base.hpp"

namespace rfl {
namespace parsing {

template <class R, class W, class T>
  requires AreReaderAndWriter<R, W, Box<T>>
struct Parser<R, W, Box<T>> {
  using InputVarType = typename R::InputVarType;
  using OutputVarType = typename W::OutputVarType;

  static Result<Box<T>> read(const R& _r, const InputVarType& _var) noexcept {
    const auto to_box = [](auto&& _t) { return Box<T>::make(std::move(_t)); };
    return Parser<R, W, std::remove_cvref_t<T>>::read(_r, _var).transform(
        to_box);
  }

  static OutputVarType write(const W& _w, const Box<T>& _box) noexcept {
    return Parser<R, W, std::remove_cvref_t<T>>::write(_w, *_box);
  }
};

}  // namespace parsing
}  // namespace rfl

#endif
