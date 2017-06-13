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


namespace disposer{ namespace{


		auto create_chains(
			module_maker_list const& maker_list,
			types::merge::chains_config&& config
		){
			std::unordered_map< std::string, chain > chains;
			std::unordered_map< std::string, id_generator > id_generators;

			for(auto& config_chain: config){
				logsys::log([&config_chain](logsys::stdlogb& os){
					os << "chain(" << config_chain.name << ") create";
				}, [&](){
					// emplace the new process chain
					chains.emplace(
						std::piecewise_construct,
						std::forward_as_tuple(
							config_chain.name
						),
						std::forward_as_tuple(
							maker_list,
							config_chain,
							id_generators[config_chain.id_generator]
						)
					);
				});
			}

			return std::make_tuple(
				std::move(chains),
				std::move(id_generators)
			);
		}


}}


namespace disposer{


	disposer::disposer():
		declarant_(*this) {}

	void module_declarant::operator()(
		std::string const& type_name,
		module_maker_fn&& fn
	){
		logsys::log([&type_name](logsys::stdlogb& os){
			os << "register module type name '" << type_name << "'";
		}, [&]{
			auto iter = disposer_.maker_list_.insert(
				std::make_pair(type_name, std::move(fn))
			);

			if(!iter.second){
				throw std::logic_error(
					"module type name '" + type_name + "' is double registered!"
				);
			}
		});
	}


	module_declarant& disposer::declarant(){
		return declarant_;
	}

	void disposer::load(std::string const& filename){
		auto config = logsys::log([&](logsys::stdlogb& os){
				os << "parse '" << filename << "'";
			}, [&](){ return parse(filename); });

		logsys::log([](logsys::stdlogb& os){ os << "check semantic"; },
			[&config](){ check_semantic(config); });

		logsys::log([](logsys::stdlogb& os){
				os << "look for unused stuff and warn about it";
			}, [&config](){ unused_warnings(config); });

		auto merged_config = logsys::log(
			[](logsys::stdlogb& os){ os << "merge"; },
			[&config](){ return merge(std::move(config)); });

		logsys::log([](logsys::stdlogb& os){ os << "create chains"; },
			[this, &merged_config](){
				std::tie(chains_, id_generators_) =
					create_chains(maker_list_, std::move(merged_config.chains));
			});
	}

	chain& disposer::get_chain(std::string const& chain){
		auto iter = chains_.find(chain);
		if(iter == chains_.end()){
			throw std::logic_error("chain(" + chain + ") does not exist");
		}
		return iter->second;
	}

	std::unordered_set< std::string > disposer::chains()const{
		std::unordered_set< std::string > result;
		for(auto& chain: chains_) result.emplace(chain.first);
		return result;
	}


}
