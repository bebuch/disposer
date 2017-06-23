//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__module__hpp_INCLUDED_
#define _disposer__module__hpp_INCLUDED_

#include "output_info.hpp"
#include "input.hpp"
#include "output.hpp"
#include "parameter.hpp"
#include "module_base.hpp"
#include "add_log.hpp"
#include "module_make_data.hpp"

#include <type_traits>
#include <atomic>


namespace disposer{


	/// \brief std::ref as callable object
	struct ref{
		template < typename T >
		constexpr auto operator()(T& iop)const noexcept{
			return std::ref(iop);
		}
	};


	/// \brief Accessory of a \ref module without log
	template < typename IOP_List >
	class module_config{
	public:
		/// \brief Constructor
		template < typename MakerList, typename MakeData, std::size_t ... I >
		module_config(
			MakerList const& maker_list,
			MakeData const& data,
			std::string_view location,
			std::index_sequence< I ... >
		)
			: // iop_list_ can be referenced before initialization
				iop_ref_list_(hana::transform(iop_list_, ref{}))
			, iop_list_(iops_make_data(
				iop_make_data(maker_list[hana::size_c< I >], data, location),
				location, hana::slice_c< 0, I >(iop_ref_list_),
				hana::size_c< I >) ...)
		{
			(void)location; // GCC bug (silance unused warning)
		}


		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename IOP >
		decltype(auto) operator()(IOP const& iop)noexcept{
			return get(*this, iop);
		}

		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename IOP >
		decltype(auto) const operator()(IOP const& iop)const noexcept{
			return get(*this, iop);
		}


	private:
		/// \brief Implementation for \ref operator()
		template < typename Config, typename IOP >
		static decltype(auto) get(Config& config, IOP const& iop)noexcept{
			using iop_t = std::remove_reference_t< IOP >;
			static_assert(
				hana::is_a< input_name_tag, iop_t > ||
				hana::is_a< output_name_tag, iop_t > ||
				hana::is_a< parameter_name_tag, iop_t >,
				"parameter is not an input_name, output_name or "
				"parameter_name");

			using iop_tag = typename iop_t::hana_tag;

			auto iop_ref = hana::find_if(config.iop_ref_list_, [&iop](auto ref){
				using tag = typename decltype(ref)::type::name_type::hana_tag;
				return hana::type_c< iop_tag > == hana::type_c< tag >
					&& ref.get().name == iop.value;
			});

			auto is_iop_valid = iop_ref != hana::nothing;
			static_assert(is_iop_valid, "requested iop doesn't exist");

			return iop_ref->get();
		}


		/// \brief Like IOP_List but with elements in std::reference_wrapper
		using iop_ref_list_type =
			decltype(hana::transform(std::declval< IOP_List& >(), ref{}));

		/// \brief hana::tuple of references to inputs, outputs and parameters
		iop_ref_list_type iop_ref_list_;

		/// \brief hana::tuple of the inputs, outputs and parameters
		IOP_List iop_list_;


		/// \brief std::vector with references to all input's (input_base)
		friend auto generate_input_list(module_config& config){
			auto iop_ref_list = hana::transform(config.iop_list_, ref{});
			auto input_ref_list = hana::filter(iop_ref_list, [](auto ref){
				return hana::is_a< input_tag >(ref.get());
			});
			return hana::unpack(input_ref_list, [](auto ... input_ref){
				return std::vector< std::reference_wrapper< input_base > >{
					std::reference_wrapper< input_base >(input_ref) ... };
			});
		}

		/// \brief std::vector with references to all output's (output_base)
		friend auto generate_output_list(module_config& config){
			auto iop_ref_list = hana::transform(config.iop_list_, ref{});
			auto output_ref_list = hana::filter(iop_ref_list, [](auto ref){
				return hana::is_a< output_tag >(ref.get());
			});
			return hana::unpack(output_ref_list, [](auto ... output_ref){
				return std::vector< std::reference_wrapper< output_base > >{
					std::reference_wrapper< output_base >(output_ref) ... };
			});
		}
	};


	/// \brief Accessory of a module during enable/disable calls
	template < typename IOP_List >
	class module_accessory
		: public module_config< IOP_List >
		, public add_log< module_accessory< IOP_List > >
	{
	public:
		/// \brief Constructor
		template < typename MakerList, typename MakeData >
		module_accessory(
			module_base& module,
			MakerList const& maker_list,
			MakeData const& data,
			std::string_view location
		)
			: module_config< IOP_List >(
					maker_list, data, location, std::make_index_sequence<
						decltype(hana::size(maker_list))::value >()
				)
			, module_(module) {}


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			os << "chain(" << module_.chain << ") module(" << module_.number
				<< ":" << module_.type_name << "): ";
		}


