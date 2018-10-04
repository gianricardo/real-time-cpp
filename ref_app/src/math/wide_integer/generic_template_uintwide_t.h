#ifndef GENERIC_TEMPLATE_UINTWIDE_T_2018_10_02_H_
  #define GENERIC_TEMPLATE_UINTWIDE_T_2018_10_02_H_

  #include <algorithm>
  #include <array>
  #include <cstddef>
  #include <cstdint>
  #include <cstring>
  #include <limits>
  #include <type_traits>

  #if !defined(WIDE_INTEGER_DISABLE_IOSTREAM)
    #include <iostream>
  #endif

  namespace wide_integer { namespace generic_template {
  // Forward declaration.
  template<typename ST,
           typename LT,
           const std::size_t Digits2>
  class uintwide_t;
  } }

  namespace std
  {
    // Forward declaration: Support for numeric_limits<>.
    template<typename ST,
             typename LT,
             const std::size_t Digits2>
    class numeric_limits<wide_integer::generic_template::uintwide_t<ST, LT, Digits2>>;
  }

  namespace wide_integer { namespace generic_template { namespace detail {

  // From an unsigned integral input parameter of type LT, extract
  // the low part of it. The type of the extracted low part is ST,
  // which has half the width of LT.
  template<typename ST, typename LT>
  ST make_lo(const LT& u)
  {
    // Compile-time checks.
    static_assert(    (std::numeric_limits<ST>::is_integer == true)
                  &&  (std::numeric_limits<LT>::is_integer == true)
                  &&  (std::numeric_limits<ST>::is_signed  == false)
                  &&  (std::numeric_limits<LT>::is_signed  == false)
                  && ((std::numeric_limits<ST>::digits * 2) == std::numeric_limits<LT>::digits),
                  "Error: Please check the characteristics of ushort_type and ularge_type");

    return static_cast<ST>(u);
  }

  // From an unsigned integral input parameter of type LT, extract
  // the high part of it. The type of the extracted high part is ST,
  // which has half the width of LT.
  template<typename ST, typename LT>
  ST make_hi(const LT& u)
  {
    // Compile-time checks.
    static_assert(    (std::numeric_limits<ST>::is_integer == true)
                  &&  (std::numeric_limits<LT>::is_integer == true)
                  &&  (std::numeric_limits<ST>::is_signed  == false)
                  &&  (std::numeric_limits<LT>::is_signed  == false)
                  && ((std::numeric_limits<ST>::digits * 2) == std::numeric_limits<LT>::digits),
                  "Error: Please check the characteristics of ushort_type and ularge_type");

    return static_cast<ST>(u >> std::numeric_limits<ST>::digits);
  }

  // Create a composite unsigned integral value having type LT.
  // Two constituents are used having type ST, whereby the
  // width of ST is half the width of LT.
  template<typename ST, typename LT>
  LT make_large(const ST& lo, const ST& hi)
  {
    // Compile-time checks.
    static_assert(    (std::numeric_limits<ST>::is_integer == true)
                  &&  (std::numeric_limits<LT>::is_integer == true)
                  &&  (std::numeric_limits<ST>::is_signed  == false)
                  &&  (std::numeric_limits<LT>::is_signed  == false)
                  && ((std::numeric_limits<ST>::digits * 2) == std::numeric_limits<LT>::digits),
                  "Error: Please check the characteristics of ushort_type and ularge_type");

    return LT(LT(static_cast<LT>(hi) << std::numeric_limits<ST>::digits) | LT(lo));
  }

  template<const std::size_t Digits2>
  struct verify_power_of_two
  {
    static const bool conditional_value =
      ((Digits2 != 0U) && ((Digits2 & (Digits2 - 1U)) == 0U));
  };

  } } } // namespace wide_integer::generic_template::detail

  namespace wide_integer { namespace generic_template {

  template<typename ST,
           typename LT,
           const std::size_t Digits2>
  class uintwide_t
  {
  public:
    static_assert(detail::verify_power_of_two<Digits2>::conditional_value == true,
                  "Error: The Digits2 template parameter must be a power of 2");

    // Class-local type definitions.
    using ushort_type = ST;
    using ularge_type = LT;

    static const std::size_t number_of_limbs =
      std::size_t(Digits2 / std::size_t(std::numeric_limits<ushort_type>::digits));

    using representation_type = std::array<ushort_type, number_of_limbs>;

    // Compile-time checks.
    static_assert(    (std::numeric_limits<ushort_type>::is_integer == true)
                  &&  (std::numeric_limits<ularge_type>::is_integer == true)
                  &&  (std::numeric_limits<ushort_type>::is_signed  == false)
                  &&  (std::numeric_limits<ularge_type>::is_signed  == false)
                  && ((std::numeric_limits<ushort_type>::digits * 2) == std::numeric_limits<ularge_type>::digits),
                  "Error: Please check the characteristics of ushort_type and ularge_type");

    // Default destructor.
    ~uintwide_t() = default;

    // Default constructor.
    uintwide_t() = default;

    // Constructors from built-in unsigned integral types that
    // are less wide than ushort_type or exactly as wide as ushort_type.
    template<typename UnsignedIntegralType>
    uintwide_t(const UnsignedIntegralType v,
              typename std::enable_if<   (std::is_fundamental<UnsignedIntegralType>::value == true)
                                      && (std::is_integral   <UnsignedIntegralType>::value == true)
                                      && (std::is_unsigned   <UnsignedIntegralType>::value == true)
                                      && (   std::numeric_limits<UnsignedIntegralType>::digits
                                          <= std::numeric_limits<ushort_type         >::digits)>::type* = nullptr)
    {
      values[0U] = ushort_type(v);

      std::fill(values.begin() + 1U, values.end(), ushort_type(0U));
    }

    // Constructors from built-in unsigned integral types that
    // are wider than ushort_type, and do not have exactly the
    // same width as ushort_type.
    template<typename UnsignedIntegralType>
    uintwide_t(const UnsignedIntegralType v,
               typename std::enable_if<   (std::is_fundamental<UnsignedIntegralType>::value == true)
                                       && (std::is_integral   <UnsignedIntegralType>::value == true)
                                       && (std::is_unsigned   <UnsignedIntegralType>::value == true)
                                       && (  std::numeric_limits<ushort_type         >::digits
                                           < std::numeric_limits<UnsignedIntegralType>::digits)>::type* = nullptr)
    {
      std::uint_fast32_t right_shift_amount_v = 0U;
      std::uint_fast8_t  index_u              = 0U;

      for( ; (index_u < values.size()) && (right_shift_amount_v < std::uint_fast32_t(std::numeric_limits<UnsignedIntegralType>::digits)); ++index_u)
      {
        values[index_u] = ushort_type(v >> (int) right_shift_amount_v);

        right_shift_amount_v += std::uint_fast32_t(std::numeric_limits<ushort_type>::digits);
      }

      std::fill(values.begin() + index_u, values.end(), ushort_type(0U));
    }

    // Constructor from the internal data representation.
    uintwide_t(const representation_type& rep)
    {
      std::copy(rep.cbegin(), rep.cend(), values.begin());
    }

    // Constructor from an initialization list.
    template<const std::size_t N>
    uintwide_t(const ushort_type(&init)[N])
    {
      static_assert(N <= number_of_limbs, "Error: The initialization list has too many elements.");

      std::copy(init, init + (std::min)(N, number_of_limbs), values.begin());
    }

    // Copy constructor.
    uintwide_t(const uintwide_t& other)
    {
      std::copy(other.values.cbegin(), other.values.cend(), values.begin());
    }

    // Constructor from constant character string.
    uintwide_t(const char* str_input)
    {
      if(rd_string(str_input) == false)
      {
        std::fill(values.begin(), values.end(), (std::numeric_limits<ushort_type>::max)());
      }
    }

    // Move constructor.
    uintwide_t(uintwide_t&& other) : values(static_cast<representation_type&&>(other.values)) { }

    // Assignment operator.
    uintwide_t& operator=(const uintwide_t& other)
    {
      if(this != &other)
      {
        std::copy(other.values.cbegin(), other.values.cend(), values.begin());
      }

      return *this;
    }

    // Implement cast operators that cast to types having less width.
    operator ushort_type() const { return values[0U]; }
    operator ularge_type() const { return detail::make_large<ushort_type, ularge_type>(values[0U], values[1U]); }

    // Provide a user interface to the internal data representation.
          representation_type&  representation()       { return values; }
    const representation_type&  representation() const { return values; }
    const representation_type& crepresentation() const { return values; }

    uintwide_t& operator+=(const uintwide_t& other)
    {
      // Unary addition function.
      std::array<ularge_type, number_of_limbs> result_as_ularge_array;

      ushort_type carry(0U);

      for(std::size_t i = 0U; i < number_of_limbs; ++i)
      {
        result_as_ularge_array[i] = ularge_type(ularge_type(values[i]) + other.values[i]) + carry;

        carry = detail::make_hi<ushort_type, ularge_type>(result_as_ularge_array[i]);

        values[i] = ushort_type(result_as_ularge_array[i]);
      }

      return *this;
    }

    uintwide_t& operator-=(const uintwide_t& other)
    {
      // Unary subtraction function.
      std::array<ularge_type, number_of_limbs> result_as_ularge_array;

      bool has_borrow = false;

      for(std::size_t i = 0U; i < number_of_limbs; ++i)
      {
        result_as_ularge_array[i] = ularge_type(values[i]) - other.values[i];

        if(has_borrow) { --result_as_ularge_array[i]; }

        has_borrow = (detail::make_hi<ushort_type, ularge_type>(result_as_ularge_array[i]) != ushort_type(0U));

        values[i] = ushort_type(result_as_ularge_array[i]);
      }

      return *this;
    }

    uintwide_t& operator*=(const uintwide_t& other)
    {
      // Unary multiplication function.
      std::array<ushort_type, number_of_limbs> w = {{ 0U }};

      for(std::size_t j = 0; j < number_of_limbs; ++j)
      {
        if(other.values[j] != ushort_type(0U))
        {
          ushort_type carry = ushort_type(0U);

          for(std::size_t i = 0U, iplusj = i + j; iplusj < number_of_limbs; ++i, ++iplusj)
          {
            const ularge_type t =
              ularge_type(ularge_type(ularge_type(values[i]) * other.values[j]) + w[iplusj]) + carry;

            w[iplusj] = detail::make_lo<ushort_type, ularge_type>(t);
            carry     = detail::make_hi<ushort_type, ularge_type>(t);
          }
        }
      }

      std::copy(w.cbegin(), w.cend(), values.begin());

      return *this;
    }

    uintwide_t& operator/=(const uintwide_t& other)
    {
      // Unary division function.
      division_knuth(other);

      return *this;
    }

    uintwide_t& operator%=(const uintwide_t& other)
    {
      // Unary modulus function.
      uintwide_t quotient(*this);

      quotient.division_knuth(other);

      operator-=(quotient *= other);

      return *this;
    }

    const uintwide_t& operator++()
    {
      // Operator pre-increment.
      std:size_t i = 0U;

      for( ; (i < (values.size() - 1U)) && (++values[i] == ushort_type(0U)); ++i) { ; }

      if(i == (values.size() - 1U)) { ++values[i]; }

      return *this;
    }

    const uintwide_t& operator--()
    {
      // Operator pre-decrement.
      std:size_t i = 0U;

      for( ; (i < (values.size() - 1U)) && (values[i]-- == ushort_type(0U)); ++i) { ; }

      if(i == (values.size() - 1U)) { --values[i]; }

      return *this;
    }

    // Operators post-increment and post decrement.
    uintwide_t operator++(int) { uintwide_t w(*this); ++(*this); return w; }
    uintwide_t operator--(int) { uintwide_t w(*this); --(*this); return w; }

    uintwide_t& operator|=(const uintwide_t& other)
    {
      // Bitwise OR.
      for(std::size_t i = 0U; i < number_of_limbs; ++i)
      {
        values[i] |= other.values[i];
      }

      return *this;
    }

    uintwide_t& operator^=(const uintwide_t& other)
    {
      // Bitwise XOR.
      for(std::size_t i = 0U; i < number_of_limbs; ++i)
      {
        values[i] ^= other.values[i];
      }

      return *this;
    }

    uintwide_t& operator&=(const uintwide_t& other)
    {
      // Bitwise AND.
      for(std::size_t i = 0U; i < number_of_limbs; ++i)
      {
        values[i] &= other.values[i];
      }

      return *this;
    }

    uintwide_t& operator<<=(const int n)
    {
      // Left-shift operator.
      if     (n <  0) { operator>>=(n); }
      else if(n == 0) { ; }
      else
      {
        if(n >= Digits2)
        {
          operator=(std::uint8_t(0U));
        }
        else
        {
          const std::size_t offset            = std::size_t(n) / std::size_t(std::numeric_limits<ushort_type>::digits);
          const std::size_t left_shift_amount = std::size_t(n) % std::size_t(std::numeric_limits<ushort_type>::digits);

          std::copy_backward(values.data(),
                             values.data() + (number_of_limbs - offset),
                             values.data() +  number_of_limbs);

          std::fill(values.begin(), values.begin() + offset, ushort_type(0U));

          ushort_type part_from_previous_value = ushort_type(0U);

          for(std::size_t i = offset; i < number_of_limbs; ++i)
          {
            const ushort_type t = values[i];

            values[i] = (t << int(left_shift_amount)) | part_from_previous_value;

            part_from_previous_value = ushort_type(t >> int(std::size_t(std::numeric_limits<ushort_type>::digits - left_shift_amount)));
          }
        }
      }

      return *this;
    }

    uintwide_t& operator>>=(const int n)
    {
      // Right-shift operator.
      if     (n <  0) { operator<<=(n); }
      else if(n == 0) { ; }
      else
      {
        if(n >= Digits2)
        {
          operator=(std::uint8_t(0U));
        }
        else
        {
          const std::size_t offset            = std::size_t(n) / std::size_t(std::numeric_limits<ushort_type>::digits);
          const std::size_t left_shift_amount = std::size_t(n) % std::size_t(std::numeric_limits<ushort_type>::digits);

          std::copy(values.begin() + offset,
                    values.begin() + number_of_limbs,
                    values.begin());

          std::fill(values.rbegin(), values.rbegin() + offset, ushort_type(0U));

          ushort_type part_from_previous_value = ushort_type(0U);

          for(std::size_t i = ((number_of_limbs - 1U) - offset); std::ptrdiff_t(i) >= 0; --i)
          {
            const ushort_type t = values[i];

            values[i] = (t >> int(left_shift_amount)) | part_from_previous_value;

            part_from_previous_value = ushort_type(t << int(std::size_t(std::numeric_limits<ushort_type>::digits - left_shift_amount)));
          }
        }
      }

      return *this;
    }

    // Implement comparison operators.
    bool operator==(const uintwide_t& other) const { return (compare(other) == std::int_fast8_t( 0)); }
    bool operator< (const uintwide_t& other) const { return (compare(other) == std::int_fast8_t(-1)); }
    bool operator> (const uintwide_t& other) const { return (compare(other) == std::int_fast8_t( 1)); }
    bool operator!=(const uintwide_t& other) const { return (compare(other) != std::int_fast8_t( 0)); }
    bool operator<=(const uintwide_t& other) const { return (compare(other) <= std::int_fast8_t( 0)); }
    bool operator>=(const uintwide_t& other) const { return (compare(other) >= std::int_fast8_t( 0)); }

    // Create helper variables and functions for supporting std::numeric_limits<>.
    static const int limits_helper_digits   = Digits2;
    static const int limits_helper_digits10 = static_cast<int>((std::uintmax_t(limits_helper_digits) * UINTMAX_C(301)) / 1000U);

    static uintwide_t limits_helper_max()
    {
      uintwide_t val;

      std::fill(val.values.begin(), val.values.end(), (std::numeric_limits<ushort_type>::max)());

      return val;
    }

    static uintwide_t limits_helper_min()
    {
      return uintwide_t(std::uint8_t(0U));
    }

    bool wr_string(      char*             str_result,
                   const std::uint_fast8_t base_rep     = 0x10U,
                   const bool              show_base    = true,
                   const bool              show_pos     = false,
                   const bool              is_uppercase = true,
                         std::size_t       field_width  = 0U,
                   const char              fill_char    = char('0')) const
    {
            uintwide_t t   (*this);
      const uintwide_t zero(std::uint8_t(0U));

      bool wr_string_is_ok = true;

      if(base_rep == 8U)
      {
      }
      else if(base_rep == 16U)
      {
        const ushort_type mask(std::uint8_t(0xFU));

        char str_temp[(32U + (Digits2 / 4U)) + 1U];

        std::size_t pos = (sizeof(str_temp) - 1U);

        if(t == zero)
        {
          --pos;

          str_temp[pos] = char('0');
        }
        else
        {
          while(t != zero)
          {
            char c(t.values[0U] & mask);

            if      (c <= 9)                            { c += char(0x30); }
            else if((c >= char(10)) && (c <= char(15))) { c += (is_uppercase ? char(55) : char(87)); }

            --pos;

            str_temp[pos] = c;

            t >>= 4;
          }
        }

        if(show_base)
        {
          --pos;

          str_temp[pos] = (is_uppercase ? char('X') : char('x'));
        }

        if(show_pos)
        {
          --pos;

          str_temp[pos] = char('+');
        }

        if(field_width != 0U)
        {
          field_width = (std::min)(field_width, std::size_t(sizeof(str_temp) - 1U));

          while(std::ptrdiff_t(pos) > std::ptrdiff_t((sizeof(str_temp) - 1U) - field_width))
          {
            --pos;

            str_temp[pos] = fill_char;
          }
        }

        str_temp[std::size_t(sizeof(str_temp) - 1U)] = char('\0');

        std::strcpy(str_result, str_temp + pos);
      }
      else if(base_rep == 10U)
      {
        char str_temp[(20U + std::size_t((std::uintmax_t(Digits2) * UINTMAX_C(301)) / UINTMAX_C(1000))) + 1U];

        std::size_t pos = (sizeof(str_temp) - 1U);

        if(t == zero)
        {
          --pos;

          str_temp[pos] = char('0');
        }
        else
        {
          const uintwide_t ten(std::uint8_t(10U));

          while(t != zero)
          {
            const uintwide_t t_temp(t);

            t /= ten;

            const char c((t_temp - (t * ten)).values[0U]);

            --pos;

            str_temp[pos] = c + char(0x30);
          }
        }

        if(show_pos)
        {
          --pos;

          str_temp[pos] = char('+');
        }

        if(field_width != 0U)
        {
          field_width = (std::min)(field_width, std::size_t(sizeof(str_temp) - 1U));

          while(std::ptrdiff_t(pos) > std::ptrdiff_t((sizeof(str_temp) - 1U) - field_width))
          {
            --pos;

            str_temp[pos] = fill_char;
          }
        }

        str_temp[std::size_t(sizeof(str_temp) - 1U)] = char('\0');

        std::strcpy(str_result, str_temp + pos);
      }
      else
      {
        wr_string_is_ok = false;
      }

      return wr_string_is_ok;
    }

  private:
    representation_type values;

    bool rd_string(const char* str_input)
    {
      std::fill(values.begin(), values.end(), ushort_type(0U));

      const std::size_t str_length = strlen(str_input);

      bool str_input_is_hex = false;

      std::size_t pos = 0U;

      if(str_length > 0U)
      {
        if(str_input[0U] == char('0'))
        {
          if(   (str_length > 1U)
             && ((str_input[1U] == char('x')) || (str_input[1U] == char('X'))))
          {
            str_input_is_hex = true;

            pos = 2U;
          }
        }
        else if(   str_input[0U] == char('x')
                || str_input[0U] == char('X'))
        {
          str_input_is_hex = true;

          pos = 1U;
        }
      }

      bool char_is_valid = true;

      for( ; ((pos < str_length) && char_is_valid); ++pos)
      {
        char c = str_input[pos];

        const bool char_is_apostrophe = (c == char(39));

        if(char_is_apostrophe == false)
        {
          if(str_input_is_hex)
          {
            if     ((c >= char('a')) && (c <= char('f'))) { c -= (0x20 + 55); }
            else if((c >= char('A')) && (c <= char('F'))) { c -=         55;  }
            else if((c >= char('0')) && (c <= char('9'))) { c -=       0x30;  }
            else                                          { char_is_valid = false; }

            if(char_is_valid)
            {
              operator<<=(4);

              values[0U] |= std::uint8_t(c);
            }
          }
          else
          {
            if   ((c >= char('0')) && (c <= char('9'))) { c -= 0x30; }
            else                                        { char_is_valid = false; }

            if(char_is_valid)
            {
              operator*=(10U);

              values[0U] += std::uint8_t(c);
            }
          }
        }
      }

      return char_is_valid;
    }

    void division_knuth(const uintwide_t& other)
    {
      // Use Knuth's long division algorithm.
      // The loop-ordering of indexes in Knuth's original
      // algorithm has been reversed due to the little-endian
      // data format used here.

      // See also:
      // D.E. Knuth, "The Art of Computer Programming, Volume 2:
      // Seminumerical Algorithms", Addison-Wesley (1998),
      // Section 4.3.1 Algorithm D and Exercise 16.

      using local_uint_index_type = std::size_t;

      local_uint_index_type u_offset = local_uint_index_type(0U);
      local_uint_index_type v_offset = local_uint_index_type(0U);

      // Compute the offsets for u and v.
      for(local_uint_index_type i = 0U; (i < number_of_limbs) && (      values[(number_of_limbs - 1U) - i] == ushort_type(0U)); ++i) { ++u_offset; }
      for(local_uint_index_type i = 0U; (i < number_of_limbs) && (other.values[(number_of_limbs - 1U) - i] == ushort_type(0U)); ++i) { ++v_offset; }

      if(v_offset == local_uint_index_type(number_of_limbs))
      {
        // The denominator is zero. Set the maximum value and return.
        // This also catches (0 / 0) and sets the maximum value for it.
        operator=(limits_helper_max());

        return;
      }

      if(u_offset == local_uint_index_type(number_of_limbs))
      {
        // The numerator is zero. Do nothing and return.
        return;
      }

      {
        const int result_of_compare_left_with_right = compare(other);

        const bool left_is_less_than_right = (result_of_compare_left_with_right == -1);
        const bool left_is_equal_to_right  = (result_of_compare_left_with_right ==  0);

        if(left_is_less_than_right)
        {
          // If the denominator is larger than the numerator,
          // then the result of the division is zero.
          operator=(std::uint8_t(0U));

          return;
        }

        if(left_is_equal_to_right)
        {
          // If the denominator is equal to the numerator,
          // then the result of the division is one.
          operator=(std::uint8_t(1U));

          return;
        }
      }

      if(v_offset == local_uint_index_type(number_of_limbs - 1U))
      {
        // The denominator has one single limb.
        // Use a one-dimensional division algorithm.

              ularge_type long_numerator    = ularge_type(0U);
        const ushort_type short_denominator = other.values[0U];

        for(local_uint_index_type i = local_uint_index_type(number_of_limbs - 1U); std::ptrdiff_t(i) >= 0; --i)
        {
          const ushort_type hi_part = ((i != local_uint_index_type(number_of_limbs - 1U))
                                        ? values[i + 1U]
                                        : ushort_type(0U));

          long_numerator = ularge_type(values[i]) + ((long_numerator - ularge_type(ularge_type(short_denominator) * hi_part)) << std::numeric_limits<ushort_type>::digits);

          values[i] = detail::make_lo<ushort_type, ularge_type>(long_numerator / ularge_type(short_denominator));
        }

        return;
      }

      // We will now use a simplified version
      // of the Knuth long division algorithm.
      {
        // Compute the normalization factor d.
        const ushort_type d =
          detail::make_lo<ushort_type, ularge_type>(  ((ularge_type(std::uint8_t(1U))) << std::numeric_limits<ushort_type>::digits)
                                                    /  (ularge_type(other.values[(number_of_limbs - 1U) - v_offset]) + ularge_type(std::uint8_t(1U))));

        // Step D1(b), normalize u -> u * d = uu.
        // Note the added digit in uu and also that
        // the data of uu have not been initialized yet.

        std::array<ushort_type, number_of_limbs + 1U> uu;

        if(d == ushort_type(1U))
        {
          // The normalization is one.
          std::copy(values.cbegin(), values.cend(), uu.begin());

          uu.back() = ushort_type(0U);
        }
        else
        {
          // Multiply u by d.
          ushort_type carry = 0U;

          local_uint_index_type i;

          for(i = local_uint_index_type(0U); i < local_uint_index_type(number_of_limbs - u_offset); ++i)
          {
            const ularge_type t = (ularge_type(values[i]) * ularge_type(d)) + ularge_type(carry);

            uu[i] = detail::make_lo<ushort_type, ularge_type>(t);
            carry = detail::make_hi<ushort_type, ularge_type>(t);
          }

          uu[i] = carry;
        }

        std::array<ushort_type, number_of_limbs> vv;

        // Step D1(c): normalize v -> v * d = vv.
        if(d == ushort_type(1U))
        {
          // The normalization is one.
          vv = other.values;
        }
        else
        {
          // Multiply v by d.
          ushort_type carry = 0U;

          for(local_uint_index_type i = local_uint_index_type(0U); i < local_uint_index_type(number_of_limbs - v_offset); ++i)
          {
            const ularge_type t = (ularge_type(other.values[i]) * ularge_type(d)) + ularge_type(carry);

            vv[i] = detail::make_lo<ushort_type, ularge_type>(t);
            carry = detail::make_hi<ushort_type, ularge_type>(t);
          }
        }

        // Step D2: Initialize j.
        // Step D7: Loop on j from m to 0.

        const local_uint_index_type n = local_uint_index_type(number_of_limbs - v_offset);
        const local_uint_index_type m = local_uint_index_type(number_of_limbs - u_offset) - n;

        for(local_uint_index_type j = local_uint_index_type(0U); j <= m; ++j)
        {
          // Step D3 [Calculate q_hat].
          //   if u[j] == v[j0]
          //     set q_hat = b - 1
          //   else
          //     set q_hat = (u[j] * b + u[j + 1]) / v[1]

          const local_uint_index_type uj  = (((number_of_limbs + 1U) - 1U) - u_offset) - j;
          const local_uint_index_type vj0 =   (number_of_limbs       - 1U) - v_offset;
          const ularge_type       u_j_j1  = (ularge_type(uu[uj]) << std::numeric_limits<ushort_type>::digits) + ularge_type(uu[uj - 1U]);

          ularge_type q_hat = ((uu[uj] == vv[vj0])
                                ? ularge_type((std::numeric_limits<ushort_type>::max)())
                                : u_j_j1 / ularge_type(vv[vj0]));

          // Decrease q_hat if necessary.
          // This means that q_hat must be decreased if the
          // expression [(u[uj] * b + u[uj - 1] - q_hat * v[vj0 - 1]) * b]
          // exceeds the range of uintwide_t.

          ularge_type t;

          for(;;)
          {
            t = u_j_j1 - ularge_type(q_hat * ularge_type(vv[vj0]));

            if(detail::make_hi<ushort_type, ularge_type>(t) != ushort_type(0U))
            {
              break;
            }

            if(   (ularge_type(vv[vj0 - 1U]) * ularge_type(q_hat))
               <= ((t << std::numeric_limits<ushort_type>::digits) + ularge_type(uu[uj - 2U])))
            {
              break;
            }

            --q_hat;
          }

          // Step D4: Multiply and subtract.
          // Replace u[j, ... j + n] by u[j, ... j + n] - q_hat * v[1, ... n].

          // Set nv = q_hat * (v[1, ... n]).
          {
            std::array<ushort_type, number_of_limbs + 1U> nv;

            ushort_type carry = 0U;

            local_uint_index_type i;

            for(i = local_uint_index_type(0U); i < n; ++i)
            {
              t     = (ularge_type(vv[i]) * ularge_type(q_hat)) + ularge_type(carry);

              nv[i] = detail::make_lo<ushort_type, ularge_type>(t);
              carry = detail::make_hi<ushort_type, ularge_type>(t);
            }

            nv[i] = carry;

            // Subtract nv[0, ... n] from u[j, ... j + n].
            local_uint_index_type borrow = 0U;
            local_uint_index_type ul     = uj - n;

            for(i = local_uint_index_type(0U); i <= n; ++i)
            {
              t        = (ularge_type(uu[ul]) - ularge_type(nv[i])) - ularge_type(std::uint8_t(borrow));

              uu[ul]   =   detail::make_lo<ushort_type, ularge_type>(t);
              borrow   = ((detail::make_hi<ushort_type, ularge_type>(t) != ushort_type(0U)) ? 1U : 0U);

              ++ul;
            }

            // Get the result data.
            values[local_uint_index_type(m - j)] = detail::make_lo<ushort_type, ularge_type>(q_hat);

            // Step D5: Test the remainder.
            // Set the result value: Set result.m_data[m - j] = q_hat
            // Condition: If u[j] < 0, in other words if the borrow
            // is non-zero, then step D6 needs to be carried out.

            if(borrow != 0U)
            {
              // Step D6: Add back.
              // Add v[1, ... n] back to u[j, ... j + n],
              // and decrease the result by 1.

              carry = 0U;
              ul    = uj - n;

              for(i = local_uint_index_type(0U); i <= n; ++i)
              {
                t        = (ularge_type(uu[ul]) + ularge_type(vv[i])) + ularge_type(carry);

                uu[ul++] = detail::make_lo<ushort_type, ularge_type>(t);
                carry    = detail::make_hi<ushort_type, ularge_type>(t);
              }

              uu[local_uint_index_type(m - j)] += carry;

              --values[local_uint_index_type(m - j)];
            }
          }
        }

        for(local_uint_index_type i = local_uint_index_type(m + 1U); i < local_uint_index_type(number_of_limbs); ++i)
        {
          // Clear the data elements that have not
          // been computed in the division algorithm.
          values[i] = 0U;
        }
      }
    }

    std::int_fast8_t compare(const uintwide_t& other) const
    {
      std::int_fast8_t return_value;
      std::ptrdiff_t   element_index;

      for(element_index = std::ptrdiff_t(number_of_limbs - 1U); element_index >= std::ptrdiff_t(0); --element_index)
      {
        if(values[std::size_t(element_index)] != other.values[std::size_t(element_index)])
        {
          break;
        }
      }

      if(element_index == std::ptrdiff_t(-1))
      {
        return_value = std::int_fast8_t(0);
      }
      else
      {
        const bool left_is_greater_than_right =
          ((values[std::size_t(element_index)] > other.values[std::size_t(element_index)]) ? true : false);

        return_value = (left_is_greater_than_right ? std::int_fast8_t(1) : std::int_fast8_t(-1));
      }

      return return_value;
    }
  };

  // Define some convenient unsigned wide integer types.
  using uint64_t    = uintwide_t<std::uint16_t, std::uint32_t,    64U>;
  using uint128_t   = uintwide_t<std::uint32_t, std::uint64_t,   128U>;
  using uint256_t   = uintwide_t<std::uint32_t, std::uint64_t,   256U>;
  using uint512_t   = uintwide_t<std::uint32_t, std::uint64_t,   512U>;
  using uint1024_t  = uintwide_t<std::uint32_t, std::uint64_t,  1024U>;
  using uint2048_t  = uintwide_t<std::uint32_t, std::uint64_t,  2048U>;
  using uint4096_t  = uintwide_t<std::uint32_t, std::uint64_t,  4096U>;
  using uint8192_t  = uintwide_t<std::uint32_t, std::uint64_t,  8192U>;

  // Prepare a base class for numeric_limits<> support.
  template<typename WideUnsignedIntegerType>
  class numeric_limits_base : public std::numeric_limits<unsigned int>
  {
  private:
    using uintwide_type = WideUnsignedIntegerType;

  public:
    static const int digits   = uintwide_type::limits_helper_digits;
    static const int digits10 = uintwide_type::limits_helper_digits10;

    static uintwide_type (max)() { return uintwide_type::limits_helper_max(); }
    static uintwide_type (min)() { return uintwide_type::limits_helper_min(); }
  };

  } } // namespace wide_integer::generic_template

  namespace std
  {
    // Specialization of std::numeric_limits<>.
    template<typename ST,
             typename LT,
             const std::size_t Digits2>
    class numeric_limits<wide_integer::generic_template::uintwide_t<ST, LT, Digits2>>
      : public wide_integer::generic_template::numeric_limits_base<wide_integer::generic_template::uintwide_t<ST, LT, Digits2>>
    {
    };
  }

  namespace wide_integer { namespace generic_template {

  template<typename ST, typename LT, const std::size_t Digits2> uintwide_t<ST, LT, Digits2> operator+ (const uintwide_t<ST, LT, Digits2>& left, const uintwide_t<ST, LT, Digits2>& right) { return uintwide_t<ST, LT, Digits2>(left).operator+=(right); }
  template<typename ST, typename LT, const std::size_t Digits2> uintwide_t<ST, LT, Digits2> operator- (const uintwide_t<ST, LT, Digits2>& left, const uintwide_t<ST, LT, Digits2>& right) { return uintwide_t<ST, LT, Digits2>(left).operator-=(right); }
  template<typename ST, typename LT, const std::size_t Digits2> uintwide_t<ST, LT, Digits2> operator* (const uintwide_t<ST, LT, Digits2>& left, const uintwide_t<ST, LT, Digits2>& right) { return uintwide_t<ST, LT, Digits2>(left).operator*=(right); }
  template<typename ST, typename LT, const std::size_t Digits2> uintwide_t<ST, LT, Digits2> operator/ (const uintwide_t<ST, LT, Digits2>& left, const uintwide_t<ST, LT, Digits2>& right) { return uintwide_t<ST, LT, Digits2>(left).operator/=(right); }
  template<typename ST, typename LT, const std::size_t Digits2> uintwide_t<ST, LT, Digits2> operator% (const uintwide_t<ST, LT, Digits2>& left, const uintwide_t<ST, LT, Digits2>& right) { return uintwide_t<ST, LT, Digits2>(left).operator%=(right); }

  template<typename ST, typename LT, const std::size_t Digits2> uintwide_t<ST, LT, Digits2> operator| (const uintwide_t<ST, LT, Digits2>& left, const uintwide_t<ST, LT, Digits2>& right) { return uintwide_t<ST, LT, Digits2>(left).operator|=(right); }
  template<typename ST, typename LT, const std::size_t Digits2> uintwide_t<ST, LT, Digits2> operator^ (const uintwide_t<ST, LT, Digits2>& left, const uintwide_t<ST, LT, Digits2>& right) { return uintwide_t<ST, LT, Digits2>(left).operator^=(right); }
  template<typename ST, typename LT, const std::size_t Digits2> uintwide_t<ST, LT, Digits2> operator& (const uintwide_t<ST, LT, Digits2>& left, const uintwide_t<ST, LT, Digits2>& right) { return uintwide_t<ST, LT, Digits2>(left).operator&=(right); }

  template<typename ST, typename LT, const std::size_t Digits2> uintwide_t<ST, LT, Digits2> operator<<(const uintwide_t<ST, LT, Digits2>& left, const int n)                              { return uintwide_t<ST, LT, Digits2>(left).operator<<=(n); }
  template<typename ST, typename LT, const std::size_t Digits2> uintwide_t<ST, LT, Digits2> operator>>(const uintwide_t<ST, LT, Digits2>& left, const int n)                              { return uintwide_t<ST, LT, Digits2>(left).operator>>=(n); }

  } } // namespace wide_integer::generic_template

#endif // GENERIC_TEMPLATE_UINTWIDE_T_2018_10_02_H_
