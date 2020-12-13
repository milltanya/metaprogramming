#include <cstddef>
#include <tuple>


template <class...>
class Annotate {};

//////////////////////////////////////////////////////////////
struct AnnotateConstructor {
    template <class... C> 
    constexpr operator Annotate<C...>() const noexcept {
        return {};
    }
};

template <class T, class... Args>
concept AggregateConstructibleFrom = requires(Args... args) {
    T{ args... };
};

template <class T, int N>
struct Tag {
    friend auto loopholeType(Tag<T, N>);   
    friend auto loopholeAnnotations(Tag<T, N>);   
};

template<class T, int N, class Type>
struct LoopholeSetType {
    friend auto loopholeType(Tag<T, N>) { return Type{}; };
};

template<class T, int N, class... Annotations>
struct LoopholeSetAnnotations {
    friend auto loopholeAnnotations(Tag<T, N>) { return Annotate<Annotations...>{}; };
};

template <class T, size_t I, class... Annotations>
struct LoopholeUbiq {
    template <class Type>
    constexpr operator Type() const noexcept {
        LoopholeSetType<T, I, Type>{};
        LoopholeSetAnnotations<T, I, Annotations...>{};
        return {};
    };
};

template <class T, size_t I, class... C>
struct ProcessFieldsImpl {
    template <class... A> 
    static constexpr size_t SetAnnotation() {
        if constexpr (AggregateConstructibleFrom<T, C..., A..., AnnotateConstructor>) {
            return SetAnnotation<A..., AnnotateConstructor>();
        } else if constexpr (AggregateConstructibleFrom<T, C..., A...,  LoopholeUbiq<T, I, A...>>) {
            return ProcessFieldsImpl<T, I+1, C..., A..., LoopholeUbiq<T, I, A...>>::SetAnnotation();
        } else {
            [[maybe_unused]]
            T t{C{}...};
            return I;
        }
    }
};

template <class T>
constexpr size_t ProcessFields() {
    return ProcessFieldsImpl<T, 0>::SetAnnotation();
}

template<class T, int N>
struct LoopholeGet {
    using Type = decltype(loopholeType(Tag<T, N>{}));
    using Annotations = decltype(loopholeAnnotations(Tag<T, N>{}));
};


//////////////////////////////////////////////////////////////////


template <class T>
struct Describe {

    static constexpr size_t num_fields = ProcessFields<T>();

    template <size_t I>
    using Field = LoopholeGet<T, I>;
};

