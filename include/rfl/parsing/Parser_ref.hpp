#ifndef RFL_PARSING_PARSER_REF_HPP_
#define RFL_PARSING_PARSER_REF_HPP_

#include <type_traits>

#include "../Ref.hpp"
#include "../Result.hpp"
#include "../always_false.hpp"
#include "Parser_base.hpp"

namespace rfl {
namespace parsing {

template <class R, class W, class T>
  requires AreReaderAndWriter<R, W, Ref<T>>
struct Parser<R, W, Ref<T>> {
  using InputVarType = typename R::InputVarType;
  using OutputVarType = typename W::OutputVarType;

  /// Expresses the variables as type T.
  static Result<Ref<T>> read(const R& _r, const InputVarType& _var) noexcept {
    const auto to_ref = [&](auto&& _t) { return Ref<T>::make(std::move(_t)); };
    return Parser<R, W, std::remove_cvref_t<T>>::read(_r, _var).transform(
        to_ref);
  }

  /// Expresses the variable a a JSON.
  static OutputVarType write(const W& _w, const Ref<T>& _ref) noexcept {
    return Parser<R, W, std::remove_cvref_t<T>>::write(_w, *_ref);
  }
};

}  // namespace parsing
}  // namespace rfl

#endif
