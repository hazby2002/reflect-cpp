#ifndef RFL_MOVE_FROM_TUPLE_HPP_
#define RFL_MOVE_FROM_TUPLE_HPP_

#include <functional>
#include <type_traits>

#include "../Array.hpp"
#include "is_flatten_field.hpp"
#include "is_named_tuple.hpp"
#include "ptr_tuple_t.hpp"
#include "tup_to_ptr_tuple.hpp"
#include "tuple_t.hpp"

namespace rfl {
namespace internal {

template <class Tuple, int _i = 0>
constexpr int calc_flattened_size() {
  if constexpr (_i == std::tuple_size_v<Tuple>) {
    return 0;
  } else {
    using T = std::remove_pointer_t<std::tuple_element_t<_i, Tuple>>;
    if constexpr (is_flatten_field_v<T>) {
      return calc_flattened_size<ptr_tuple_t<typename T::Type>>() +
             calc_flattened_size<Tuple, _i + 1>();
    } else {
      return 1 + calc_flattened_size<Tuple, _i + 1>();
    }
  }
}

template <class TargetTupleType, class PtrTupleType, int _j = 0, class... Args>
auto unflatten_ptr_tuple(PtrTupleType& _t, Args... _args) {
  constexpr auto i = sizeof...(Args);

  constexpr auto size = std::tuple_size_v<std::remove_cvref_t<TargetTupleType>>;

  if constexpr (i == size) {
    return std::make_tuple(_args...);
  } else {
    using T = std::remove_cvref_t<
        std::remove_pointer_t<std::tuple_element_t<i, TargetTupleType>>>;

    if constexpr (is_flatten_field_v<T>) {
      using SubTargetTupleType =
          ptr_tuple_t<std::remove_pointer_t<typename T::Type>>;

      constexpr int flattened_size = calc_flattened_size<SubTargetTupleType>();

      return unflatten_ptr_tuple<TargetTupleType, PtrTupleType,
                                 _j + flattened_size>(
          _t, _args...,
          unflatten_ptr_tuple<SubTargetTupleType, PtrTupleType, _j>(_t));

    } else {
      return unflatten_ptr_tuple<TargetTupleType, PtrTupleType, _j + 1>(
          _t, _args..., std::get<_j>(_t));
    }
  }
}

template <class T, class Pointers, class... Args>
auto move_from_pointers(Pointers& _ptrs, Args&&... _args) {
  constexpr auto i = sizeof...(Args);
  if constexpr (i == std::tuple_size_v<std::remove_cvref_t<Pointers>>) {
    return T{std::move(_args)...};
  } else {
    using FieldType = std::tuple_element_t<i, std::remove_cvref_t<Pointers>>;

    if constexpr (std::is_pointer_v<FieldType>) {
      return move_from_pointers<T>(_ptrs, std::move(_args)...,
                                   std::move(*std::get<i>(_ptrs)));

    } else {
      using PtrTupleType = ptr_tuple_t<std::remove_cvref_t<T>>;

      using U = std::remove_cvref_t<typename std::remove_pointer_t<
          typename std::tuple_element_t<i, PtrTupleType>>::Type>;

      return move_from_pointers<T>(_ptrs, std::move(_args)...,
                                   move_from_pointers<U>(std::get<i>(_ptrs)));
    }
  }
}

template <class T, std::size_t n>
auto make_tuple_from_element(T (*arr)[n]) {
  if constexpr (std::is_array_v<T>) {
    return [&]<std::size_t... i>(std::index_sequence<i...>) {
      return std::tuple_cat(make_tuple_from_element(&(*arr)[i])...);
    }(std::make_index_sequence<n>());
  } else {
    return [&]<std::size_t... i>(std::index_sequence<i...>) {
      return std::make_tuple(&(*arr)[i]...);
    }(std::make_index_sequence<n>());
  }
}

template <class T>
auto make_tuple_from_element(Array<T>* arr) {
  return make_tuple_from_element(reinterpret_cast<T*>(arr));
}

auto make_tuple_from_element(auto& e) { return std::make_tuple(e); }

auto flatten_c_arrays(const auto& tuple) {
  return [&]<std::size_t... i>(std::index_sequence<i...>) {
    return std::tuple_cat(make_tuple_from_element(std::get<i>(tuple))...);
  }(std::make_index_sequence<
             std::tuple_size_v<std::remove_cvref_t<decltype(tuple)>>>());
}

/// Creates a struct of type T from a tuple by moving the underlying
/// fields.
template <class T, class TupleType>
T move_from_tuple(TupleType&& _t) {
  auto ptr_tuple = tup_to_ptr_tuple(_t);

  using TargetTupleType = tuple_t<std::remove_cvref_t<T>>;

  auto pointers =
      flatten_c_arrays(unflatten_ptr_tuple<TargetTupleType>(ptr_tuple));
  return move_from_pointers<T>(pointers);
}

}  // namespace internal
}  // namespace rfl

#endif
