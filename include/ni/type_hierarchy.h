//
// MIT License
//
// Copyright © 2018
// Native Instruments
//
// For more detailed information, please read the LICENSE in the root directory.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - >8


//!---------------------------------------------------------------------------------------------------------------------
//!
//!  \file
//!
//!  `type_hierarchy` is a framework that allows to build polymorphic inheritance based hierarchies with runtime type
//!  information to allow casting along the hierarchy. The main goal is to not depend on the RTTI based C++ mechanism
//!  which has a lot over overhead. See benchmarks for a comparison (general speed-up: 10-20 times faster).
//!
//!  The framework provides a mechanism to inherit types from others to build the actual library and provides tools
//!  to test for convertibility and casting. It is also well integrated into `ni::match` which is meant to be the
//!  main use case. It does not provide any mechanism to have some sort of function overload.
//!
//!  Example usage scenario:
//!  ```
//!  struct UserDefinedBase { ... };   // this can have some common state that is shared with all objects.
//!  
//!  // get the root type from the hierarchy which serves as reference/pointer to base class.
//!  using Root = ni::type_hierarchy::from_base<UserDefinedBase>;
//!  
//!  // derive custom types, the first template argument must always be the derived type itself. This is
//!  // required due to the missing reflections in C++ and is used to assign a unique id to the new type.
//!  struct Child1 : ni::sub_type<Child1, Root> {...};              // 1st level type
//!  struct Child2 : ni::sub_type<Child2, Root> {...};              // another 1st level type
//!  struct GrandChild1 : ni::sub_type<GrandChild1, Child1> {...};  // 2nd level type deriving from Child1
//!  struct GrandChild2 : ni::sub_type<GrandChild2, Child1> {...};  // another 2nd level type
//!  struct GreatGrandChild1 : ni::sub_type<GreatGrandChild1, GrandChild1> {...};  // 3rd level type
//!  ```
//!
//!  By default there are 4 hierarchies with 255 possible types per level & branch. The client can configure
//!  this when creating the root type by specifying the number of bits to encode the ids on each level.
//!
//!  Some examples
//!  ```
//!  using VeryDeepHierarchy = ni::type_hierarchy::from_base<Base, 4, 4, 4, 4, 4, 4, 4, 4>;  // 15 types on each level
//!  using FlatHierarchy = ni::type_hierarchy::from_base<Base, 32>;              // 4 billion types on only one level
//!  using MixedHierarchy = ni::type_hierarchy::from_base<Base, 3, 23, 6>;       // 7 types on level 1, 8 million types
//!                                                                              // on level 2, 63 types on the level 3
//!  ```
//!
//!  In order to prevent copy-paste errors (due to the CRTP-redundancy) it is advisable to use the macro
//!  `NI_SUB_TYPE` to derive types. Example:
//!  ```
//!   struct NI_SUB_TYPE( Child1, Root ) {...};
//!   // or
//!   NI_SUB_TYPE( struct Child1, Root ) {...};
//!   // instead of
//!   struct Child1 : ni::sub_type<Child1, Root> {...};
//!  ```
//!
//!
//!  TODO (nice to haves)
//!  • test dyn_cast from/to several levels w/ & w/o const
//!  • test with actual storage
//!  • assert that derived class is arg in id_holder (i.e. assert that the 1st arg of sub_type is always the deriver)
//!
//!---------------------------------------------------------------------------------------------------------------------

#pragma once

#include <ni/meta/scan_add.h>
#include <ni/meta/fold_add.h>

#include <boost/assert.hpp>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <type_traits>

namespace ni {

namespace type_hierarchy_detail {

    // The implementation is based on three parts:
    // 1. a system to tag and identify the inheritance level
    // 2. a system of types that get interleaved into the hierarchy to manage the types' identity
    // 3. a system to assign unique IDs to each type and test for castability.


    //------------------------------------------------------------------------------------------------------------------
    // 1. Level System
    //------------------------------------------------------------------------------------------------------------------

    // each class in hierarchy derives from a level_tag
    template <int Level>
    struct level_tag {};


    // level_of computes the level of a given type based on the type's level_tag
    template <typename T, typename LevelTag = level_tag<0>>
    struct level_of;

    template <typename T, int N>
    struct level_of<T, level_tag<N>> : std::conditional_t
    <  std::is_convertible<T, level_tag<N>>::value
    ,  level_of<T, level_tag<N+1>>
    ,  std::integral_constant<int, N-1>
    > {};

    template <typename T>
    struct level_of<T, level_tag<0>> : std::conditional_t
    <  std::is_convertible<T, level_tag<0>>::value
    ,  level_of<T, level_tag<1>>
    ,  std::integral_constant<int, -1>
    > {};

