#include <disposer/tool/ct_pretty_name.hpp>

#include <array>
#include <optional>
#include <string_view>


namespace disposer{ namespace{


	constexpr bool is_name_char(char c)noexcept{
		return
			(c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9') ||
			c == '_';
	}

	std::array< std::pair< std::string, std::string >, 11 > replace_table{{
		{"long double", "long double"},
		{"float", "float32"},
		{"double", "float64"},
		{type_index::type_id< std::uint8_t >().pretty_name(), "uint8"},
		{type_index::type_id< std::uint16_t >().pretty_name(), "uint16"},
		{type_index::type_id< std::uint64_t >().pretty_name(), "uint64"},
		{type_index::type_id< std::uint32_t >().pretty_name(), "uint32"},
		{type_index::type_id< std::int8_t >().pretty_name(), "uint8"},
		{type_index::type_id< std::int16_t >().pretty_name(), "uint16"},
		{type_index::type_id< std::int64_t >().pretty_name(), "uint64"},
		{type_index::type_id< std::int32_t >().pretty_name(), "uint32"}}};

	inline std::optional< std::size_t > find_replace_index(
		std::string_view source_text
	){
		for(std::size_t i = 0; i < replace_table.size(); ++i){
			auto const& from = replace_table[i].first;
			if(source_text.size() < from.size()) continue;
			if(from == source_text.substr(0, from.size())){
				return i;
			}
		}
		return {};
	}


} }


namespace disposer{


	std::string purify_type_print(std::string const& type){
		std::string result = "";
		result.reserve(type.size());

		bool test = true;
		for(std::size_t i = 0; i < type.size(); ++i){
			auto const c = type[i];
			if(!test){
				if(!is_name_char(c)){
					test = true;
				}

				result += c;
				continue;
			}

			if(!is_name_char(c)){
				result += c;
				continue;
			}

			std::string_view rest(&type[i], type.size() - i);
			std::string abc(rest);
			auto const index = find_replace_index(rest);

			if(!index){
				result += c;
				continue;
			}

			auto const& replace_pair = replace_table[*index];
			auto const& from = replace_pair.first;
			auto const& to = replace_pair.second;

			test = false;
			if(type.size() - i > from.size()){
				auto const ce = type[i + from.size()];
				if(is_name_char(ce)){
					result += c;
					continue;
				}
			}

			result += to;
			i += from.size() - 1;
		}

		return result;
	}


}
