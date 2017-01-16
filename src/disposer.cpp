//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/disposer.hpp>
#include <disposer/check_semantic.hpp>
#include <disposer/unused_warnings.hpp>
#include <disposer/merge.hpp>
#include <disposer/make_data.hpp>
#include <disposer/module_base.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <cassert>


disposer::disposer::disposer():
	adder_(*this) {}

void disposer::module_adder::operator()(
	std::string const& type_name,
	maker_function&& function
){
	log([&type_name](log_base& os){
		os << "register module type name '" << type_name << "'";
	}, [&]{
		auto iter = disposer_.maker_list_.insert(
			std::make_pair(type_name, std::move(function))
		);

		if(!iter.second){
			throw std::logic_error(
				"Module type name '" + type_name + "' is double registered!"
			);
		}
	});
}


disposer::module_adder& disposer::disposer::adder(){
	return adder_;
}


class disposer::disposer::impl{
public:
	std::unordered_map< std::string, disposer::maker_function >& maker_list;

	// variable name = { output&, is_last_use? }
	using variables_map =
		std::map< std::string, std::pair< output_base&, bool > >;

	module_ptr make_module(make_data&& data);

	auto create_modules(
		types::merge::chain const& config_chain,
		variables_map& variables
	);

	void activate_output_types(
		types::merge::chain const& config_chain,
		std::vector< module_ptr > const& modules,
		variables_map const& variables
	);

	void connect_modules(
		types::merge::chain const& config_chain,
		std::vector< module_ptr > const& modules,
		variables_map& variables
	);

	auto create_chains(types::merge::config&& config);

	template < typename T >
	static auto find(T& container, std::string const& data){
		for(auto& value: container){
			if(value.get().name == data) return value;
		}
		assert(false);
		std::exit(1);
	}
};


disposer::module_ptr disposer::disposer::impl::make_module(make_data&& data){
	auto iter = maker_list.find(data.type_name);

	if(iter == maker_list.end()){
		throw std::logic_error(
			"Module '" + data.chain + "." + data.name + "': " + "Type '" +
			data.type_name + "' is unknown!"
		);
	}

	try{
		auto result = iter->second(data);
		for(auto const& param: data.params.unused()){
			log([&data, &param](log_base& os){
				os << "In chain '" << data.chain << "' module '"
					<< data.name << "': Unused data '" << param.first
					<< "'='" << param.second << "'"; });
		}
		return result;
	}catch(std::exception const& error){
		throw std::runtime_error(
			"Module '" + data.chain + "." + data.name + "': " + error.what()
		);
	}
}

auto disposer::disposer::impl::create_modules(
	types::merge::chain const& config_chain,
	variables_map& variables
){
	std::vector< module_ptr > modules;

	for(std::size_t i = 0; i < config_chain.modules.size(); ++i){
		auto& config_module = config_chain.modules[i];

		log([&config_module](log_base& os){
			os << "create module '" << config_module.module.first << "'";
		}, [&](){
			io_list config_inputs;
			for(auto& config_input: config_module.inputs){
				config_inputs.emplace(config_input.name);
			}

			io_list config_outputs;
			for(auto& config_output: config_module.outputs){
				config_outputs.emplace(config_output.name);
			}

			modules.push_back(make_module({
				config_module.module.second.type_name,
				config_chain.name,
				config_module.module.first,
				i,
				std::move(config_inputs),
				std::move(config_outputs),
				config_module.module.second.parameters
			}));

			auto& module = *modules.back();

			// save output variable names
			for(auto& config_output: config_module.outputs){
				auto& output = find(module.outputs_, config_output.name).get();

				variables.emplace(
					config_output.variable,
					std::pair< output_base&, bool >(output, true)
				);
			}
		});
	}

	return modules;
}

void disposer::disposer::impl::activate_output_types(
	types::merge::chain const& config_chain,
	std::vector< module_ptr > const& modules,
	variables_map const& variables
){
	// go through all modules and activate output types in the target inputs
	auto module_ptr_iter = modules.begin();
	for(auto& config_module: config_chain.modules){
		auto& module = **module_ptr_iter++;

		log([&config_module](log_base& os){
			os << "activate input and output types in module '"
				<< config_module.module.first << "'";
		}, [&](){
			// config_module.inputs containes all active input
			// names
			for(auto& config_input: config_module.inputs){
				auto output_iter = variables.find(config_input.variable);
				assert(output_iter != variables.end());

				auto& output = output_iter->second.first;
				auto& input = find(module.inputs_, config_input.name).get();

				// try to activate the types from output in input
				if(!input.activate_types(output.active_types())){
					std::ostringstream os;
					os << "In chain '" << config_chain.name << "' module '"
						<< module.name << "': Variable '"
						<< config_input.variable
						<< "' is incompatible with input '"
						<< config_input.name << "'" << " (active '"
						<< config_input.variable << "' types: ";

					bool first = true;
					for(auto& type: output.active_types()){
						if(first){
							first = false;
						}else{
							os << ", ";
						}

						os << "'" << type.pretty_name() << "'";
					}

					os << "; possible '" << config_input.name << "' types: ";

					first = true;
					for(auto& type: input.types()){
						if(first){
							first = false;
						}else{
							os << ", ";
						}

						os << "'" << type.pretty_name() << "'";
					}

					os << ")";

					throw std::logic_error(os.str());
				}
			}

			module.input_ready();

			// config_module.outputs containes all active output names
			for(auto& config_output: config_module.outputs){
				auto output_iter = variables.find(config_output.variable);
				assert(output_iter != variables.end());

				auto& output = output_iter->second.first;

				if(output.active_types().empty()){
					std::ostringstream os;
					os << "In chain '" << config_chain.name << "' module '"
						<< module.name << "': Output '" + config_output.name
						<< "' (Variable: '" << config_output.variable
						<< "') has no active output types";

					throw std::logic_error(os.str());
				}
			}
		});
	}
}

