//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__enabled_chain__hpp_INCLUDED_
#define _disposer__core__enabled_chain__hpp_INCLUDED_

#include "system_ref.hpp"


namespace disposer{


	/// \brief A resource guard for chain enable/disable
	class enabled_chain{
	public:
		/// \brief Calls enable on the chain object
		enabled_chain(class system_ref& system_ref, std::string const& name)
			: chain_(system_ref.get_chain(name))
		{
			chain_.enable();
		}

		/// \brief Calls enable on the chain object
		enabled_chain(class system& system, std::string const& name)
			: chain_(system.get_chain(name))
		{
			chain_.enable();
		}

		/// \brief Calls disable on the chain object
		~enabled_chain(){
			chain_.disable();
		}

		/// \brief Exec chain
		bool exec()noexcept{
			return chain_.exec();
		}

		/// \brief Get name of the chain
		std::string const& name()const noexcept{
			return chain_.name;
		}


	private:
		/// \brief The chain object
		chain& chain_;
	};


}


#endif
