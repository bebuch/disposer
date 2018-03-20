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


	namespace{

		template < typename T >
		std::string name(){
			return type_index::type_id< T >().pretty_name();
		}

	}


	auto const replace_table = []{
			std::array< std::pair< std::string, std::string >, 17 > list{{
					{name< char >(),               "char"},
					{name< char16_t >(),           "char16_t"},
					{name< char32_t >(),           "char32_t"},
					{name< signed char >(),        "signed char"},
					{name< unsigned char >(),      "unsigned char"},
					{name< short >(),              "short"},
					{name< unsigned short >(),     "unsigned short"},
					{name< int >(),                "int"},
					{name< unsigned int >(),       "unsigned int"},
					{name< long >(),               "long"},
					{name< unsigned long >(),      "unsigned long"},
					{name< long long >(),          "long long"},
					{name< unsigned long long >(), "unsigned long long"},
					{name< float >(),              "float32"},
					{name< double >(),             "float64"},
					{name< long double >(),        "long double"},
					{name< bool >(),               "bool"}
				}};

			auto exchange = [&list](
					std::string const& first,
					std::string&& second
				){
					for(auto& pair: list){
						if(pair.first == first){
							pair.second = std::move(second);
						}
					}
				};

			exchange(name< std::uint8_t >(), "uint8"),
			exchange(name< std::uint16_t >(), "uint16"),
			exchange(name< std::uint64_t >(), "uint64"),
			exchange(name< std::uint32_t >(), "uint32"),
			exchange(name< std::int8_t >(), "int8"),
			exchange(name< std::int16_t >(), "int16"),
			exchange(name< std::int64_t >(), "int64"),
			exchange(name< std::int32_t >(), "int32");

			std::sort(list.begin(), list.end(),
				[](auto const& lhs, auto const& rhs){
					return lhs.first.size() > rhs.first.size();
				});

			return list;
		}();

	inline std::optional< std::size_t > find_replace_index(
		std::string_view source_text
	){
		for(std::size_t i = 0; i < replace_table.size(); ++i){
			auto const& from = replace_table[i].first;
			if(source_text.size() < from.size()) continue;
			if(
				from == source_text.substr(0, from.size()) &&
				(
					source_text.size() == from.size() ||
					!is_name_char(source_text[from.size()])
				)
			){
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

			test = false;

			std::string_view rest(&type[i], type.size() - i);
			auto const index = find_replace_index(rest);

			if(!index){
				result += c;
				continue;
			}

			auto const& [from, to] = replace_table[*index];

			result += to;
			i += from.size() - 1;
		}

		return result;
	}


}