	protected:
		module_base& module_;
	};


	/// \brief Accessory of a module during exec calls
	template < typename IOP_List >
	class module_exec_accessory
		: public module_accessory< IOP_List >
		, public add_log< module_exec_accessory< IOP_List > >
	{
	private:
		using log_class =
			add_log< module_exec_accessory< IOP_List > >;

	public:
		using module_accessory< IOP_List >::module_accessory;

		using log_class::log;
		using log_class::exception_catching_log;

		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			os << "id(" << this->module_.id << ") chain("
				<< this->module_.chain << ") module(" << this->module_.number
				<< ":" << this->module_.type_name << ") exec: ";
		}
	};


	/// \brief Transfrom a \ref module_accessory to a \ref module_exec_accessory
	template < typename Accessory >
	struct to_exec_accessory;

	/// \brief Transfrom a \ref module_accessory to a \ref module_exec_accessory
	template < typename IOP_List >
	struct to_exec_accessory< module_accessory< IOP_List > >{
		using type = module_exec_accessory< IOP_List >;
	};

	/// \brief Transfrom a \ref module_accessory to a \ref module_exec_accessory
	template < typename Accessory >
	using to_exec_accessory_t = typename to_exec_accessory< Accessory >::type;


	/// \brief Wrapper for the module enable function
	template < typename ModuleEnableFn >
	class module_enable{
	public:
		module_enable(ModuleEnableFn&& module_fn)
			: module_fn_(static_cast< ModuleEnableFn&& >(module_fn)) {}

		template < typename Accessory >
		auto operator()(Accessory const& accessory)const{
// TODO: remove result_of-version as soon as libc++ supports invoke_result_t
#if __clang__
			if constexpr(std::is_callable_v<
				ModuleEnableFn const(Accessory const&) >
			){
				return module_fn_(accessory);
			}else if constexpr(std::is_callable_v< ModuleEnableFn const() >){
				return module_fn_();
			}else{
				static_assert(false_c< Accessory >,
					"module_enable function must be invokable with module& or "
					"without an argument");
			}
#else
			if constexpr(
				std::is_invocable_v< ModuleEnableFn const, Accessory const& >
			){
				return module_fn_(accessory);
			}else if constexpr(std::is_invocable_v< ModuleEnableFn const >){
				(void)accessory; // silance GCC
				return module_fn_();
			}else{
				static_assert(false_c< Accessory >,
					"module_enable function must be invokable with module& or "
					"without an argument");
			}
#endif
		}

	private:
		ModuleEnableFn const module_fn_;
	};

	/// \brief Wrapper for the module enable function
	template < typename ModuleExecFn >
	class module_exec{
	public:
		module_exec(ModuleExecFn&& module_fn)
			: module_fn_(static_cast< ModuleExecFn&& >(module_fn)) {}

		template < typename Accessory >
		auto operator()(Accessory& accessory, std::size_t id){
// TODO: remove result_of-version as soon as libc++ supports invoke_result_t
#if __clang__
			if constexpr(
				std::is_callable_v< ModuleExecFn(Accessory&, std::size_t) >
			){
				return module_fn_(accessory, id);
			}else if constexpr(
				std::is_callable_v< ModuleExecFn(Accessory&) >
			){
				return module_fn_(accessory);
			}else if constexpr(
				std::is_callable_v< ModuleExecFn(std::size_t) >
			){
				return module_fn_(id);
			}else if constexpr(std::is_callable_v< ModuleExecFn() >){
				return module_fn_();
			}else{
				static_assert(false_c< Accessory >,
					"exec_fn is not invokable with module& and/or id or "
					"without arguments");
			}
#else
			(void)id; // silance GCC
			(void)accessory; // silance GCC
			if constexpr(
				std::is_invocable_v< ModuleExecFn, Accessory&, std::size_t >
			){
				return module_fn_(accessory, id);
			}else if constexpr(
				std::is_invocable_v< ModuleExecFn, Accessory& >
			){
				return module_fn_(accessory);
			}else if constexpr(
				std::is_invocable_v< ModuleExecFn, std::size_t >
			){
				return module_fn_(id);
			}else if constexpr(std::is_invocable_v< ModuleExecFn >){
				return module_fn_();
			}else{
				static_assert(false_c< Accessory >,
					"exec_fn is not invokable with module& and/or id or "
					"without arguments");
			}
#endif
		}

	private:
		ModuleExecFn module_fn_;
	};


	/// \brief The actual module type
	template < typename IOP_List, typename EnableFn >
	class module: public module_base{
	public:
		/// \brief Type for enable_fn
		using accessory_type = module_accessory< IOP_List >;

		/// \brief Type for exec_fn
		using exec_accessory_type = module_exec_accessory< IOP_List >;


// TODO: remove result_of-version as soon as libc++ supports invoke_result_t
#if __clang__
		static_assert(std::is_callable_v<
			module_enable< EnableFn >(accessory_type const&) >,
			"enable_fn is not invokable with const module&");

		using exec_fn_t = module_exec< std::result_of_t<
			module_enable< EnableFn >(accessory_type const&) > >;
#else
		static_assert(std::is_invocable_v<
			module_enable< EnableFn >, accessory_type const& >,
			"enable_fn is not invokable with const module&");

		using exec_fn_t = module_exec< std::invoke_result_t<
			module_enable< EnableFn >, accessory_type const& > >;
#endif


		/// \brief Constructor
		template < typename MakerList, typename MakeData >
		module(
			MakerList const& maker_list,
			MakeData const& data,
			std::string_view location,
			module_enable< EnableFn > const& enable_fn
		)
			: module_base(
					data.type_name, data.chain, data.number,
					// accessory_ can be referenced before initialization
					generate_input_list(accessory_),
					generate_output_list(accessory_)
				)
			, accessory_(*this, maker_list, data, location)
			, enable_fn_(enable_fn) {}


	private:
		/// \brief Enables the module for exec calls
		virtual void enable()override{
			exec_fn_.emplace(enable_fn_(
				static_cast< accessory_type const& >(accessory_)));
		}

		/// \brief Disables the module for exec calls
		virtual void disable()noexcept override{
			exec_fn_.reset();
		}


		/// \brief The actual worker function called one times per trigger
		virtual void exec()override{
			if(exec_fn_){
				(*exec_fn_)(accessory_, id);
			}else{
				throw std::logic_error("module is not enabled");
			}
		}


		/// \brief Module access object for exec_fn_
		exec_accessory_type accessory_;

		/// \brief The function object that is called in enable()
		module_enable< EnableFn > enable_fn_;

		/// \brief The function object that is called in exec()
		std::optional< exec_fn_t > exec_fn_;
	};


	/// \brief Maker function for \ref module in a std::unique_ptr
	template < typename MakerList, typename MakeData, typename EnableFn >
	auto make_module_ptr(
		MakerList const& maker_list,
		MakeData const& data,
		std::string_view location,
		module_enable< EnableFn > const& enable_fn
	){
		auto list_type = hana::unpack(maker_list, [](auto const& ... maker){
			return hana::type_c< decltype(hana::make_tuple(
					std::declval< typename
						decltype(hana::typeid_(maker))::type::type >() ...
				)) >;
		});

		using iop_list_type = typename decltype(list_type)::type;

		return std::make_unique< module< iop_list_type, EnableFn > >(
			maker_list, data, location, enable_fn);
	}


	/// \brief Provids types for constructing an module
	template <
		typename IOP_MakerList,
		typename EnableFn >
	struct module_maker{
		/// \brief Tuple of input/output/parameter-maker objects
		IOP_MakerList makers;

		/// \brief The function object that is called in enable()
		module_enable< EnableFn > enable_fn;


		/// \brief Create an module object
		auto operator()(module_make_data const& data)const{
			// Check config file data for undefined inputs, outputs and
			// parameters, warn about parameters, throw for inputs and outputs
			auto const location = data.location();
			{
				auto inputs = invalid_inputs(location, makers, data.inputs);
				auto outputs = invalid_outputs(location, makers, data.outputs);
				check_parameters(location, makers, data.parameters);

				if(!inputs.empty() || !outputs.empty()){
					throw std::logic_error(location + "some inputs or "
						"outputs don't exist, see previos log messages for "
						"more details");
				}
			}

			std::string const basic_location = data.basic_location();

			// Create the module
			return make_module_ptr(makers, data, basic_location, enable_fn);
		}
	};


	/// \brief Wraps all given IOP configurations into a hana::tuple
	template < typename ... IOP_Makers >
	constexpr auto module_configure(IOP_Makers&& ... list){
		static_assert(hana::and_(hana::true_c, hana::or_(
			hana::is_a< input_maker_tag, IOP_Makers >(),
			hana::is_a< output_maker_tag, IOP_Makers >(),
			hana::is_a< parameter_maker_tag, IOP_Makers >()) ...),
			"at least one of the module configure arguments is not a disposer "
			"input, output or parameter maker");

		return hana::make_tuple(static_cast< IOP_Makers&& >(list) ...);
	}

	struct unit_test_key;

	/// \brief Hana Tag for \ref module_register_fn
	struct module_register_fn_tag{};

	/// \brief Registers a module configuration in the \ref disposer
	template <
		typename IOP_MakerList,
		typename EnableFn >
	class module_register_fn{
	public:
		/// \brief Hana tag to identify module register functions
		using hana_tag = module_register_fn_tag;

		/// \brief Constructor
		constexpr module_register_fn(
			IOP_MakerList&& list,
			module_enable< EnableFn >&& enable_fn
		)
			: called_flag_(false)
			, maker_{
				static_cast< IOP_MakerList&& >(list),
				std::move(enable_fn)
			}
			{}

		/// \brief Call this function to register the module with the given type
		///        name via the given module_declarant
		void operator()(std::string const& module_type, module_declarant& add){
			if(!called_flag_.exchange(true)){
				add(module_type,
					[maker{std::move(maker_)}](module_make_data const& data){
						return maker(data);
					});
			}else{
				throw std::runtime_error("called module register function '"
					+ module_type + "' more than once");
			}
		}


	private:
		/// \brief Operator must only called once!
		std::atomic< bool > called_flag_;

		/// \brief The module_maker object
		module_maker< IOP_MakerList, EnableFn > maker_;

		friend struct unit_test_key;
	};


}


#endif
