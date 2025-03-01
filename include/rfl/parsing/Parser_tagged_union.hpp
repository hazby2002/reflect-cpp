#ifndef RFL_PARSING_PARSER_TAGGED_UNION_HPP_
#define RFL_PARSING_PARSER_TAGGED_UNION_HPP_

#include <type_traits>

#include "../Result.hpp"
#include "../TaggedUnion.hpp"
#include "../always_false.hpp"
#include "Parser_base.hpp"
#include "TaggedUnionWrapper.hpp"

namespace rfl {
namespace parsing {

template <class R, class W, internal::StringLiteral _discriminator,
          class... AlternativeTypes>
  requires AreReaderAndWriter<R, W,
                              TaggedUnion<_discriminator, AlternativeTypes...>>
struct Parser<R, W, TaggedUnion<_discriminator, AlternativeTypes...>> {
  using ResultType = Result<TaggedUnion<_discriminator, AlternativeTypes...>>;

 public:
  using InputObjectType = typename R::InputObjectType;
  using InputVarType = typename R::InputVarType;

  using OutputObjectType = typename W::OutputObjectType;
  using OutputVarType = typename W::OutputVarType;

  static ResultType read(const R& _r, const InputVarType& _var) noexcept {
    const auto get_disc = [&_r](auto _obj) {
      return get_discriminator(_r, _obj);
    };

    const auto to_result = [&_r, _var](const std::string& _disc_value) {
      return find_matching_alternative(_r, _disc_value, _var);
    };

    return _r.to_object(_var).and_then(get_disc).and_then(to_result);
  }

  static OutputVarType write(
      const W& _w, const TaggedUnion<_discriminator, AlternativeTypes...>&
                       _tagged_union) noexcept {
    const auto handle = [&](const auto& _val) -> OutputVarType {
      return write_wrapped(_w, _val);
    };
    return std::visit(handle, _tagged_union.variant_);
  }

 private:
  template <int _i = 0>
  static ResultType find_matching_alternative(
      const R& _r, const std::string& _disc_value,
      const InputVarType& _var) noexcept {
    if constexpr (_i == sizeof...(AlternativeTypes)) {
      return Error("Could not parse tagged union, could not match " +
                   _discriminator.str() + " '" + _disc_value + "'.");
    } else {
      using AlternativeType = std::remove_cvref_t<
          std::variant_alternative_t<_i, std::variant<AlternativeTypes...>>>;

      if (contains_disc_value<AlternativeType>(_disc_value)) {
        const auto to_tagged_union = [](auto&& _val) {
          return TaggedUnion<_discriminator, AlternativeTypes...>(
              std::move(_val));
        };

        const auto embellish_error = [&](Error&& _e) {
          return Error(
              "Could not parse tagged union with "
              "discrimininator " +
              _discriminator.str() + " '" + _disc_value + "': " + _e.what());
        };

        return Parser<R, W, AlternativeType>::read(_r, _var)
            .transform(to_tagged_union)
            .or_else(embellish_error);

      } else {
        return find_matching_alternative<_i + 1>(_r, _disc_value, _var);
      }
    }
  }

  /// Retrieves the discriminator.
  static Result<std::string> get_discriminator(
      const R& _r, const InputObjectType& _obj) noexcept {
    const auto embellish_error = [](const auto& _err) {
      return Error("Could not parse tagged union: Could not find field '" +
                   _discriminator.str() +
                   "' or type of field was not a string.");
    };

    const auto to_type = [&_r](auto _var) {
      return _r.template to_basic_type<std::string>(_var);
    };

    return _r.get_field(_discriminator.str(), _obj)
        .and_then(to_type)
        .or_else(embellish_error);
  }

  /// Determines whether the discriminating literal contains the value
  /// retrieved from the object.
  template <class T>
  static inline bool contains_disc_value(
      const std::string& _disc_value) noexcept {
    return internal::tag_t<T>::contains(_disc_value);
  }

  /// Writes a wrapped version of the original object, which contains the tag.
  template <class T>
  static OutputVarType write_wrapped(const W& _w, const T& _val) noexcept {
    const auto tag = internal::make_tag<T>();
    using TagType = std::remove_cvref_t<decltype(tag)>;
    if constexpr (internal::has_fields<std::remove_cvref_t<T>>()) {
      using WrapperType =
          TaggedUnionWrapperWithFields<T, TagType, _discriminator>;
      const auto wrapper = WrapperType{.tag = tag, .fields = &_val};
      return Parser<R, W, WrapperType>::write(_w, wrapper);
    } else {
      using WrapperType =
          TaggedUnionWrapperNoFields<T, TagType, _discriminator>;
      const auto wrapper = WrapperType{.tag = tag, .fields = &_val};
      return Parser<R, W, WrapperType>::write(_w, wrapper);
    }
  }
};

}  // namespace parsing
}  // namespace rfl

#endif
