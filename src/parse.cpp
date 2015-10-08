//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/parse.hpp>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <fstream>
#include <string>
#include <complex>
#include <iostream>


BOOST_FUSION_ADAPT_STRUCT(
	disposer::types::parse::parameter,
	key,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	disposer::types::parse::parameter_set,
	name,
	parameters
)


BOOST_FUSION_ADAPT_STRUCT(
	disposer::types::parse::module,
	name,
	type_name,
	parameter_sets,
	parameters
)


BOOST_FUSION_ADAPT_STRUCT(
	disposer::types::parse::io,
	name,
	variable
)

BOOST_FUSION_ADAPT_STRUCT(
	disposer::types::parse::chain_module,
	name,
	inputs,
	outputs
)

BOOST_FUSION_ADAPT_STRUCT(
	disposer::types::parse::chain,
	name,
	group,
	id_generator,
	modules
)


BOOST_FUSION_ADAPT_STRUCT(
	disposer::types::parse::config,
	sets,
	modules,
	chains
)


namespace disposer{


	 namespace parser{


		namespace x3 = boost::spirit::x3;

		using x3::lit;
		using x3::char_;
		using x3::eol;
		using x3::eoi;


		x3::rule< class space > const space("space");
		x3::rule< class empty_lines > const empty_lines("empty_lines");
		x3::rule< class separator > const separator("separator");
		x3::rule< class comment > const comment("comment");
		x3::rule< class keyword_spaces, std::string > const keyword_spaces("keyword_spaces");
		x3::rule< class keyword, std::string > const keyword("keyword");
		x3::rule< class value_spaces, std::string > const value_spaces("value_spaces");
		x3::rule< class value, std::string > const value("value");
		x3::rule< class parameter, types::parse::parameter > const parameter("parameter");

		auto const space_def =
			lit(' ') | '\t'
		;

		auto const empty_lines_def =
			*(*space >> eol) >> -(*space >> eoi)
		;

		auto const separator_def =
			*space >> ((eol >> empty_lines >> -comment) | eoi)
		;

		auto const comment_def =
			*space >> (lit('#') | ';') >> *(char_ - eol) >> separator
		;

		auto const keyword_spaces_def =
			+(char_(' ') | char_('\t')) >> !(eol | eoi | '=')
		;

		auto const keyword_def =
			(char_ - space - '=' - eol) >>
			*(keyword_spaces | +(char_ - space - eol - '='))
		;

		auto const value_spaces_def =
			+(char_(' ') | char_('\t')) >> !(eol | eoi)
		;

		auto const value_def = 
			(char_ - space - eol) >>
			*(value_spaces | +(char_ - space - eol))
		;

		auto const parameter_def = 
			"\t\t" >> !("parameter_set" >> *space >> '=') >> keyword  >>
			*space >> '=' >> *space >> value >> separator
		;

		BOOST_SPIRIT_DEFINE(
			space,
			empty_lines,
			separator,
			comment,
			keyword_spaces,
			keyword,
			value_spaces,
			value,
			parameter
		)


		namespace set{


			x3::rule< class parameter_set, types::parse::parameter_set > const parameter_set("parameter_set");
			x3::rule< class parameter_sets, types::parse::parameter_sets > const parameter_sets("parameter_sets");

			auto const parameter_set_def = 
				'\t' >> keyword >> separator >>
				*parameter
			;

			auto const parameter_sets_def = 
				"parameter_set" >> separator >>
				*parameter_set
			;

			BOOST_SPIRIT_DEFINE(
				parameter_set,
				parameter_sets
			)

			auto grammar = parameter_sets;


		}


		namespace module{


			x3::rule< class module, types::parse::module > const module("module");
			x3::rule< class modules, types::parse::modules > const modules("modules");

			auto const module_def = 
				'\t' >> keyword >> *space >> '=' >> *space >> value >> separator >>
				*("\t\tparameter_set" >> *space >> '=' >> *space >> value >> separator) >>
				*parameter
			;

			auto const modules_def =
				"module" >> separator >>
				*module
			;

			BOOST_SPIRIT_DEFINE(
				module,
				modules
			)

			auto grammar = modules;


		}


		namespace chain{


			x3::rule< class io, types::parse::io > const io("io");
			x3::rule< class chain_module, types::parse::chain_module > const chain_module("chain_module");
			x3::rule< class chain, types::parse::chain > const chain("chain");
			x3::rule< class chains, types::parse::chains > const chains("chains");

			auto const io_def =
				"\t\t\t\t" >> keyword >> *space >> '=' >> *space >> value >> separator
			;

			auto const chain_module_def =
				"\t\t" >> keyword >> separator >>
				-(
					"\t\t\t<-" >> separator >>
					*io
				) >>
				-(
					"\t\t\t->" >> separator >>
					*io
				)
			;

			auto const chain_def =
				'\t' >> keyword >> *space >> -('=' >> *space >> value) >> separator >>
				-("\t\tid_generator" >> *space >> '=' >> *space >> value >> separator) >>
				*chain_module
			;

			auto const chains_def =
				"chain" >> separator >>
				*chain
			;

			BOOST_SPIRIT_DEFINE(
				io,
				chain_module,
				chain,
				chains
			)

			auto grammar = chains;


		}


		x3::rule< class config_class, types::parse::config > const config("config");

		auto const config_def = x3::no_skip[x3::expect[
			empty_lines >> comment >>
			set::grammar >>
			module::grammar >>
			chain::grammar
		]];

		BOOST_SPIRIT_DEFINE(
			config
		)


		auto const grammar = config;


	}


	types::parse::config parse(std::istream& is){
		namespace x3 = boost::spirit::x3;

		std::string str{
			std::istreambuf_iterator< char >{is},
			std::istreambuf_iterator< char >{}
		};

		types::parse::config config;

		auto iter = str.begin();
		auto end = str.end();

		x3::ascii::space_type space;
		bool const match =
			phrase_parse(iter, end, parser::grammar, space, config);

		if(!match || iter != end){
			// FIXME: eol can be '\n', '\r' or "\r\n"
			auto line = 1 + std::count(str.begin(), iter, '\n');
			auto pos =
				std::find(std::make_reverse_iterator(iter), str.rend(), '\n') -
				std::make_reverse_iterator(iter);

			throw std::runtime_error(
				"Syntax error, near line:charakter " +
				std::to_string(line) + ":" + std::to_string(pos)
			);
		}

		return config;
	}


	types::parse::config parse(std::string const& filename){
		std::ifstream is(filename.c_str());
		if(!is.is_open()){
			throw std::runtime_error("Can not open '" + filename + "'");
		}

		return parse(is);
	}



}
