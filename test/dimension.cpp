#include <disposer/core/dimension.hpp>


int main(){
	using namespace disposer;

	{
		// dimension construtors
		constexpr dimension< int, char, float > default_constructed;
		constexpr auto copy_constructed = default_constructed;
		constexpr auto move_constructed = std::move(default_constructed);
		(void)copy_constructed; (void)move_constructed;
	}

	{
		// construction by dimension_c
		constexpr auto variable_constructed = dimension_c< int, char, float >;
		(void)variable_constructed;
	}

	{
		// dimension_list construction by dimension_c
		auto list0 = dimension_list{};
		static_assert(std::is_same_v< decltype(list0), dimension_list<> >);
		auto list1 = dimension_list{
				dimension_c< int, char, float >
			};
		static_assert(std::is_same_v< decltype(list1), dimension_list<
				dimension< int, char, float >
			> >);
		auto list2 = dimension_list{
				dimension_c< int, char, float >,
				dimension_c< double, bool >
			};
		static_assert(std::is_same_v< decltype(list2), dimension_list<
				dimension< int, char, float >,
				dimension< double, bool >
			> >);
	}

	{
		// direct dimension_list construction
		auto list0 = dimension_list<>{};
		auto list1 = dimension_list<
				dimension< int, char, float >
			>{};
		auto list2 = dimension_list<
				dimension< int, char, float >,
				dimension< double, bool >
			>{};
		(void)list0; (void)list1; (void)list2;
	}

// 	constexpr auto invalid_0_type = dimension_c<>;
// 	constexpr auto invalid_1_type = dimension_c< int >;
}
