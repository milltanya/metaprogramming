#include <cstddef>
#include <type_traits>


template <class...>
class Annotate {};

//////////////////////////////////////////////////////////////

template <class T, class... Args>
concept AggregateConstructibleFrom = requires(Args... args) {
    T{ args... };
};

//////////////////////////////////////////////////////////////

template <class Type, size_t FieldIndex>
struct TagField {
    friend constexpr auto loopholeType(TagField<Type, FieldIndex>);   
    friend constexpr size_t loopholeAnnotateNumber(TagField<Type, FieldIndex>); 
};

template <class Type, size_t FieldIndex, class FieldType>
struct LoopholeSetType {
    friend constexpr auto loopholeType(TagField<Type, FieldIndex>) { return FieldType{}; };
};

template <class Type, size_t FieldIndex, size_t AnnotateNumber>
struct LoopholeSetAnnotateNumber {
    friend constexpr size_t loopholeAnnotateNumber(TagField<Type, FieldIndex>) { return AnnotateNumber; };
};

template <class Type, size_t FieldIndex, size_t AnnotateNumber>
struct LoopholeUbiq {
    template <class T>
    constexpr operator T() const noexcept {
        LoopholeSetType<Type, FieldIndex, T>{};
        LoopholeSetAnnotateNumber<Type, FieldIndex, AnnotateNumber>{};
        return {};
    };
};

//////////////////////////////////////////////////////////////

template <class Type, size_t FieldIndex, size_t AnnotationIndex>
struct TagAnnotate {
    friend constexpr auto loopholeAnnotate(TagAnnotate<Type, FieldIndex, AnnotationIndex>);
};

template <class Type, size_t FieldIndex, size_t AnnotationIndex, class... Annotations>
struct LoopholeSetAnnotate {
    friend constexpr auto loopholeAnnotate(TagAnnotate<Type, FieldIndex, AnnotationIndex>) { return Annotate<Annotations...>{}; };
};

template <class Type, size_t FieldIndex, size_t AnnotationIndex>
struct LoopholeAnnotate {
    template <class... Annotations> 
    constexpr operator Annotate<Annotations...>() const noexcept {
        LoopholeSetAnnotate<Type, FieldIndex, AnnotationIndex, Annotations...>{};
        return {};
    }
};

//////////////////////////////////////////////////////////////

template <class Type, size_t FieldIndex, size_t AnnotateIndex, class... AllFields>
static constexpr size_t ProcessFieldsImpl () {
    if constexpr (AggregateConstructibleFrom<Type, AllFields..., LoopholeAnnotate<Type, FieldIndex, AnnotateIndex>>) {
        return ProcessFieldsImpl<Type, FieldIndex, AnnotateIndex+1, AllFields..., LoopholeAnnotate<Type, FieldIndex, AnnotateIndex>>();
    } else if constexpr (AggregateConstructibleFrom<Type, AllFields..., LoopholeUbiq<Type, FieldIndex, AnnotateIndex>>) {
        return ProcessFieldsImpl<Type, FieldIndex+1, 0, AllFields..., LoopholeUbiq<Type, FieldIndex, AnnotateIndex>>();
    } else {
        [[maybe_unused]]
        Type t{AllFields{}...};
        return FieldIndex;
    }
};

template <class Type>
constexpr size_t ProcessFields() {
    return ProcessFieldsImpl<Type, 0, 0>();
}

//////////////////////////////////////////////////////////////

template <class Annotation1, class Annotation2>
struct ConcatAnnotate;

template <class... Annotations1, class... Annotations2>
struct ConcatAnnotate<Annotate<Annotations1...>, Annotate<Annotations2...>> {
    using type = Annotate<Annotations1..., Annotations2...>;
};

template <class Type, size_t FieldIndex, size_t AnnotateIndex, size_t AnnotateNumber, class Annotate1>
struct ConcatAllAnnotations {
    using type = Annotate1;
};

template <class Type, size_t FieldIndex, size_t AnnotateIndex, size_t AnnotateNumber, class Annotate1> requires (AnnotateIndex < AnnotateNumber)
struct ConcatAllAnnotations<Type, FieldIndex, AnnotateIndex, AnnotateNumber, Annotate1> {
    using current = typename ConcatAnnotate<Annotate1, decltype(loopholeAnnotate(TagAnnotate<Type, FieldIndex, AnnotateIndex>()))>::type;
    using type = typename ConcatAllAnnotations<Type, FieldIndex, AnnotateIndex+1, AnnotateNumber, current>::type;
};

template <class Type, size_t FieldIndex>
struct GetAnnotations {
    using type = typename ConcatAllAnnotations<Type, FieldIndex, 0, loopholeAnnotateNumber(TagField<Type, FieldIndex>{}), Annotate<>>::type;
};

//////////////////////////////////////////////////////////////

template <class Annotation, class Annotations>
constexpr bool hasAnnotationClass;

template <class Annotation, class... Annotations>
constexpr bool hasAnnotationClass<Annotation, Annotate<Annotations...>> = (std::is_same_v<Annotation, Annotations> || ... || false);

//////////////////////////////////////////////////////////////

template <template <class...> class Template, class Class>
constexpr bool IsSameTemplate  = false;

template <template <class...> class Template, class... Args>
constexpr bool IsSameTemplate<Template, Template<Args...>> = true;

template <template <class...> class AnnotationTemplate, class Annotations>
constexpr bool hasAnnotationTemplate;

template <template <class...> class AnnotationTemplate, class... Annotations>
constexpr bool hasAnnotationTemplate<AnnotationTemplate, Annotate<Annotations...>> = (IsSameTemplate<AnnotationTemplate, Annotations> || ... || false);

//////////////////////////////////////////////////////////////

template <template <class...> class AnnotationTemplate, class Head, class...Tail>
struct getAnnotationTemplateImpl {
    using type = typename getAnnotationTemplateImpl<AnnotationTemplate, Tail...>::type;
};

template <template <class...> class AnnotationTemplate, class Head, class...Tail> requires (IsSameTemplate<AnnotationTemplate, Head>)
struct getAnnotationTemplateImpl<AnnotationTemplate, Head, Tail...> {
    using type = Head;
};

template <template <class...> class AnnotationTemplate, class Annotations>
struct getAnnotationTemplate;

template <template <class...> class AnnotationTemplate, class... Annotations>
struct getAnnotationTemplate<AnnotationTemplate, Annotate<Annotations...>> { 
    using type = typename getAnnotationTemplateImpl<AnnotationTemplate, Annotations...>::type;
};

//////////////////////////////////////////////////////////////

template<class T, size_t N, size_t M>
struct LoopholeGet;

template<class T, size_t N, size_t M> requires (N < M)
struct LoopholeGet<T, N, M> {
    using Type = decltype(loopholeType(TagField<T, N>{}));
    using Annotations = typename GetAnnotations<T, N>::type;

    template <template <class...> class AnnotationTemplate>
    static constexpr bool has_annotation_template = hasAnnotationTemplate<AnnotationTemplate, Annotations>;

    template <class Annotation>
    static constexpr bool has_annotation_class = hasAnnotationClass<Annotation, Annotations>;

    template <template <class...> class AnnotationTemplate> requires (has_annotation_template<AnnotationTemplate>)
    using FindAnnotation = getAnnotationTemplate<AnnotationTemplate, Annotations>::type;
};

//////////////////////////////////////////////////////////////

template <class T>
struct Describe {

    static constexpr size_t num_fields = ProcessFields<T>();

    template <size_t I>
    using Field = LoopholeGet<T, I, num_fields>;
};

