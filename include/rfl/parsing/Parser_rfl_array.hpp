#ifndef RFL_PARSING_PARSER_RFL_ARRAY_HPP_
#define RFL_PARSING_PARSER_RFL_ARRAY_HPP_

#include <type_traits>

#include "../Array.hpp"
#include "../Result.hpp"
#include "../always_false.hpp"
#include "../internal/to_std_array.hpp"
#include "Parser_array.hpp"
#include "Parser_base.hpp"

namespace rfl {
namespace parsing {

template <class R, class W, class T>
  requires AreReaderAndWriter<R, W, Array<T>>
struct Parser<R, W, Array<T>> {
 public:
  using InputArrayType = typename R::InputArrayType;
  using InputVarType = typename R::InputVarType;

  using OutputArrayType = typename W::OutputArrayType;
  using OutputVarType = typename W::OutputVarType;

  using RflArray = Array<T>;
  using StdArray = internal::to_std_array_t<T>;

  static Result<RflArray> read(const R& _r, const InputVarType& _var) noexcept {
    return Parser<R, W, StdArray>::read(_r, _var);
  }

  static OutputVarType write(const W& _w, const RflArray& _arr) noexcept {
    return Parser<R, W, StdArray>::write(_w, _arr);
  }
};

}  // namespace parsing
}  // namespace rfl

#endif
