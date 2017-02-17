//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
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


namespace disposer{


	namespace{


		auto create_chains(
			module_maker_list const& maker_list,
			types::merge::config&& config
		){
			std::unordered_map< std::string, chain > chains;
			std::unordered_map< std::string, id_generator > id_generators;
			std::unordered_map<
				std::string, std::vector< std::reference_wrapper< chain > >
			> groups;

			for(auto& config_chain: config.chains){
				log([&config_chain](log_base& os){
					os << "create chain '" << config_chain.name << "'";
				}, [&](){
					// add the new group if not exist and get pointer
					auto group_iter = groups.emplace(
						std::piecewise_construct,
						std::make_tuple(config_chain.group),
						std::make_tuple()
					).first;

					// emplace the new process chain
					auto chain_iter = chains.emplace(
						std::piecewise_construct,
						std::forward_as_tuple(
							config_chain.name
						),
						std::forward_as_tuple(
							maker_list,
							config_chain,
							id_generators[config_chain.id_generator],
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


	}


	disposer::disposer():
		declarant_(*this) {}

	void module_declarant::operator()(
		std::string const& type_name,
		module_maker_function&& function
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


	module_declarant& disposer::declarant(){
		return declarant_;
	}

	void disposer::load(std::string const& filename){
		auto config = log([&](log_base& os){
				os << "parse '" << filename << "'";
			}, [&](){ return parse(filename); });

		log([](log_base& os){ os << "check semantic"; },
			[&config](){ check_semantic(config); });

		log([](log_base& os){
			os << "look for unused stuff and warn about it";
			}, [&config](){ unused_warnings(config); });

		auto merged_config = log([](log_base& os){ os << "merge"; },
			[&config](){ return merge(std::move(config)); });

		log([](log_base& os){ os << "create chains"; },
			[this, &merged_config](){
				std::tie(chains_, id_generators_, groups_) =
					create_chains(maker_list_, std::move(merged_config));
			});
	}

	void disposer::exec(std::string const& chain){
		auto iter = chains_.find(chain);
		if(iter == chains_.end()){
			throw std::logic_error(
				"executed chain '" + chain + "' does not exist"
			);
		}
		iter->second.exec();
	}

	std::unordered_set< std::string > disposer::chains()const{
		std::unordered_set< std::string > result;
		for(auto& chain: chains_) result.emplace(chain.first);
		return result;
	}

	std::vector< std::string > disposer::chains(
		std::string const& group
	)const{
		auto iter = groups_.find(group);
		if(iter == groups_.end()) return {};

		std::vector< std::string > result;
		result.reserve(iter->second.size());
		for(auto& chain: iter->second) result.emplace_back(chain.get().name);
		return result;
	}

	std::unordered_set< std::string > disposer::groups()const{
		std::unordered_set< std::string > result;
		for(auto& group: groups_) result.emplace(group.first);
		return result;
	}


}
