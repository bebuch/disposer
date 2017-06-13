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


	/// \brief Accessory of a \ref module without log
	template <
		typename Inputs,
		typename Outputs,
		typename Parameters >
	class module_config{
	public:
		/// \brief Constructor
		module_config(
			Inputs&& inputs,
			Outputs&& outputs,
			Parameters&& parameters
		):
			inputs_(std::move(inputs)),
			outputs_(std::move(outputs)),
			parameters_(std::move(parameters)) {}


		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename ConfigList >
		decltype(auto) operator()(ConfigList&& list)noexcept{
			return get(*this, static_cast< ConfigList&& >(list));
		}

		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename ConfigList >
		decltype(auto) operator()(ConfigList&& list)const noexcept{
			return get(*this, static_cast< ConfigList&& >(list));
		}


	private:
		/// \brief Implementation for \ref operator()
		template < typename Config, typename ConfigList >
		static decltype(auto) get(Config& config, ConfigList&&)noexcept{
			using io_t =
				std::remove_cv_t< std::remove_reference_t< ConfigList > >;
			static_assert(
				hana::is_a< input_name_tag, io_t > ||
				hana::is_a< output_name_tag, io_t > ||
				hana::is_a< parameter_name_tag, io_t >,
				"parameter io must be an input_name, an output_name or a"
				"parameter_name");

			if constexpr(hana::is_a< input_name_tag, io_t >){
				static_assert(have_input(io_t::value),
					"input doesn't exist");
				return config.inputs_[io_t::value];
			}else if constexpr(hana::is_a< output_name_tag, io_t >){
				static_assert(have_output(io_t::value),
					"output doesn't exist");
				return config.outputs_[io_t::value];
			}else{
				static_assert(have_parameter(io_t::value),
					"parameter doesn't exist");
				return config.parameters_[io_t::value];
			}
		}


		/// \brief hana::true_ if input_name exists
		template < typename String >
		constexpr static auto have_input(String){
			return
				hana::contains(decltype(hana::keys(inputs_))(), String());
		}

		/// \brief hana::true_ if output_name exists
		template < typename String >
		constexpr static auto have_output(String){
			return
				hana::contains(decltype(hana::keys(outputs_))(), String());
		}

		/// \brief hana::true_ if parameter_name exists
		template < typename String >
		constexpr static auto have_parameter(String){
			return
				hana::contains(decltype(hana::keys(parameters_))(), String());
		}


		/// \brief hana::map from input_name's to input's
		Inputs inputs_;

		/// \brief hana::map from output_name's to output's
		Outputs outputs_;

		/// \brief hana::map from parameter_name's to parameter's
		Parameters parameters_;


		/// \brief std::vector with references to all input's (input_base)
		friend auto generate_input_list(module_config& config){
			return hana::unpack(config.inputs_, [](auto& ... input){
				return std::vector< std::reference_wrapper< input_base > >{
					std::ref(static_cast< input_base& >(hana::second(input)))
					... };
			});
		}

		/// \brief std::vector with references to all output's (output_base)
		friend auto generate_output_list(module_config& config){
			return hana::unpack(config.outputs_, [](auto& ... output){
				return std::vector< std::reference_wrapper< output_base > >{
					std::ref(static_cast< output_base& >(hana::second(output)))
					... };
			});
		}
	};


	/// \brief Maker of \ref module_config
	template <
		typename Inputs,
		typename Outputs,
		typename Parameters >
	auto make_module_config(
		Inputs&& inputs,
		Outputs&& outputs,
		Parameters&& parameters
	){
		return module_config<
				std::remove_const_t< std::remove_reference_t< Inputs > >,
				std::remove_const_t< std::remove_reference_t< Outputs > >,
				std::remove_const_t< std::remove_reference_t< Parameters > >
			>(
				static_cast< Inputs&& >(inputs),
				static_cast< Outputs&& >(outputs),
				static_cast< Parameters&& >(parameters)
			);
	}


	/// \brief Accessory of a module during enable/disable calls
	template <
		typename Inputs,
		typename Outputs,
		typename Parameters >
	class module_accessory
		: public module_config< Inputs, Outputs, Parameters >
		, public add_log< module_accessory< Inputs, Outputs, Parameters > >
	{
	public:
		/// \brief Constructor
		module_accessory(
			module_base& module,
			Inputs&& inputs,
			Outputs&& outputs,
			Parameters&& parameters
		)
			: module_config< Inputs, Outputs, Parameters >(
					std::move(inputs),
					std::move(outputs),
					std::move(parameters)
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
	template <
		typename Inputs,
		typename Outputs,
		typename Parameters >
	class module_exec_accessory
		: public module_accessory< Inputs, Outputs, Parameters >
		, public add_log< module_exec_accessory< Inputs, Outputs, Parameters > >
	{
	private:
		using log_class =
			add_log< module_exec_accessory< Inputs, Outputs, Parameters > >;

	public:
		using module_accessory< Inputs, Outputs, Parameters >::module_accessory;

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
	template < typename Inputs, typename Outputs, typename Parameters >
	struct to_exec_accessory< module_accessory< Inputs, Outputs, Parameters > >{
		using type = module_exec_accessory< Inputs, Outputs, Parameters >;
	};

	/// \brief Transfrom a \ref module_accessory to a \ref module_exec_accessory
	template < typename Accessory >
	using to_exec_accessory_t = typename to_exec_accessory< Accessory >::type;


	/// \brief The actual module type
	template <
		typename Inputs,
		typename Outputs,
		typename Parameters,
		typename EnableFn >
	class module: public module_base{
	public:
		/// \brief Type for exec_fn
		using accessory_type =
			module_accessory< Inputs, Outputs, Parameters >;

		/// \brief Type for exec_fn
		using exec_accessory_type =
			module_exec_accessory< Inputs, Outputs, Parameters >;


// TODO: remove result_of-version as soon as libc++ supports invoke_result_t
#if __clang__
			static_assert(std::is_callable_v<
				EnableFn(accessory_type const&) >);
#else
			static_assert(std::is_invocable_v<
				EnableFn, accessory_type const& >);
#endif


		/// \brief Constructor
		module(
			std::string const& module_type,
			std::string const& chain,
			std::size_t number,
			id_increase_t id_increase,
			Inputs&& inputs,
			Outputs&& outputs,
			Parameters&& parameters,
			EnableFn const& enable_fn
		)
			: module_base(
					module_type, chain, number, id_increase,
					// these two functions can be called prior initialzation
					generate_input_list(accessory_),
					generate_output_list(accessory_)
				)
			, enable_fn_(enable_fn)
			, accessory_(*this,
					std::move(inputs),
					std::move(outputs),
					std::move(parameters)
				) {}


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


		/// \brief Type of the exec-function
// TODO: remove result_of-version as soon as libc++ supports invoke_result_t
#if __clang__
		using exec_fn_t = std::result_of_t<
			EnableFn(accessory_type const&) >;

		static_assert(std::is_callable_v<
			exec_fn_t(exec_accessory_type&, std::size_t) >);
#else
		using exec_fn_t = std::invoke_result_t<
			EnableFn, accessory_type const& >;

		static_assert(std::is_invocable_v<
			exec_fn_t, exec_accessory_type&, std::size_t >);
#endif


		/// \brief The function object that is called in enable()
		EnableFn enable_fn_;

		/// \brief The function object that is called in exec()
		std::optional< exec_fn_t > exec_fn_;

		/// \brief Module access object for exec_fn_
		exec_accessory_type accessory_;
	};


	/// \brief Maker function for \ref module in a std::unique_ptr
	template <
		typename Inputs,
		typename Outputs,
		typename Parameters,
		typename EnableFn >
	auto make_module_ptr(
		std::string const& type_name,
		std::string const& chain,
		std::size_t number,
		id_increase_t id_increase,
		Inputs&& inputs,
		Outputs&& outputs,
		Parameters&& parameters,
		EnableFn&& enable_fn
	){
		return std::make_unique< module<
				std::remove_const_t< std::remove_reference_t< Inputs > >,
				std::remove_const_t< std::remove_reference_t< Outputs > >,
				std::remove_const_t< std::remove_reference_t< Parameters > >,
				std::remove_const_t< std::remove_reference_t< EnableFn > >
			> >(
				type_name, chain, number, id_increase,
				static_cast< Inputs&& >(inputs),
				static_cast< Outputs&& >(outputs),
				static_cast< Parameters&& >(parameters),
				static_cast< EnableFn&& >(enable_fn)
			);
	}


	/// \brief Provids types for constructing an module
	template <
		typename IOP_MakerList,
		typename IdIncreaseFn,
		typename EnableFn >
	struct module_maker{
		/// \brief Tuple of input/output/parameter-maker objects
		IOP_MakerList makers;

		/// \brief Must return a std::size_t > 0
		id_increase_fn< IdIncreaseFn > id_increase;

		/// \brief The function object that is called in enable()
		EnableFn enable_fn;


		/// \brief Create an module object
		auto operator()(module_make_data const& data)const{
			// Check config file data for undefined inputs, outputs and
			// parameters, warn about parameters, throw for inputs and outputs
			auto input_names = hana::transform(
				hana::filter(makers, hana::is_a< input_maker_tag >),
				[](auto const& input_maker){
					return input_maker.name;
				});

			std::set< std::string > input_name_list;
			std::transform(data.inputs.begin(), data.inputs.end(),
				std::inserter(input_name_list, input_name_list.end()),
				[](auto const& pair){ return pair.first; });
			hana::for_each(input_names,
				[&input_name_list](auto const& name){
					input_name_list.erase(name.c_str());
				});

			auto const location = data.location();
			for(auto const& in: input_name_list){
				logsys::log([&location, &in](logsys::stdlogb& os){
					os << location << "input("
						<< in << ") doesn't exist (ERROR)";
				});
			}


			auto output_names = hana::transform(
				hana::filter(makers, hana::is_a< output_maker_tag >),
				[](auto const& output_maker){
					return output_maker.name;
				});

			std::set< std::string > output_name_list = data.outputs;
			hana::for_each(output_names,
				[&output_name_list](auto const& name){
					output_name_list.erase(name.c_str());
				});

			for(auto const& out: output_name_list){
				logsys::log([&location, &out](logsys::stdlogb& os){
					os << location << "output("
						<< out << ") doesn't exist (ERROR)";
				});
			}


			auto parameters_names = hana::transform(
				hana::filter(makers, hana::is_a< parameter_maker_tag >),
				[](auto const& parameters_maker){
					return parameters_maker.name;
				});

			std::set< std::string > parameter_name_list;
			std::transform(data.parameters.begin(), data.parameters.end(),
				std::inserter(parameter_name_list, parameter_name_list.end()),
				[](auto const& pair){ return pair.first; });
			hana::for_each(parameters_names,
				[&parameter_name_list](auto const& name){
					parameter_name_list.erase(name.c_str());
				});

			for(auto const& param: parameter_name_list){
				logsys::log([&location, &param](logsys::stdlogb& os){
					os << location << "parameter("
						<< param << ") doesn't exist (WARNING)";
				});
			}


			if(!input_name_list.empty() || !output_name_list.empty()){
				throw std::logic_error(location + "some inputs or "
					"outputs don't exist, see previos log messages for more "
					"details");
			}

			std::string const basic_location = data.basic_location();

			// create inputs, outputs and parameter in the order of there
			// definition in the module
			auto iop_list = hana::fold_left(makers, hana::make_tuple(),
				[&data, &location, &basic_location](auto&& get, auto&& maker){
					auto is_input =
						hana::is_a< input_maker_tag >(maker);
					auto is_output =
						hana::is_a< output_maker_tag >(maker);
					auto is_parameter =
						hana::is_a< parameter_maker_tag >(maker);

					static_assert(is_input || is_output || is_parameter,
						"maker is not an iop (this is a bug in disposer!)");

					if constexpr(is_input){
						auto iter = data.inputs.find(maker.name.c_str());
						auto output = iter == data.inputs.end()
							? static_cast< output_base* >(nullptr)
							: std::get< 0 >(iter->second);

						auto last_use = output
							? std::get< 1 >(iter->second)
							: true;

						auto info = output
							? std::optional< output_info >(
								output->enabled_types())
							: std::optional< output_info >();

						return hana::append(
							static_cast< decltype(get)&& >(get),
							maker(make_iop_list(iop_log{basic_location,
								"input", maker.name.c_str()}, get),
								output, last_use, info));
					}else if constexpr(is_output){
						return hana::append(
							static_cast< decltype(get)&& >(get),
							maker(make_iop_list(iop_log{basic_location,
								"output", maker.name.c_str()}, get)));
					}else{
						auto const name = maker.name.c_str();
						auto const iter = data.parameters.find(name);
						auto const found = iter != data.parameters.end();

						bool all_specialized = true;

						auto get_value =
							[&location, &all_specialized, &maker, found, name,
								iter]
							(auto type) -> std::optional< std::string_view >
						{
							if(!found) return {};

							auto const specialization = iter->second
								.specialized_values.find(
									maker.to_text[type].c_str());
							auto const end =
								iter->second.specialized_values.end();
							if(specialization == end){
								all_specialized = false;
								if(!iter->second.generic_value){
									throw std::logic_error(
										location + "parameter("
										+ std::string(name) + ") has neither a "
										"generic value but a specialization "
										"for type '" + specialization->first
										+ "'"
									);
								}else{
									return {*iter->second.generic_value};
								}
							}else{
								return {specialization->second};
							}
						};

						auto params = hana::to_map(hana::transform(
							maker.types,
							[&get_value](auto&& type){
								return hana::make_pair(type, get_value(type));
							}));

						if(found && all_specialized
							&& iter->second.generic_value
						){
							logsys::log([&location, name](logsys::stdlogb& os){
								os << location << "parameter("
									<< name << ") has specialized values for "
									"all its types, the also given generic "
									"value will never be used (WARNING)";
							});
						}

						return hana::append(
							static_cast< decltype(get)&& >(get),
							maker(make_iop_list(iop_log{basic_location,
								"parameter", maker.name.c_str()}, get),
								std::move(params)));
					}
				}
			);

			auto const id_increase_value = id_increase(
				make_iop_list(iop_log{basic_location,
					"id increase", ""}, iop_list));

			// A helper function
			auto as_map = [](auto&& xs){
				return hana::to_map(hana::transform(
					static_cast< decltype(xs)&& >(xs),
					[](auto&& x){
						return hana::make_pair(x.name, std::move(x));
					}));
			};

			auto inputs = as_map(hana::filter(
				std::move(iop_list), hana::is_a< input_tag >));
			auto outputs = as_map(hana::filter(
				std::move(iop_list), hana::is_a< output_tag >));
			auto parameters = as_map(hana::filter(
				std::move(iop_list), hana::is_a< parameter_tag >));

			// Create the module
			return make_module_ptr(
					data.type_name, data.chain, data.number,
					id_increase_value,
					std::move(inputs),
					std::move(outputs),
					std::move(parameters),
					enable_fn
				);
		}
	};


	/// \brief Wraps all given IOP configurations into a hana::tuple
	template < typename ... IOP_Makers >
	constexpr auto configure(IOP_Makers&& ... list){
		static_assert(hana::and_(hana::true_c, hana::or_(
			hana::is_a< input_maker_tag, IOP_Makers >(),
			hana::is_a< output_maker_tag, IOP_Makers >(),
			hana::is_a< parameter_maker_tag, IOP_Makers >()) ...),
			"at least one of the configure arguments is not a disposer input, "
			"output or parameter maker");

		return hana::make_tuple(static_cast< IOP_Makers&& >(list) ...);
	}

	struct unit_test_key;

	/// \brief Registers a module configuration in the \ref disposer
	template <
		typename IOP_MakerList,
		typename IdIncreaseFn,
		typename EnableFn >
	class register_fn{
	public:
		/// \brief Constructor
		template <
			typename IOP_MakerListParam,
			typename IdIncreaseFnParam,
			typename EnableFnParam >
		constexpr register_fn(
			IOP_MakerListParam&& list,
			IdIncreaseFnParam&& id_increase,
			EnableFnParam&& enable_fn
		)
			: called_flag_(false)
			, maker_{
				static_cast< IOP_MakerListParam&& >(list),
				static_cast< IdIncreaseFnParam&& >(id_increase),
				static_cast< EnableFnParam&& >(enable_fn)
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
				throw std::runtime_error("called register function '"
					+ module_type + "' more than once");
			}

		}


	private:
		/// \brief Operator must only called once!
		std::atomic< bool > called_flag_;

		/// \brief The module_maker object
		module_maker< IOP_MakerList, IdIncreaseFn, EnableFn > maker_;

		friend struct unit_test_key;
	};


	/// \brief Maker function for \ref register_fn
	template <
		typename IOP_MakerList,
		typename IdIncreaseFn,
		typename EnableFn >
	constexpr auto make_register_fn(
		IOP_MakerList&& list,
		IdIncreaseFn&& id_increase,
		EnableFn&& enable_fn
	){
		return register_fn<
				std::remove_reference_t< IOP_MakerList >,
				std::remove_reference_t< IdIncreaseFn >,
				std::remove_reference_t< EnableFn >
			>(
				static_cast< IOP_MakerList&& >(list),
				static_cast< IdIncreaseFn&& >(id_increase),
				static_cast< EnableFn&& >(enable_fn)
			);
	}


}


#endif
