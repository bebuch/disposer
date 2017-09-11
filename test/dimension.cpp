#include <disposer/core/dimension.hpp>


template < std::size_t DI, std::size_t I >
using ic = disposer::ct_index_component< DI, I >;


int main(){
	using namespace disposer;

	// dimension construtors
	{
		constexpr dimension< int, char, float > default_constructed;
		constexpr auto copy_constructed = default_constructed;
		constexpr auto move_constructed = std::move(default_constructed);
		(void)copy_constructed; (void)move_constructed;
	}

	// construction by dimension_c
	{
		constexpr auto variable_constructed = dimension_c< int, char, float >;
		(void)variable_constructed;
	}

	// dimension_list construction by dimension_c and
	// dimension_list
	{
		auto list = dimension_list{};
		static_assert(std::is_same_v< decltype(list), dimension_list<> >);

		dimension_list plist{list};
		static_assert(std::is_same_v< decltype(plist), dimension_list<> >);
	}
	{
		auto list = dimension_list{dimension_c< int, char, float >};
		static_assert(std::is_same_v< decltype(list),
			dimension_list< dimension< int, char, float > > >);

		dimension_list plist{list};
		static_assert(std::is_same_v< decltype(plist),
			dimension_list< dimension< int, char, float > > >);

		static_assert(std::is_same_v<
			decltype(reduce_dimension_list(plist, ic< 0, 0 >{})),
			dimension_list< dimension< int > > >);

		static_assert(std::is_same_v<
			decltype(reduce_dimension_list(plist, ic< 0, 1 >{})),
			dimension_list< dimension< char > > >);

		static_assert(std::is_same_v<
			decltype(reduce_dimension_list(plist, ic< 0, 2 >{})),
			dimension_list< dimension< float > > >);
	}
	{
		auto list = dimension_list{
				dimension_c< int, char, float >,
				dimension_c< double, bool >
			};
		static_assert(std::is_same_v< decltype(list), dimension_list<
				dimension< int, char, float >,
				dimension< double, bool >
			> >);

		dimension_list plist{list};
		static_assert(std::is_same_v< decltype(plist),
			dimension_list<
				dimension< int, char, float >,
				dimension< double, bool >
			> >);

		static_assert(std::is_same_v<
			decltype(reduce_dimension_list(plist, ic< 0, 0 >{})),
			dimension_list< dimension< int >, dimension< double, bool > > >);

		static_assert(std::is_same_v<
			decltype(reduce_dimension_list(plist, ic< 0, 1 >{})),
			dimension_list< dimension< char >, dimension< double, bool > > >);

		static_assert(std::is_same_v<
			decltype(reduce_dimension_list(plist, ic< 0, 2 >{})),
			dimension_list< dimension< float >, dimension< double, bool > > >);

		static_assert(std::is_same_v<
			decltype(reduce_dimension_list(plist, ic< 1, 0 >{})),
			dimension_list<
				dimension< int, char, float >, dimension< double >
			> >);

		static_assert(std::is_same_v<
			decltype(reduce_dimension_list(plist, ic< 1, 1 >{})),
			dimension_list<
				dimension< int, char, float >, dimension< bool >
			> >);

		static_assert(std::is_same_v<
			decltype(reduce_dimension_list(
				reduce_dimension_list(plist, ic< 0, 0 >{}), ic< 1, 0 >{})),
			dimension_list< dimension< int >, dimension< double > > >);

		static_assert(std::is_same_v<
			decltype(reduce_dimension_list(
				reduce_dimension_list(plist, ic< 0, 1 >{}), ic< 1, 0 >{})),
			dimension_list< dimension< char >, dimension< double > > >);

		static_assert(std::is_same_v<
			decltype(reduce_dimension_list(
				reduce_dimension_list(plist, ic< 0, 2 >{}), ic< 1, 0 >{})),
			dimension_list< dimension< float >, dimension< double > > >);

		static_assert(std::is_same_v<
			decltype(reduce_dimension_list(
				reduce_dimension_list(plist, ic< 0, 0 >{}), ic< 1, 1 >{})),
			dimension_list< dimension< int >, dimension< bool > > >);

		static_assert(std::is_same_v<
			decltype(reduce_dimension_list(
				reduce_dimension_list(plist, ic< 0, 1 >{}), ic< 1, 1 >{})),
			dimension_list< dimension< char >, dimension< bool > > >);

		static_assert(std::is_same_v<
			decltype(reduce_dimension_list(
				reduce_dimension_list(plist, ic< 0, 2 >{}), ic< 1, 1 >{})),
			dimension_list< dimension< float >, dimension< bool > > >);
	}

	// direct dimension_list construction
	{
		auto list = dimension_list<>{};
		(void)list;
	}
	{
		auto list = dimension_list< dimension< int, char, float > >{};
		(void)list;
	}
	{
		auto list = dimension_list<
				dimension< int, char, float >,
				dimension< double, bool >
			>{};
		(void)list;
	}

// 	constexpr auto invalid_0_type = dimension_c<>;
// 	constexpr auto invalid_1_type = dimension_c< int >;
}