void disposer::disposer::impl::connect_modules(
	types::merge::chain const& config_chain,
	std::vector< module_ptr > const& modules,
	variables_map& variables
){
	// go backward through all modules, because of the last_use information
	auto module_ptr_iter = modules.end();
	for(auto& config_module: boost::adaptors::reverse(config_chain.modules)){
		--module_ptr_iter;
		auto& module = **module_ptr_iter;

		log([&config_module](log_base& os){
			os << "connect inputs of module '" << config_module.module.first
				<< "'";
		}, [&](){
			// config_module.inputs containes all active input names
			for(auto& config_input: config_module.inputs){
				auto output_iter = variables.find(config_input.variable);
				assert(output_iter != variables.end());

				auto& output = output_iter->second.first;
				auto& last_use = output_iter->second.second;

				auto& input = find(module.inputs_, config_input.name).get();

				// connect input to output
				output.signal.connect(input, last_use);

				// the next one is no more the last use of the variable
				last_use = false;
			}
		});
	}
}

auto disposer::disposer::impl::create_chains(types::merge::config&& config){
	std::unordered_map< std::string, chain > chains;
	std::unordered_map< std::string, id_generator > id_generators;
	std::unordered_map<
		std::string, std::vector< std::reference_wrapper< chain > >
	> groups;

	for(auto& config_chain: config.chains){
		log([&config_chain](log_base& os){
			os << "create chain '" << config_chain.name << "'";
		}, [&](){
			variables_map variables;

			auto modules = create_modules(config_chain, variables);

			activate_output_types(config_chain, modules, variables);

			connect_modules(config_chain, modules, variables);

			// add the new group if not exist and get pointer
			auto group_iter = groups.emplace(
				std::piecewise_construct,
				std::make_tuple(config_chain.group),
				std::make_tuple()
			).first;

			// emplace the new process chain
			auto chain_iter = chains.emplace(
				std::piecewise_construct,
				std::make_tuple(config_chain.name),
				std::forward_as_tuple(
					std::move(modules),
					id_generators[config_chain.id_generator],
					config_chain.name,
					group_iter->first
				)
			).first;

			// add chain to group
			group_iter->second.push_back(chain_iter->second);
		});
	}

	return std::make_tuple(
		std::move(chains),
		std::move(id_generators),
		std::move(groups)
	);
}

void disposer::disposer::load(std::string const& filename){
	auto config = log([&](log_base& os){ os << "parse '" << filename << "'"; },
		[&](){
			return parse(filename);
		});

	log([](log_base& os){ os << "check semantic"; },
		[&config](){
			check_semantic(config);
		});

	log([](log_base& os){ os << "look for unused stuff and warn about it"; },
		[&config](){
			unused_warnings(config);
		});

	auto merged_config = log([](log_base& os){ os << "merge"; },
		[&config](){
			return merge(std::move(config));
		});

	log([](log_base& os){ os << "create chains"; },
		[this, &merged_config](){
			std::tie(chains_, id_generators_, groups_) =
				impl{maker_list_}.create_chains(std::move(merged_config));
		});
}

void disposer::disposer::trigger(std::string const& chain){
	auto iter = chains_.find(chain);
	if(iter == chains_.end()){
		throw std::logic_error(
			"triggered chain '" + chain + "' does not exist"
		);
	}
	iter->second.trigger();
}

std::unordered_set< std::string > disposer::disposer::chains()const{
	std::unordered_set< std::string > result;
	for(auto& chain: chains_) result.emplace(chain.first);
	return result;
}

std::vector< std::string > disposer::disposer::chains(
	std::string const& group
)const{
	auto iter = groups_.find(group);
	if(iter == groups_.end()) return {};

	std::vector< std::string > result;
	result.reserve(iter->second.size());
	for(auto& chain: iter->second) result.emplace_back(chain.get().name);
	return result;
}

std::unordered_set< std::string > disposer::disposer::groups()const{
	std::unordered_set< std::string > result;
	for(auto& group: groups_) result.emplace(group.first);
	return result;
}
