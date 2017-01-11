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
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <fstream>
#include <string>
#include <complex>
#include <iostream>

#include <disposer/log_base.hpp>
#include <disposer/log.hpp>



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


		x3::rule< struct space > const space("space");

		x3::rule< struct empty_lines > const empty_lines("empty_lines");

		x3::rule< struct separator > const separator("separator");

		x3::rule< struct comment > const comment("comment");

		x3::rule< struct keyword_spaces, std::string > const
			keyword_spaces("keyword_spaces");

		x3::rule< struct keyword, std::string > const keyword("keyword");

		x3::rule< struct value_spaces, std::string > const
			value_spaces("value_spaces");

		x3::rule< struct value, std::string > const value("value");

		x3::rule< struct parameter, types::parse::parameter > const
			parameter("parameter");


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


			x3::rule<
				struct parameter_set, types::parse::parameter_set
			> const parameter_set("parameter_set");

			x3::rule<
				struct parameter_sets, types::parse::parameter_sets
			> const parameter_sets("parameter_sets");


			auto const parameter_set_def =
				'\t' >> keyword >> separator >>
				*parameter
			;

			auto const parameter_sets_def =
				"parameter_set" > separator >>
				*parameter_set
			;


			BOOST_SPIRIT_DEFINE(
				parameter_set,
				parameter_sets
			)


			auto grammar = parameter_sets;


		}


		namespace module{


			x3::rule< struct module, types::parse::module > const
				module("module");

			x3::rule< struct modules, types::parse::modules > const
				modules("modules");


			auto const module_def =
				'\t' >> keyword >> *space >>
					'=' >> *space >> value >> separator >>
				*("\t\tparameter_set" >> *space >>
					'=' >> *space >> value >> separator) >>
				*parameter
			;

			auto const modules_def =
				"module" > separator >>
				*module
			;

			BOOST_SPIRIT_DEFINE(
				module,
				modules
			)

			auto grammar = modules;


		}


		namespace chain{


			x3::rule< struct io, types::parse::io > const io("io");

			x3::rule< struct chain_module, types::parse::chain_module > const
				chain_module("chain_module");

			x3::rule< struct chain, types::parse::chain > const chain("chain");

			x3::rule< struct chains, types::parse::chains > const
				chains("chains");


			auto const io_def =
				"\t\t\t\t" >> keyword >> *space >>
					 '=' >> *space >> value >> separator
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
				'\t' >> keyword >> *space >>
					-('=' >> *space >> value) >> separator >>
				-("\t\tid_generator" >> *space >>
					'=' >> *space >> value >> separator) >>
				*chain_module
			;

			auto const chains_def =
				"chain" > separator >>
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


		template < typename Iterator >
		std::size_t line_count(Iterator first, Iterator last)
		{
			namespace x3 = boost::spirit::x3;
			using x3::eol;

			std::size_t n = 1;
			auto const inc = [&n](x3::unused_type){ ++n; };

			x3::phrase_parse(first, last,
				x3::no_skip[ *(*(x3::omit[char_] - eol) >> eol[inc]) ],
				x3::space);

			return n;
		}


		template < typename Iterator >
		std::string get_error_line_before(Iterator first, Iterator pos)
		{
			namespace x3 = boost::spirit::x3;
			using x3::eol;

			auto line_start = std::find_if(
				std::make_reverse_iterator(pos),
				std::make_reverse_iterator(first),
				[](char c){ return c == '\n' || c == '\r'; }).base();

			return std::string(line_start, pos);
		}

		template < typename Iterator >
		std::string get_error_line_after(Iterator pos, Iterator last)
		{
			namespace x3 = boost::spirit::x3;
			using x3::eol;

			auto line_end = std::find_if(
				pos, last, [](char c){ return c == '\n' || c == '\r'; });

			return std::string(pos, line_end);
		}


		struct config{
			template < typename Iter, typename Exception, typename Context >
			x3::error_handler_result on_error(
				Iter& first, Iter const& last,
				Exception const& x, Context const& context
			){
				using namespace std::literals::string_literals;

				auto const line_number = line_count(first, x.where());
				auto const before = get_error_line_before(first, x.where());
				auto const after = get_error_line_after(x.where(), last);
				std::ostringstream os;
				os << "Syntax error at line " << line_number << ", pos "
					<< before.size() << ": '" << before << after << "'";

				throw std::runtime_error(os.str());

				return x3::error_handler_result::fail;
			}
		};
		x3::rule< config, types::parse::config >
			const config("config");


		auto const config_def = x3::no_skip[
			empty_lines > comment >
			set::grammar >
			module::grammar >
			chain::grammar
		];


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
			auto const line_number = parser::line_count(str.begin(), iter);
			auto const before =
				parser::get_error_line_before(str.begin(), iter);
			auto const after = parser::get_error_line_after(iter, end);
			std::ostringstream os;
			os << "Syntax error at line " << line_number << ", pos "
				<< before.size() << ": '" << before << after << "'";
			throw std::runtime_error(os.str());
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