    template <>
    struct level_of<void, level_tag<0>> : std::integral_constant<int, 0> {};

    template <typename T>
    constexpr int level_of_v = level_of<T>::value;



    //------------------------------------------------------------------------------------------------------------------
    // 2. Type Hierarchy
    //------------------------------------------------------------------------------------------------------------------

    // the types 'type<>' manage the ids which are used by identify each type at runtime

    template <typename Config, typename... >
    struct id_holder;

    template <typename Config>
    struct config_holder {};

    template <typename T>
    struct get_config
    {
        template <typename Config>
        static auto deduce(config_holder<Config>*) -> Config;
        template <typename Config>
        static auto deduce(const config_holder<Config>*) -> Config;
        static auto deduce(void*) -> void;

        using type = decltype(deduce(std::declval<T*>()));
    };

    template <typename T>
    using get_config_t = typename get_config<T>::type;

    // base type inherits from user defined root
    // holds the id which is used by the system to identify each type at runtime
    template <typename Config>
    struct id_holder<Config> : Config::base_type, level_tag<0>, config_holder<Config>
    {
    private:
        typename Config::id_t  m_type_hierarchy_id__;
        template <typename, typename... >
        friend struct id_holder;
    protected:
        id_holder() = default;
    public:
        typename Config::id_t type_hierarchy_id__() const { return m_type_hierarchy_id__; }
    };


    // each derived type gets its own id by putting this type in between base and derived
    template <typename Config, typename Derived>
    struct id_holder<Config, Derived, void> : id_holder<Config>, level_tag<1>
    {
    private:
        static const typename Config::id_t  s_type_hierarchy__;
        static typename Config::id_t type_hierarchy_init_id__();
        template <typename, typename...>
        friend struct id_holder;
        template <typename, typename, typename>
        friend struct convertible_to_impl;
    public:
        id_holder()
        {
            id_holder::m_type_hierarchy_id__ = s_type_hierarchy__;
        }
    };

    // all deeper derivatives derive from some SuperType that has to derive from a type<N,...>
    template <typename Config, typename Derived, typename SuperType>
    struct id_holder<Config, Derived, SuperType> : SuperType, level_tag<level_of_v<SuperType>+1>
    {
    private:
        static_assert( level_of_v<SuperType> < typename Config::bits_per_level{}.size()
                     , "Supported number of hierarchy levels exceeded."
                     );
        static const typename Config::id_t  s_type_hierarchy__;
        static typename Config::id_t type_hierarchy_init_id__();
        template <typename, typename... >
        friend struct id_holder;
        template <typename, typename, typename>
        friend struct convertible_to_impl;
    public:
        using super_t = id_holder<Config, Derived, SuperType>;

        template <typename... Args>
        id_holder(Args&&... args) : SuperType{std::forward<Args>(args)...}
        {
            SuperType::m_type_hierarchy_id__ = s_type_hierarchy__;
        }
    };



    //------------------------------------------------------------------------------------------------------------------
    // 3. Static Type IDs
    //------------------------------------------------------------------------------------------------------------------

    template <typename T, size_t N> struct constexpr_array { T data[N]; };

    template <int N, int... Ns>
    constexpr int get(std::integer_sequence<int,Ns...>)
    {
        return constexpr_array<int, sizeof...(Ns)>{{Ns...}}.data[N];
    }

    template <typename Config, int N>
    constexpr int lookup_v = get<N>(typename Config::level_shifts{});

    template <typename Config, typename T>
    constexpr typename Config::id_t shift_v = lookup_v<Config, level_of_v<T>>;

    template <typename Config, typename T>
    constexpr typename Config::id_t mask_v = (1 << shift_v<Config, T>) - 1;



    // next_id() generates the id which is used to tag each type on each level
    template <typename Config, int Level=0>
    typename Config::id_t next_id()
    {
        static typename Config::id_t id{};
        ++id;
        BOOST_ASSERT_MSG( id < (static_cast<typename Config::id_t>(1) << get<Level>(typename Config::bits_per_level{}))
                        , "Ids for level are exhausted."
                        );
        return id;
    }


    template <typename TargetType, typename TargetConfig, typename SourceConfig>
    struct convertible_to_impl
    {
        static bool apply(id_holder<SourceConfig> const&)
        {
            return false;
        }
    };

    template <typename TargetType, typename Config>
    struct convertible_to_impl<TargetType, Config, Config>
    {
        static bool apply(id_holder<Config> const& src)
        {
            return (mask_v<Config, TargetType> & src.type_hierarchy_id__()) == TargetType::s_type_hierarchy__;
        }
    };

    template <typename Config>
    struct convertible_to_impl<typename Config::base_type, void, Config>
    {
        static bool apply(id_holder<Config> const&)
        {
            return true;
        }
    };

