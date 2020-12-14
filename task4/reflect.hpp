#include <cstddef>
#include <tuple>


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
    // typedef typename Annotate<AllFields...>::something_made_up X;
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

template <class... Annotations1, class... Annotations2>
constexpr auto ConcatAnnotate(Annotate<Annotations1...>, Annotate<Annotations2...>) {
    return Annotate<Annotations1..., Annotations2...>{};
};

template <class Type, size_t FieldIndex, size_t AnnotateIndex, size_t AnnotateNumber, class Annotate1>
constexpr auto ConcatTwoAnnotates() {
    return Annotate1{};
};

template <class Type, size_t FieldIndex, size_t AnnotateIndex, size_t AnnotateNumber, class Annotate1> requires (AnnotateIndex < AnnotateNumber)
constexpr auto ConcatTwoAnnotates() {
    return ConcatTwoAnnotates<Type, FieldIndex, AnnotateIndex+1, AnnotateNumber, decltype(ConcatAnnotate(Annotate1{}, loopholeAnnotate(TagAnnotate<Type, FieldIndex, AnnotateIndex>())))>();
};

template <class Type, size_t FieldIndex>
constexpr auto GetAnnotations() {
    return ConcatTwoAnnotates<Type, FieldIndex, 0, loopholeAnnotateNumber(TagField<Type, FieldIndex>{}), Annotate<>>();
};

//////////////////////////////////////////////////////////////

template <class Annotation>
constexpr bool hasAnnotationClassImpl() {
    return false;   
}

template <class Annotation, class Head, class...Tail>
constexpr bool hasAnnotationClassImpl() {
    if constexpr (std::is_same_v<Annotation, Head>) {
        return true;
    } else {
        return hasAnnotationClassImpl<Annotation, Tail...>();
    }
}

template <class Annotation, class Annotations>
struct hasAnnotationClass;

template <class Annotation, class... Annotations>
struct hasAnnotationClass<Annotation, Annotate<Annotations...>> { 
    static constexpr bool has = hasAnnotationClassImpl<Annotation, Annotations...>();
};

//////////////////////////////////////////////////////////////

template <template <class...> class AnnotationTemplate>
constexpr bool hasAnnotationTemplateImpl() {
    return false;   
}

template <template <class...> class AnnotationTemplate, class Head>
struct IsSameTemplate {
    static constexpr bool value = false;
};

template <template <class...> class AnnotationTemplate, class... Args>
struct IsSameTemplate<AnnotationTemplate, AnnotationTemplate<Args...>> {
    static constexpr bool value = true;
};

template <template <class...> class AnnotationTemplate, class Head, class...Tail>
constexpr bool hasAnnotationTemplateImpl() {
    if constexpr (IsSameTemplate<AnnotationTemplate, Head>::value) {
        return true;
    } else {
        return hasAnnotationTemplateImpl<AnnotationTemplate, Tail...>();
    }
}

template <template <class...> class AnnotationTemplate, class Annotations>
struct hasAnnotationTemplate;

template <template <class...> class AnnotationTemplate, class... Annotations>
struct hasAnnotationTemplate<AnnotationTemplate, Annotate<Annotations...>> { 
    static constexpr bool has = hasAnnotationTemplateImpl<AnnotationTemplate, Annotations...>();
};

//////////////////////////////////////////////////////////////

template <template <class...> class AnnotationTemplate, class Head, class...Tail>
struct getAnnotationTemplateImpl {
    using type = typename getAnnotationTemplateImpl<AnnotationTemplate, Tail...>::type;
};

template <template <class...> class AnnotationTemplate, class Head, class...Tail> requires (IsSameTemplate<AnnotationTemplate, Head>::value)
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
struct LoopholeGet {
    using Type = decltype(loopholeType(TagField<T, N>{}));
    using Annotations = decltype(GetAnnotations<T, N>());

    template <template <class...> class AnnotationTemplate>
    static constexpr bool has_annotation_template = hasAnnotationTemplate<AnnotationTemplate, Annotations>::has;

    template <class Annotation>
    static constexpr bool has_annotation_class = hasAnnotationClass<Annotation, Annotations>::has;

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

