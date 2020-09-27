#ifndef CONFIG_H
#define CONFIG_H
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/phoenix/object/construct.hpp>
#include <boost/phoenix/statement/for.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/variant.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace internal
{
using vector_string = boost::variant<std::vector<std::string>, std::string>;

struct vector_string_visitor
    : public boost::static_visitor<std::vector<std::string>>
{
  std::vector<std::string> operator()(const std::vector<std::string>& v) const
  {
    return v;
  }
  std::vector<std::string> operator()(const std::string& v) const
  {
    std::vector<std::string> vs;
    vs.push_back(v);
    return vs;
  }
};

struct config_row
{
  std::string key;
  vector_string values;
};

using config_file = std::vector<config_row>;
}  // namespace internal

BOOST_FUSION_ADAPT_STRUCT(internal::config_row, (std::string, key),
                          (internal::vector_string, values))

struct config
{
  std::string api_key;
  std::string api_secret;
};

class config_parser
{
  template<typename Iterator>
  struct skipper : boost::spirit::qi::grammar<Iterator>
  {
    skipper()
        : skipper::base_type(start)
    {
      start   = boost::spirit::ascii::space | comment;
      comment = '#' >> *(boost::spirit::qi::char_ - boost::spirit::eol)
                >> boost::spirit::eol;
    }

    boost::spirit::qi::rule<Iterator> start;
    boost::spirit::qi::rule<Iterator> comment;
  };

  template<typename Iterator>
  struct grammar
      : boost::spirit::qi::grammar<Iterator, internal::config_file(),
                                   skipper<Iterator>>
  {
    grammar()
        : grammar::base_type(start, "start")
    {
      namespace qi      = boost::spirit::qi;
      namespace ascii   = boost::spirit::ascii;
      namespace phoenix = boost::phoenix;

      using ascii::space;
      using phoenix::at_c;
      using qi::_1;
      using qi::_2;
      using qi::_val;
      using qi::char_;
      using qi::eps;
      using qi::lexeme;
      using qi::lit;
      using qi::alnum;

      start = +expr;
      expr  = key[at_c<0>(_val) = _1] >> ':' >> value[at_c<1>(_val) = _1];
      key   = lexeme[+(char_ - ':')[_val += _1]];
      value = slice | str;
      slice = '[' >> str % ',' >> ']';
      str   = lexeme[+(alnum | char_('-') | char_(':'))[_val += _1]];

      start.name("start");
      expr.name("expr");
      key.name("key");
      value.name("value");
      slice.name("array");
      str.name("string");

      using phoenix::construct;
      using phoenix::val;
      using qi::_3;
      using qi::_4;
      using qi::fail;
      using qi::on_error;

      on_error<fail>(start, std::cout << _4 << " on "
                                      << construct<std::string>(_3, _2)
                                      << std::endl);
    }

    boost::spirit::qi::rule<Iterator, internal::config_file(),
                            skipper<Iterator>>
        start;
    boost::spirit::qi::rule<Iterator, internal::config_row(), skipper<Iterator>>
        expr;
    boost::spirit::qi::rule<Iterator, std::string(), skipper<Iterator>> key;
    boost::spirit::qi::rule<Iterator, internal::vector_string(),
                            skipper<Iterator>>
        value;
    boost::spirit::qi::rule<Iterator, std::vector<std::string>(),
                            skipper<Iterator>>
        slice;
    boost::spirit::qi::rule<Iterator, std::string(), skipper<Iterator>> str;
  };

public:
  template<typename Iterator>
  static config parse_line(Iterator begin, Iterator end)
  {
    grammar<Iterator> grammar;
    internal::config_file res;

    bool r = boost::spirit::qi::phrase_parse(begin, end, grammar,
                                             skipper<Iterator>(), res);
    if (!r)
      throw std::runtime_error("error parsing: " + std::string(begin, end));

    config c;
    for (const auto& e : res)
    {
      const auto& v =
          boost::apply_visitor(internal::vector_string_visitor(), e.values);
      if (e.key == "api_key")
      {
        c.api_key = v[0];
      }
      else if (e.key == "api_secret")
      {
        c.api_secret = v[0];
      }
    }

    return c;
  }

  static config parse_file(const std::string& filename)
  {
    std::ifstream is(filename, std::ios::binary);
    is.unsetf(std::ios::skipws);

    if (not is)
      throw std::runtime_error("can't open file: " + filename);

    return parse_line(boost::spirit::istream_iterator(is),
                      boost::spirit::istream_iterator());
  }
};
#endif
