//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__input__hpp_INCLUDED_
#define _disposer__core__input__hpp_INCLUDED_

#include "input_base.hpp"
#include "input_name.hpp"


namespace disposer{


	/// \brief Hana Tag for input
	struct input_tag{};

	template < typename Name, typename T >
	class input;

	/// \brief Class input_key access key
	struct input_key{
	private:
		/// \brief Constructor
		constexpr input_key()noexcept = default;

		template < typename Name, typename T >
		friend class input;
	};

	/// \brief The actual input type
	template < typename Name, typename T, bool IsRequired >
	class input: public input_base{
	public:
#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
		static_assert(hana::is_a< input_name_tag, Name >);
#endif

		/// \brief Hana tag to identify inputs
		using hana_tag = input_tag;


		/// \brief Whether the input must always be connected with an output
		static constexpr auto is_required = std::bool_constant< IsRequired >();


		/// \brief Compile time name of the input
		using name_type = Name;

		/// \brief Name object
		static constexpr auto name = name_type{};


		/// \brief Constructor
		input(output_base* output)
			: input_base(verify_output(output)) {}


	private:
		/// \brief Verify that the output connection is valid
		static output_base* verify_output(output_base* output){
			if(output != nullptr){
				auto const type = output->type(input_key(), hana::type_c< T >);
				if(type_index< T > != type){
					throw std::logic_error("input(" + std::to_std_string(name)
						+ ") of type [" + type_index< T >.pretty_name()
						+ "] is connected with output of type ["
						+ type.pretty_name() + "]");
				}
			}else if constexpr(is_required){
				throw std::logic_error("input(" + std::to_std_string(name)
					+ ") is required");
			}

			return output;
		}
	};


}


#endif
