#include <disposer/core/dimension.hpp>


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
	// partial_deduced_dimension_list
	{
		auto list = dimension_list{};
		static_assert(std::is_same_v< decltype(list), dimension_list<> >);

		partial_deduced_dimension_list plist{list};
		static_assert(std::is_same_v< decltype(plist),
			partial_deduced_dimension_list<> >);
	}
	{
		auto list = dimension_list{
				dimension_c< int, char, float >
			};
		static_assert(std::is_same_v< decltype(list), dimension_list<
				dimension< int, char, float >
			> >);

		partial_deduced_dimension_list plist{list};
		static_assert(std::is_same_v< decltype(plist),
			partial_deduced_dimension_list<
				dimension< int, char, float >
			> >);

		static_assert(std::is_same_v<
			decltype(make_partial_deduced_dimension_list(plist,
				hana::make_tuple(ct_index_component< 0, 0 >{}))),
			partial_deduced_dimension_list<
				hana::basic_type< int >
			> >);

		static_assert(std::is_same_v<
			decltype(make_partial_deduced_dimension_list(plist,
				hana::make_tuple(ct_index_component< 0, 1 >{}))),
			partial_deduced_dimension_list<
				hana::basic_type< char >
			> >);

		static_assert(std::is_same_v<
			decltype(make_partial_deduced_dimension_list(plist,
				hana::make_tuple(ct_index_component< 0, 2 >{}))),
			partial_deduced_dimension_list<
				hana::basic_type< float >
			> >);
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

		partial_deduced_dimension_list plist{list};
		static_assert(std::is_same_v< decltype(plist),
			partial_deduced_dimension_list<
				dimension< int, char, float >,
				dimension< double, bool >
			> >);

		static_assert(std::is_same_v<
			decltype(make_partial_deduced_dimension_list(plist,
				hana::make_tuple(ct_index_component< 0, 0 >{}))),
			partial_deduced_dimension_list<
				hana::basic_type< int >, dimension< double, bool >
			> >);

		static_assert(std::is_same_v<
			decltype(make_partial_deduced_dimension_list(plist,
				hana::make_tuple(ct_index_component< 0, 1 >{}))),
			partial_deduced_dimension_list<
				hana::basic_type< char >, dimension< double, bool >
			> >);

		static_assert(std::is_same_v<
			decltype(make_partial_deduced_dimension_list(plist,
				hana::make_tuple(ct_index_component< 0, 2 >{}))),
			partial_deduced_dimension_list<
				hana::basic_type< float >, dimension< double, bool >
			> >);

		static_assert(std::is_same_v<
			decltype(make_partial_deduced_dimension_list(plist,
				hana::make_tuple(ct_index_component< 1, 0 >{}))),
			partial_deduced_dimension_list<
				dimension< int, char, float >, hana::basic_type< double >
			> >);

		static_assert(std::is_same_v<
			decltype(make_partial_deduced_dimension_list(plist,
				hana::make_tuple(ct_index_component< 1, 1 >{}))),
			partial_deduced_dimension_list<
				dimension< int, char, float >, hana::basic_type< bool >
			> >);

		static_assert(std::is_same_v<
			decltype(make_partial_deduced_dimension_list(plist,
				hana::make_tuple(
					ct_index_component< 0, 0 >{},
					ct_index_component< 1, 0 >{}))),
			partial_deduced_dimension_list<
				hana::basic_type< int >, hana::basic_type< double >
			> >);

		static_assert(std::is_same_v<
			decltype(make_partial_deduced_dimension_list(plist,
				hana::make_tuple(
					ct_index_component< 0, 1 >{},
					ct_index_component< 1, 0 >{}))),
			partial_deduced_dimension_list<
				hana::basic_type< char >, hana::basic_type< double >
			> >);

		static_assert(std::is_same_v<
			decltype(make_partial_deduced_dimension_list(plist,
				hana::make_tuple(
					ct_index_component< 0, 2 >{},
					ct_index_component< 1, 0 >{}))),
			partial_deduced_dimension_list<
				hana::basic_type< float >, hana::basic_type< double >
			> >);

		static_assert(std::is_same_v<
			decltype(make_partial_deduced_dimension_list(plist,
				hana::make_tuple(
					ct_index_component< 0, 0 >{},
					ct_index_component< 1, 1 >{}))),
			partial_deduced_dimension_list<
				hana::basic_type< int >, hana::basic_type< bool >
			> >);

		static_assert(std::is_same_v<
			decltype(make_partial_deduced_dimension_list(plist,
				hana::make_tuple(
					ct_index_component< 0, 1 >{},
					ct_index_component< 1, 1 >{}))),
			partial_deduced_dimension_list<
				hana::basic_type< char >, hana::basic_type< bool >
			> >);

		static_assert(std::is_same_v<
			decltype(make_partial_deduced_dimension_list(plist,
				hana::make_tuple(
					ct_index_component< 0, 2 >{},
					ct_index_component< 1, 1 >{}))),
			partial_deduced_dimension_list<
				hana::basic_type< float >, hana::basic_type< bool >
			> >);
	}

	// direct dimension_list construction
	{
		auto list = dimension_list<>{};
		(void)list;
	}
	{
		auto list = dimension_list<
				dimension< int, char, float >
			>{};
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
