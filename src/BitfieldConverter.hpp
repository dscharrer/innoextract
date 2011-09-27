 
#include <boost/utility/enable_if.hpp>
#include <boost/integer.hpp>
#include <boost/integer/static_log2.hpp>
#include <boost/integer/static_min_max.hpp>
#include <boost/integer_traits.hpp>

namespace boost {
	template <class T>
	class integer_traits<const T> : public integer_traits<T> { };
}

template <size_t N, class Type = void, class Enable = void>
struct is_power_of_two {
	static const bool value = false;
};
template <size_t N, class Type>
struct is_power_of_two<N, Type, typename boost::enable_if_c<(N & (N - 1)) == 0>::type> {
	static const bool value = true;
	typedef Type type;
};

template <size_t N, class Enable = void>
struct log_next_power_of_two {
	static const size_t value = boost::static_log2<N>::value + 1;
};
template <size_t N>
struct log_next_power_of_two<N, typename boost::enable_if<is_power_of_two<N> >::type> {
	static const size_t value = boost::static_log2<N>::value;
};

template <size_t N, class Enable = void>
struct next_power_of_two {
	static const size_t value = size_t(1) << (boost::static_log2<N>::value + 1);
};
template <size_t N>
struct next_power_of_two<N, typename boost::enable_if<is_power_of_two<N> >::type> {
	static const size_t value = N;
};


struct fast_integers {
	
private:
	
	template <size_t Bits, class Dummy = void> struct _impl { };
	template <class Dummy> struct _impl<8, Dummy> { typedef uint_fast8_t type; };
	template <class Dummy> struct _impl<16, Dummy> { typedef uint_fast16_t type; };
	template <class Dummy> struct _impl<32, Dummy> { typedef uint_fast32_t type; };
	template <class Dummy> struct _impl<64, Dummy> { typedef uint_fast64_t type; };
	template <class Dummy> struct _impl<128, Dummy> { typedef __uint128_t type; };
	
public:
	
	template <size_t Bits>
	struct bits : public _impl<boost::static_unsigned_max<8, next_power_of_two<Bits>::value>::value> { };
	
};

struct exact_integers {
	
private:
	
	template <size_t Bits, class Dummy = void> struct _impl { };
	template <class Dummy> struct _impl<8, Dummy> { typedef uint8_t type; };
	template <class Dummy> struct _impl<16, Dummy> { typedef uint16_t type; };
	template <class Dummy> struct _impl<32, Dummy> { typedef uint32_t type; };
	template <class Dummy> struct _impl<64, Dummy> { typedef uint64_t type; };
	template <class Dummy> struct _impl<128, Dummy> { typedef __uint128_t type; };
	
public:
	
	template <size_t Bits>
	struct bits : public _impl<boost::static_unsigned_max<8, next_power_of_two<Bits>::value>::value> { };
	
};

struct bitset_types {
	
	template <size_t Bits>
	struct bits {
		typedef std::bitset<Bits> type;
	};
	
};


/*!
 * Converter that rearranges bits in an integer.
 *
 * Conversion is reduced to a minimal number of mask & shift operations at compile-time. 
 *
 * Usage:
 *
 *  bitset_converter&lt;&gt; is an empty converter list (cannot be used without adding at least one mappings).
 *  (list)::add::map&lt;from, to&gt maps the from'th input bit to the to'th output bit.
 *
 *  Convenience function to add a continous region of mappings:
 *  bitset_converter&lt;&gt;::add::value&lt;to2&gt; is equivalent to bitset_converter&lt;&gt;::add::map&lt;0, to2&gt;
 *  (list)::add::map&lt;from, to&gt::add::value&lt;to2&gt; is equivalent to ::add::map&lt;from, to&gt::add::map&lt;from + 1, to2&gt;
 *
 *  Inut bits without a corresponding "from" entry are ignored.
 *  Output bit without a corresponding "to" entry are always zero.
 *
 *  The same input/output bit can appear in multiple mappings.
 *
 *  Invoke the converter: (list)::convert(integer)
 *
 * Limitations:
 *
 *  Input bits must fit in a native integer type provided by in_types::bits&lt;bits&gt;.
 *
 *  Output bits must fit in an integer type selected by out_types::bits&lt;bits&gt;.
 *
 * Example:
 *
 *  // Create a converter that swaps the first two bits, keeps the next one and ignores all others.
 *  typedef bitset_converter<>::add::map<0, 1>::add::map<1, 0>::add::value<2> Converter;
 *
 *  // Convert something.
 *  Converter::convert(3);
 * 
 */
template <class out_types = fast_integers, class in_types = fast_integers>
struct bitset_converter {
	
private:
	
