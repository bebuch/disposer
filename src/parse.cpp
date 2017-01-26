//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
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


		x3::rule< struct space_tag > const space("space");

		x3::rule< struct space_lines_tag > const space_lines("space_lines");

		x3::rule< struct separator_tag > const separator("separator");

		x3::rule< struct comment_tag > const comment("comment");

		x3::rule< struct keyword_spaces_tag > const
			keyword_spaces("keyword_spaces");

		x3::rule< struct keyword_tag, std::string > const keyword("keyword");

		x3::rule< struct value_spaces_tag > const
			value_spaces("value_spaces");

		x3::rule< struct value_tag, std::string > const value("value");

		struct parameter_tag;
		x3::rule< parameter_tag, types::parse::parameter > const
			parameter("parameter");

		struct prevent_parameter_set_tag;
		x3::rule< prevent_parameter_set_tag > const
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


		struct prevent_parameter_set_tag: error_base{
			virtual const char* message()const override{
				return "a parameter, but a parameter name "
					"('\t\tname = value\n') must not be 'parameter_set'";
			}
		};


		namespace module{

			struct modules_tag: error_base{
				virtual const char* message()const override;
			};

		}

		namespace chain{

			struct chains_tag: error_base{
				virtual const char* message()const override;
			};

		}


		namespace set{


			struct parameter_set_tag;
			x3::rule<
				parameter_set_tag, types::parse::parameter_set
			> const parameter_set("parameter_set");

			struct parameter_set_params_tag;
			x3::rule<
				parameter_set_params_tag, std::vector< types::parse::parameter >
			> const parameter_set_params("parameter_set_params");

			struct parameter_sets_tag;
			x3::rule<
				parameter_sets_tag, types::parse::parameter_sets
			> const parameter_sets("parameter_sets");

			struct parameter_sets_params_tag;
			x3::rule<
				parameter_sets_params_tag, types::parse::parameter_sets
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
					&x3::expect["module" >> separator]
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


			struct parameter_set_tag: error_base{
				virtual const char* message()const override{
					return "a parameter '\t\tname = value\n' with name != "
						"'parameter_set'";
				}
			};

			struct parameter_set_params_tag: error_base{
				virtual const char* message()const override{
					return "at least one parameter line '\t\tname = value\n' "
						"with name != 'parameter_set'";
				}
			};

			struct parameter_sets_tag: error_base{
				template < typename Iter, typename Exception, typename Context >
				x3::error_handler_result on_error(
					Iter& first, Iter const& last,
					Exception const& x, Context const& context
				){
					msg_ = "keyword line 'parameter_set\n'";
					if(x.which() != "separator"){
						msg_ += " or ";
						msg_ += module::modules_tag().message();
					}
					return error_base::on_error(first, last, x, context);
				}

				virtual const char* message()const override{
					return msg_.c_str();
				}

				std::string msg_;
			};

			struct parameter_sets_params_tag: error_base{
				virtual const char* message()const override{
					return "at least one parameter set line '\tname\n'";
				}
			};



		}


		namespace module{


			struct module_tag;
			x3::rule< module_tag, types::parse::module > const
				module("module");

			struct module_sets_tag;
			x3::rule< module_sets_tag, std::vector< std::string > > const
				module_sets("module_sets");

			struct modules_tag;
			x3::rule< modules_tag, types::parse::modules > const
				modules("modules");

			struct modules_params_tag;
			x3::rule< modules_params_tag, types::parse::modules > const
				modules_params("modules_params");


			auto const module_def =
				(('\t' > keyword) > *space > '=' > *space > value > separator)
				>> module_sets >> *parameter
			;

			auto const module_sets_def =
				*(
					("\t\tparameter_set" >> *space >> '=')
					> *space > value > separator
				) >> &x3::expect[
					("\t\t" >> keyword) |
					("\t" >> keyword) |
					("chain")
				]
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


			struct module_tag: error_base{
				virtual const char* message()const override{
					return "a module line '\tname = module\n'";
				}
			};

			struct module_sets_tag: error_base{
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
						msg_ += " or ";
						msg_ += module_tag().message();
						msg_ += " or ";
						msg_ += chain::chains_tag().message();
					}
					return error_base::on_error(first, last, x, context);
				}

				virtual const char* message()const override{
					return msg_.c_str();
				}

				std::string msg_;
			};

			const char* modules_tag::message()const{
				return "keyword line 'module\n'";
			}

			struct modules_params_tag: error_base{
				virtual const char* message()const override{
					return "at least one module line '\tname = module\n'";
				}
			};


		}


		namespace chain{


			struct input_tag;
			x3::rule< input_tag, types::parse::io > const input("input");

			struct output_tag;
			x3::rule< output_tag, types::parse::io > const output("output");

			struct input_params_tag;
			x3::rule< input_params_tag, std::vector< types::parse::io > >
				const input_params("input_params");

			struct output_params_tag;
			x3::rule< output_params_tag, std::vector< types::parse::io > >
				const output_params("output_params");

			struct inputs_tag;
			x3::rule< inputs_tag, std::vector< types::parse::io > > const
				inputs("inputs");

			struct outputs_tag;
			x3::rule< outputs_tag, std::vector< types::parse::io > > const
				outputs("outputs");

			struct chain_module_tag;
			x3::rule< chain_module_tag, types::parse::chain_module > const
				chain_module("chain_module");

			struct chain_tag;
			x3::rule< chain_tag, types::parse::chain > const chain("chain");

			struct chain_params_tag;
			x3::rule< chain_params_tag, std::vector< types::parse::chain_module > >
				const chain_params("chain_params");

			struct group_tag;
			x3::rule< group_tag, std::string > const group("group");

			struct id_generator_tag;
			x3::rule< id_generator_tag, std::string > const
				id_generator("id_generator");

			struct chains_tag;
			x3::rule< chains_tag, types::parse::chains > const
				chains("chains");

			struct chains_params_tag;
			x3::rule< chains_params_tag, types::parse::chains > const
				chains_params("chains_params");


			auto const input_def =
				"\t\t\t\t" > (keyword >> *space) >
					('=' >> *space) > value > separator
			;

			auto const output_def =
				"\t\t\t\t" > (keyword >> *space) >
					('=' >> *space) > value > separator
			;

			auto const input_params_def =
				x3::expect[+input]
			;

			auto const output_params_def =
				x3::expect[+output]
			;

			auto const inputs_def =
				("\t\t\t<-" > separator) >>
				input_params
			;

			auto const outputs_def =
				("\t\t\t->" > separator) >>
				output_params
			;

			auto const chain_module_def =
				("\t\t" > keyword > separator) >>
				-inputs >>
				-outputs
			;

			auto const group_def =
				('=' >> *space) > value
			;

			auto const id_generator_def =
				("\t\tid_generator" >> *space) >
					('=' >> *space) > value > separator
			;

			auto const chain_params_def =
				x3::expect[+chain_module]
			;

			auto const chain_def =
				('\t' > (keyword >> *space) > -group > separator) >>
				-id_generator >>
				chain_params
			;

			auto const chains_def =
				(x3::expect["chain"] > separator) >>
				chains_params
			;

			auto const chains_params_def =
				x3::expect[+chain]
			;


			BOOST_SPIRIT_DEFINE(
				input,
				output,
				inputs,
				outputs,
				input_params,
				output_params,
				chain_module,
				chain_params,
				chain,
				group,
				id_generator,
				chains_params,
				chains
			)


			auto grammar = chains;


			const char* chains_tag::message()const{
				return "keyword line 'chain\n'";
			}

			struct input_tag: error_base{
				virtual const char* message()const override{
					return "input map '\t\t\t\tparameter = variable'";
				}
			};

			struct output_tag: error_base{
				virtual const char* message()const override{
					return "output map '\t\t\t\tparameter = variable'";
				}
			};

			struct input_params_tag: error_base{
				virtual const char* message()const override{
					return "at least one input map "
						"'\t\t\t\tparameter = variable'";
				}
			};

			struct output_params_tag: error_base{
				virtual const char* message()const override{
					return "at least one output map "
						"'\t\t\t\tparameter = variable'";
				}
			};

			struct inputs_tag: error_base{
				virtual const char* message()const override{
					return "keyword line '\t\t\t->\n'";
				}
			};

			struct outputs_tag: error_base{
				virtual const char* message()const override{
					return "keyword line '\t\t\t<-\n'";
				}
			};

			struct chain_module_tag: error_base{
				virtual const char* message()const override{
					return "a module '\t\tmodule\n'";
				}
			};

			struct chains_params_tag: error_base{
				virtual const char* message()const override{
					return "at least one chain line '\tname [= group]\n'";
				}
			};

			struct group_tag: error_base{
				virtual const char* message()const override{
					return "a chain line with group '\tname = group\n'";
				}
			};

			struct id_generator_tag: error_base{
				virtual const char* message()const override{
					return "a id_generator line '\t\tid_generator = name\n', "
						"where id_generator is a keyword";
				}
			};


		}


		struct config_tag;
		x3::rule< config_tag, types::parse::config >
			const config("config");

		auto const config_def = x3::no_skip[x3::expect[
			space_lines >> -comment >>
			-set::grammar >> module::grammar >> chain::grammar
		]];


		BOOST_SPIRIT_DEFINE(
			config
		)


		auto const grammar = config;


		struct config_tag: error_base{
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
