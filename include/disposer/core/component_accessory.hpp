//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__component_accessory__hpp_INCLUDED_
#define _disposer__core__component_accessory__hpp_INCLUDED_

#include "disposer.hpp"

#include "../tool/add_log.hpp"
#include "../tool/false_c.hpp"

#include <boost/hana/core/is_a.hpp>
#include <boost/hana/index_if.hpp>
#include <boost/hana/optional.hpp>


namespace disposer{


	struct parameter_name_tag;

	/// \brief Accessory of a component in component modules
	template <
		typename TypeList,
		typename State,
		typename Parameters >
	class component_accessory
		: public add_log< component_accessory<
			TypeList, State, Parameters > >
	{
	public:
		/// \brief Constructor
		component_accessory(
			TypeList,
			State& state,
			Parameters const& parameters,
			::disposer::disposer& disposer,
			std::string_view location
		)noexcept
			: state_(state)
			, parameters_(parameters)
			, disposer_(disposer)
			, location_(location) {}


		/// \brief Get reference to a parameter-object via
		///        its corresponding compile time name
		template < typename Name >
		auto const& operator()(Name const& name)const noexcept{
			return get(*this, name);
		}

		/// \brief Get reference to a parameter-object via
		///        its corresponding compile time name
		template < typename Name >
		auto& operator()(Name const& name)noexcept{
			return get(*this, name);
		}

		/// \brief Get type by dimension index
		template < std::size_t DI >
		static constexpr auto dimension(hana::size_t< DI > i)noexcept{
			static_assert(DI < TypeList::type_count,
				"component has less then DI dimensions");
			return TypeList::types[i];
		}

		/// \brief Get access to the state object if one exists
		auto& state()noexcept{
			return state_;
		}

		/// \brief Get read only access to the state object if one exists
		auto const& state()const noexcept{
			return state_;
		}

		/// \brief Get reference to the disposer object
		::disposer::disposer& disposer()noexcept{
			return disposer_;
		}

		/// \brief Get const reference to the disposer object
		::disposer::disposer const& disposer()const noexcept{
			return disposer_;
		}


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			os << location_;
		}


	private:
		template < typename This, typename Name >
		static auto& get(This& this_, Name const& name)noexcept{
			using name_t = std::remove_reference_t< Name >;
			if constexpr(hana::is_a< parameter_name_tag, name_t >()){
				auto const index = hana::index_if(this_.parameters_,
					[name](auto const& p){ return p.name == name; });

				static_assert(!hana::is_nothing(index),
					"parameter name doesn't exist");

				return this_.parameters_[*index].get();
			}else{
				static_assert(detail::false_c< Name >,
					"name is not a parameter_name");
			}
		}


		/// \brief Component state
		State& state_;

		/// \brief hana::tuple of parameters
		Parameters const& parameters_;

		/// \brief Reference to the disposer object
		::disposer::disposer& disposer_;

		/// \brief Prefix for log messages
		std::string_view location_;
	};


}


#endif