    template <typename TargetType, typename SourceType,
        typename = std::enable_if_t<std::is_base_of<level_tag<0>, SourceType>::value> >
    bool convertible_to(SourceType const& x)
    {
        using config_t = get_config_t<SourceType>;
        auto const& src = static_cast<id_holder<config_t> const&>(x);
        return convertible_to_impl<TargetType, get_config_t<TargetType>, config_t>::apply(src);
    }


    template <typename Config, typename Derived>
    typename Config::id_t id_holder<Config, Derived, void>::type_hierarchy_init_id__()
    {
        static const typename Config::id_t id = next_id<Config, 0>();
        return id;
    }

    template <typename Config, typename Derived>
    const typename Config::id_t id_holder<Config, Derived, void>::s_type_hierarchy__ = type_hierarchy_init_id__();

    template <typename Config, typename Derived, typename SuperType>
    typename Config::id_t id_holder<Config, Derived, SuperType>::type_hierarchy_init_id__()
    {
        constexpr auto shift = shift_v<Config, SuperType>;
        static const typename Config::id_t id =
            SuperType::type_hierarchy_init_id__() | (next_id<Config, level_of_v<SuperType>>() << shift);
        return id;
    }


    template <typename Config, typename Derived, typename SuperType>
    const typename Config::id_t id_holder<Config, Derived, SuperType>::s_type_hierarchy__ = type_hierarchy_init_id__();


    //------------------------------------------------------------------------------------------------------------------
    // Casting
    //------------------------------------------------------------------------------------------------------------------

    template <typename TargetType, typename Config>
    TargetType* dyn_cast(id_holder<Config>* p)
    {
        return convertible_to<TargetType>(*p) ? static_cast<TargetType*>(p) : nullptr;
    }

    template <typename TargetType, typename Config, typename T1>
    TargetType* dyn_cast(id_holder<Config, T1, void>* p)
    {
        return convertible_to<TargetType>(*p) ? static_cast<TargetType*>(p) : nullptr;
    }

    template <typename TargetType, typename Config>
    TargetType const* dyn_cast(id_holder<Config> const* p)
    {
        return convertible_to<TargetType>(*p) ? static_cast<TargetType*>(p) : nullptr;
    }

    template <typename TargetType, typename Config, typename D>
    TargetType const* dyn_cast(id_holder<Config, D, void> const* p)
    {
        return convertible_to<TargetType>(*p) ? static_cast<TargetType const*>(p) : nullptr;
    }


    //------------------------------------------------------------------------------------------------------------------
    // Configure & Build Hierarchy
    //------------------------------------------------------------------------------------------------------------------

    template <int NumBits>
    struct int_for_bits { using type = typename int_for_bits<NumBits+1>::type; };

    template <> struct int_for_bits<8>  { using type = std::uint8_t; };
    template <> struct int_for_bits<16> { using type = std::uint16_t; };
    template <> struct int_for_bits<32> { using type = std::uint32_t; };
    template <> struct int_for_bits<64> { using type = std::uint64_t; };

    template <typename BaseType, int... BitsPerLevel>
    struct builder
    {
        struct config
        {
            using base_type = BaseType;
            using bits_per_level = std::integer_sequence<int, BitsPerLevel...>;
            using level_shifts = meta::scan_add_t<int, 0, BitsPerLevel...>;
            using id_t = typename int_for_bits<meta::fold_add_v<int, BitsPerLevel...>>::type;
        };

        using root_t = type_hierarchy_detail::id_holder<config>;
    };

    template <typename BaseType>
    struct builder<BaseType> : builder<BaseType, 8, 8, 8, 8> {};

    template <typename BaseType, int... BitsPerLevel>
    using root_t = typename builder<BaseType, BitsPerLevel...>::root_t;


    template <typename Derived, typename Super>
    struct sub_type_impl
    {
        using type = id_holder<get_config_t<Super>, Derived, Super>;
    };

    template <typename Derived, typename Config>
    struct sub_type_impl<Derived, id_holder<Config>>
    {
        using type = id_holder<Config, Derived, void>;
    };

}

namespace type_hierarchy {

    template <typename BaseType, int... BitsPerLevel>
    using from_base = type_hierarchy_detail::root_t<BaseType, BitsPerLevel...>;

    template <typename Derived, typename Super>
    using sub_type = typename type_hierarchy_detail::sub_type_impl<Derived, Super>::type;

    using type_hierarchy_detail::convertible_to;
}

using type_hierarchy::sub_type;
using type_hierarchy::convertible_to;


#define  NI_SUB_TYPE( DERIVED, SUPER )  DERIVED : public ni::sub_type< DERIVED, SUPER >


} // ::ni
