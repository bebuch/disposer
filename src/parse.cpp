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
#include <map>

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


		class syntax_error: public std::logic_error{
		public:
			syntax_error(std::string&& message, std::string::iterator pos):
				std::logic_error(std::move(message)),
				pos_(pos)
				{}

			std::string::iterator pos()const{ return pos_; }


		private:
			std::string::iterator const pos_;
		};


		namespace x3 = boost::spirit::x3;


		using x3::lit;
		using x3::char_;
		using x3::eol;
		using x3::eoi;


		/// \brief Parser for line counting
		template < typename Iterator >
		std::size_t line_count(Iterator first, Iterator last){
			namespace x3 = boost::spirit::x3;
			using x3::eol;

			std::size_t n = 1;
			auto const inc = [&n](x3::unused_type){ ++n; };

			x3::phrase_parse(first, last,
				x3::no_skip[ *(*(x3::omit[char_] - eol) >> eol[inc]) ],
				x3::space);

			return n;
		}

		/// \brief Get text from the last line
		template < typename Iterator >
		std::string get_error_line_before(Iterator first, Iterator pos){
			namespace x3 = boost::spirit::x3;
			using x3::eol;

			auto line_start = std::find_if(
				std::make_reverse_iterator(pos),
				std::make_reverse_iterator(first),
				[](char c){ return c == '\n' || c == '\r'; }).base();

			return std::string(line_start, pos);
		}

		/// \brief Get text from the first line
		template < typename Iterator >
		std::string get_error_line_after(Iterator pos, Iterator last){
			namespace x3 = boost::spirit::x3;
			using x3::eol;

			auto line_end = std::find_if(
				pos, last, [](char c){ return c == '\n' || c == '\r'; });

			if(last == line_end){
				return std::string(pos, line_end);
			}else{
				return std::string(pos, line_end) + '\n';
			}
		}


		struct error_base{
			template < typename Iter, typename Exception, typename Context >
			x3::error_handler_result on_error(
				Iter& /*first*/, Iter const& /*last*/,
				Exception const& x, Context const& /*context*/
			){
				throw syntax_error(this->message(), x.where());
			}

			virtual const char* message()const = 0;
		};


		x3::rule< struct space > const space("space");

		x3::rule< struct space_lines > const space_lines("space_lines");

		x3::rule< struct separator > const separator("separator");

		x3::rule< struct comment > const comment("comment");

		x3::rule< struct keyword_spaces > const
			keyword_spaces("keyword_spaces");

		x3::rule< struct keyword, std::string > const keyword("keyword");

		x3::rule< struct value_spaces > const
			value_spaces("value_spaces");

		x3::rule< struct value, std::string > const value("value");

		struct parameter_tag;
		x3::rule< parameter_tag, types::parse::parameter > const
			parameter("parameter");

		struct prevent_parameter_set;
		x3::rule< prevent_parameter_set > const
			prevent_parameter_set("prevent_parameter_set");


		auto const space_def =
			lit(' ') | '\t'
		;

		auto const space_lines_def =
			*(*space >> eol)
		;

		auto const separator_def =
			*space >> (eol >> space_lines >> -comment)
		;

		auto const comment_def =
			*space >> (lit('#') | ';') >> *(char_ - eol) >> separator
		;

		auto const keyword_spaces_def =
			+(char_(' ') | char_('\t')) >> !(eol | '=')
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

		auto const prevent_parameter_set_def =
			x3::expect[!("parameter_set" >> *space >> '=')]
		;

		auto const parameter_def =
			("\t\t" >> prevent_parameter_set) > keyword  >
			*space > '=' > *space > value > separator
		;


		BOOST_SPIRIT_DEFINE(
			space,
			space_lines,
			separator,
			comment,
			keyword_spaces,
			keyword,
			value_spaces,
			value,
			prevent_parameter_set,
			parameter
		)


		struct parameter_tag: error_base{
			virtual const char* message()const override{
				return "a parameter '\t\tname = value\n' with name != "
					"'parameter_set'";
			}
		};


		struct prevent_parameter_set: error_base{
			virtual const char* message()const override{
				return "a parameter, but a parameter name "
					"('\t\tname = value\n') must not be 'parameter_set'";
			}
		};


		namespace module{

			struct modules: error_base{
				virtual const char* message()const override;
			};

		}


		namespace set{


			struct parameter_set;
			x3::rule<
				parameter_set, types::parse::parameter_set
			> const parameter_set("parameter_set");

			struct parameter_set_params;
			x3::rule<
				parameter_set_params, std::vector< types::parse::parameter >
			> const parameter_set_params("parameter_set_params");

			struct parameter_sets;
			x3::rule<
				parameter_sets, types::parse::parameter_sets
			> const parameter_sets("parameter_sets");

			struct parameter_sets_params;
			x3::rule<
				parameter_sets_params, types::parse::parameter_sets
			> const parameter_sets_params("parameter_sets_params");


			auto const parameter_set_def =
				('\t' > keyword > separator) >>
				parameter_set_params
			;

			auto const parameter_set_params_def =
				x3::expect[+parameter]
			;

			auto const parameter_sets_def =
				(
					("parameter_set" > separator) |
					(x3::expect["chain" >> separator])
					[([](auto& ctx){ x3::_pass(ctx) = false; })]
				) >> parameter_sets_params
			;

			auto const parameter_sets_params_def =
				x3::expect[+parameter_set]
			;


			BOOST_SPIRIT_DEFINE(
				parameter_set,
				parameter_set_params,
				parameter_sets,
				parameter_sets_params
			)


			auto grammar = parameter_sets;


			struct parameter_set: error_base{
				virtual const char* message()const override{
					return "a parameter '\t\tname = value\n' with name != "
						"'parameter_set'";
				}
			};

			struct parameter_set_params: error_base{
				virtual const char* message()const override{
					return "at least one parameter line '\t\tname = value\n' "
						"with name != 'parameter_set'";
				}
			};

			struct parameter_sets: error_base{
				template < typename Iter, typename Exception, typename Context >
				x3::error_handler_result on_error(
					Iter& first, Iter const& last,
					Exception const& x, Context const& context
				){
					msg_ = "keyword line 'parameter_set\n'";
					if(x.which() != "separator"){
						msg_ += " or ";
						msg_ += module::modules().message();
					}
					return error_base::on_error(first, last, x, context);
				}

				virtual const char* message()const override{
					return msg_.c_str();
				}

				std::string msg_;
			};

			struct parameter_sets_params: error_base{
				virtual const char* message()const override{
					return "at least one parameter set line '\tname\n'";
				}
			};



		}


		namespace module{


			struct module;
			x3::rule< module, types::parse::module > const
				module("module");

			struct module_sets;
			x3::rule< module_sets, std::vector< std::string > > const
				module_sets("module_sets");

			struct modules;
			x3::rule< modules, types::parse::modules > const
				modules("modules");

			struct modules_params;
			x3::rule< modules_params, types::parse::modules > const
				modules_params("modules_params");


			auto const module_def =
				(('\t' > keyword) > *space > '=' > *space > value > separator)
				>> module_sets >> *parameter
			;

			auto const module_sets_def =
				*(
					(
						("\t\tparameter_set" >> *space >> '=')
						> *space > value > separator
					) | (
						(x3::expect["\t\t" >> keyword])
						[([](auto& ctx){ x3::_pass(ctx) = false; })]
					)
				)
			;

			auto const modules_def =
				(x3::expect["module"] > separator) >>
				modules_params
			;

			auto const modules_params_def =
				x3::expect[+module]
			;

			BOOST_SPIRIT_DEFINE(
				module,
				module_sets,
				modules,
				modules_params
			)

			auto grammar = modules;


			struct module: error_base{
				virtual const char* message()const override{
					return "a module line '\tname = module\n'";
				}
			};

			struct module_sets: error_base{
				template < typename Iter, typename Exception, typename Context >
				x3::error_handler_result on_error(
					Iter& first, Iter const& last,
					Exception const& x, Context const& context
				){
					msg_ = "a parameter_set reference "
						"'\t\tparameter_set = name\n', where 'parameter_set' "
						"is a keyword and 'name' the name of the referenced "
						"parameter set";
					if(x.which() != "value"){
						msg_ += " or ";
						msg_ += parameter_tag().message();
					}
					return error_base::on_error(first, last, x, context);
				}

				virtual const char* message()const override{
					return msg_.c_str();
				}

				std::string msg_;
			};

			const char* modules::message()const{
				return "keyword line 'module\n'";
			}

			struct modules_params: error_base{
				virtual const char* message()const override{
					return "at least one module line '\tname = module\n'";
				}
			};


		}


		namespace chain{


			x3::rule< struct io, types::parse::io > const io("io");

			x3::rule< struct chain_module, types::parse::chain_module > const
				chain_module("chain_module");

			x3::rule< struct chain, types::parse::chain > const chain("chain");

			struct chains;
			x3::rule< chains, types::parse::chains > const
				chains("chains");


			auto const io_def =
				"\t\t\t\t" > (keyword >> *space) >
					('=' >> *space) > value > separator
			;

			auto const chain_module_def =
				("\t\t" > keyword > separator) >>
				-(
					("\t\t\t<-" > separator) >>
					*io
				) >>
				-(
					("\t\t\t->" > separator) >>
					*io
				)
			;

			auto const chain_def =
				('\t' > (keyword >> *space) >>
					-(('=' >> *space) > value) > separator) >>
				-(("\t\tid_generator" >> *space) >
					('=' >> *space) > value > separator) >>
				*chain_module
			;

			auto const chains_def =
				(x3::expect["chain"] > separator) >>
				*chain
			;


			BOOST_SPIRIT_DEFINE(
				io,
				chain_module,
				chain,
				chains
			)


			auto grammar = chains;


			struct chains: error_base{
				virtual const char* message()const override{
					return "keyword line 'chain\n'";
				}
			};


		}


		struct config;
		x3::rule< config, types::parse::config >
			const config("config");

		auto const config_def = x3::no_skip[x3::expect[
			space_lines >> -comment >>
			-set::grammar >> module::grammar >> chain::grammar
		]];


		BOOST_SPIRIT_DEFINE(
			config
		)


		auto const grammar = config;


		struct config: error_base{
			virtual const char* message()const override{
				return "keyword line 'parameter_set\n' or keyword line "
					"'module\n'";
			}
		};


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

		try{
			bool const match =
				phrase_parse(iter, end, parser::grammar, space, config);

			if(!match || iter != end){
				auto const line_number = parser::line_count(str.begin(), iter);
				auto const before =
					parser::get_error_line_before(str.begin(), iter);
				auto const after = parser::get_error_line_after(iter, end);
				std::ostringstream os;
				os << "Syntax error at line " << line_number << ", pos "
					<< before.size() << ": '" << before << after
					<< "', incomplete parsing (programming error!)";
				throw std::runtime_error(os.str());
			}

			return config;
		}catch(parser::syntax_error const& e){
			auto const line_number = parser::line_count(str.begin(), e.pos());
			auto const before =
				parser::get_error_line_before(str.begin(), e.pos());
			auto const after = parser::get_error_line_after(e.pos(), end);

			std::ostringstream os;
			os << "Syntax error at line " << line_number << ", pos "
				<< before.size() << ": '" << before << after
				<< "', expected " << e.what();

			throw std::runtime_error(os.str());
		}
	}


	types::parse::config parse(std::string const& filename){
		std::ifstream is(filename.c_str());
		if(!is.is_open()){
			throw std::runtime_error("Can not open '" + filename + "'");
		}

		return parse(is);
	}



}
