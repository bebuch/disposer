//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__chain__hpp_INCLUDED_
#define _disposer__core__chain__hpp_INCLUDED_

#include "id_generator.hpp"
#include "exec_info.hpp"

#include "../config/chain_module_list.hpp"
#include "../config/embedded_config.hpp"

#include <mutex>
#include <condition_variable>


namespace disposer{


	struct chain_not_lockable: std::runtime_error{
		chain_not_lockable(std::string const& chain_name)
			: std::runtime_error("chain(" + chain_name
				+ ") is enabled, can't lock") {}
	};


	/// \brief A process chain
	///
	/// Properties:
	/// - no 2 identical modules (in different executions) must running
	///   simultaneously
	/// - it must not be overtaken
	class chain{
	public:
		/// \brief Construct a proccess chain
		///
		/// \param config_chain configuration data from config file
		/// \param generate_id Reference to a id_generator
		chain(
			module_maker_list const& module_makers,
			component_module_makers_list& component_module_makers,
			types::embedded_config::chain const& config_chain,
			id_generator& generate_id
		);


		/// \brief Disable all modules and destruct chain
		~chain();


		/// \brief Chaines are not copyable
		chain(chain const&) = delete;

		/// \brief Chaines are not movable
		chain(chain&&) = delete;


		/// \brief Chaines are not copyable
		chain& operator=(chain const&) = delete;

		/// \brief Chaines are not movable
		chain& operator=(chain&&) = delete;


		/// \brief Execute the proccess chain
		///
		/// The chain must be enabled, otherwise an exception is thrown.
		exec_info exec();


		/// \brief Enables the chain for exec calls
		///
		/// With every enable() call the enabled counter is increased. It is
		/// decreased by calling disable().
		///
		/// If the counter was 0 before the call, than all modules in the chain
		/// become enabled.
		void enable();

		/// \brief Disables the chain for exec calls
		///
		/// With every enable() call the enabled counter is increased. It is
		/// decreased by calling disable().
		///
		/// If the counter becomes 0 through the call, than all modules in the
		/// chain become disabled.
		void disable()noexcept;


		/// \brief true if enable was more often called than disable
		bool is_enabled()const{
			return enable_count_ > 0;
		}


		/// \brief Name of the chain
		std::string const name;


	private:
		/// \brief List of modules
		chain_module_list const modules_;


		/// \brief Referenz to the global id_generator
		id_generator& generate_id_;

		/// \brief Chain local id generator
		id_generator generate_exec_id_;


		/// \brief Mutex for enable and disable
		std::mutex enable_mutex_;

		/// \brief Count of enable() calls minus count of disable() calls
		std::atomic< std::size_t > enable_count_;


		/// \brief Count of running exec() calls
		std::atomic< std::size_t > exec_calls_count_;

		/// \brief Manages exec() and enable() / disable() calls
		std::condition_variable enable_cv_;
	};


}


#endif