	typedef ptrdiff_t shift_type;
	typedef size_t index_type;
	
	template <class Combiner, class Entry>
	struct IterateEntries {
		static const typename Combiner::type value = Combiner::template combine<Entry, (IterateEntries<Combiner, typename Entry::next>::value)>::value;
	};
	template <class Combiner> struct IterateEntries<Combiner, void> { static const typename Combiner::type value = Combiner::base; };
	template <class Type, Type Base> struct Combiner { typedef Type type; static const Type base = Base; };
	
	template<class Getter, class Type>
	struct MaxCombiner : public Combiner<Type, boost::integer_traits<Type>::const_min> {
		template <class Entry, Type accumulator>
		struct combine { static const Type value = boost::static_signed_max<Getter::template get<Entry>::value, accumulator>::value; };
	};
	
	template<class Getter, class Type>
	struct MinCombiner : public Combiner<Type, boost::integer_traits<Type>::const_max> {
		template <class Entry, Type accumulator>
		struct combine { static const Type value = boost::static_signed_min<Getter::template get<Entry>::value, accumulator>::value; };
	};
	
	struct ShiftGetter { template<class Entry> struct get { static const shift_type value = Entry::shift; }; };
	struct FromGetter { template<class Entry> struct get { static const index_type value = Entry::from; }; };
	struct ToGetter { template<class Entry> struct get { static const index_type value = Entry::to; }; };
	
	template<shift_type Shift, class Type>
	struct ShiftMaskCombiner : public Combiner<Type, Type(0)> {
		template <class Entry, Type mask>
		struct combine { static const Type value = mask | ( (Entry::shift == Shift) ? (Type(1) << Entry::from) : Type(0) ); };
	};
	
	template<class List>
	struct Builder;
	
	template<index_type From, index_type To, class Next = void>
	struct Entry {
		
		typedef Entry<From, To, Next> This;
		
		static const index_type from = From;
		static const index_type to = To;
		typedef Next next;
		
		static const shift_type shift = shift_type(from) - shift_type(to);
		
		static const shift_type max_shift = IterateEntries<MaxCombiner<ShiftGetter, shift_type>, This>::value;
		static const shift_type min_shift = IterateEntries<MinCombiner<ShiftGetter, shift_type>, This>::value;
		
		static const index_type in_bits = IterateEntries<MaxCombiner<FromGetter, index_type>, This>::value + 1;
		typedef typename in_types::template bits<in_bits>::type in_type;
		
		static const index_type out_bits = IterateEntries<MaxCombiner<ToGetter, index_type>, This>::value + 1;
		typedef typename out_types::template bits<out_bits>::type out_type;
		
		template<shift_type Shift>
		struct ShiftMask { static const in_type value = IterateEntries<ShiftMaskCombiner<Shift, in_type>, This>::value; };
		
		template <shift_type Shift>
		inline static typename boost::enable_if_c<(Shift >= shift_type(0)), out_type>::type evaluate(in_type value) {
			return out_type((value & ShiftMask<Shift>::value) >> Shift);
		}
		template <shift_type Shift>
		inline static typename boost::enable_if_c<(Shift < shift_type(0)), out_type>::type evaluate(in_type value) {
			return out_type(value & ShiftMask<Shift>::value) << (-Shift); 
		}
		
		template<shift_type Shift, class Enable = void>
		struct NextShift { static const shift_type value = Shift + 1; };
		template<shift_type Shift>
		struct NextShift<Shift, typename boost::enable_if_c<Shift != max_shift && ShiftMask<Shift + 1>::value == in_type(0)>::type > {
			static const shift_type value = NextShift<Shift + 1>::value;
		};
		
		template <shift_type Shift>
		inline static typename boost::enable_if_c<(NextShift<Shift>::value != max_shift + 1), out_type>::type map(in_type value) {
			return evaluate<Shift>(value) | (map<NextShift<Shift>::value>(value));
		}
		template <shift_type Shift>
		inline static typename boost::enable_if_c<(NextShift<Shift>::value == max_shift + 1), out_type>::type map(in_type value) {
			return evaluate<Shift>(value);
		}
		
	public:
		
		typedef Builder<This> add;
		
		static out_type convert(in_type value) {
			return map<min_shift>(value);
		}
		
	};
	
	template<class List>
	struct Builder {
		
		template<index_type From, index_type To>
		struct map : public Entry<From, To, List> { };
		
		template<index_type To, class Current = List>
		struct value : public Entry<Current::from + 1, To, Current> { };
		
		template<index_type To>
		struct value<To, void> : public Entry<0, To> { };
		
	};
	
public:
	
	typedef Builder<void> add;
	
};
