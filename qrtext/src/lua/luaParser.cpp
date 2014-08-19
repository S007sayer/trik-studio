#include "qrtext/src/lua/luaParser.h"

#include "qrtext/core/parser/parserRef.h"
#include "qrtext/core/parser/operators/parserCombinators.h"
#include "qrtext/core/parser/operators/expressionParser.h"

#include "qrtext/core/ast/expression.h"

#include "qrtext/lua/ast/addition.h"
#include "qrtext/lua/ast/assignment.h"
#include "qrtext/lua/ast/bitwiseAnd.h"
#include "qrtext/lua/ast/bitwiseLeftShift.h"
#include "qrtext/lua/ast/bitwiseNegation.h"
#include "qrtext/lua/ast/bitwiseOr.h"
#include "qrtext/lua/ast/bitwiseRightShift.h"
#include "qrtext/lua/ast/bitwiseXor.h"
#include "qrtext/lua/ast/block.h"
#include "qrtext/lua/ast/concatenation.h"
#include "qrtext/lua/ast/division.h"
#include "qrtext/lua/ast/equality.h"
#include "qrtext/lua/ast/exponentiation.h"
#include "qrtext/lua/ast/false.h"
#include "qrtext/lua/ast/fieldInitialization.h"
#include "qrtext/lua/ast/floatNumber.h"
#include "qrtext/lua/ast/functionCall.h"
#include "qrtext/lua/ast/greaterOrEqual.h"
#include "qrtext/lua/ast/greaterThan.h"
#include "qrtext/lua/ast/identifier.h"
#include "qrtext/lua/ast/indexingExpression.h"
#include "qrtext/lua/ast/inequality.h"
#include "qrtext/lua/ast/integerDivision.h"
#include "qrtext/lua/ast/integerNumber.h"
#include "qrtext/lua/ast/length.h"
#include "qrtext/lua/ast/lessOrEqual.h"
#include "qrtext/lua/ast/lessThan.h"
#include "qrtext/lua/ast/methodCall.h"
#include "qrtext/lua/ast/modulo.h"
#include "qrtext/lua/ast/multiplication.h"
#include "qrtext/lua/ast/nil.h"
#include "qrtext/lua/ast/not.h"
#include "qrtext/lua/ast/number.h"
#include "qrtext/lua/ast/string.h"
#include "qrtext/lua/ast/subtraction.h"
#include "qrtext/lua/ast/tableConstructor.h"
#include "qrtext/lua/ast/true.h"
#include "qrtext/lua/ast/unaryMinus.h"

using namespace qrtext::lua;
using namespace qrtext::lua::details;
using namespace qrtext::core;
using namespace qrtext::core::parser;
using namespace qrtext::core::lexer;

LuaParser::LuaParser(QList<Error> &errors)
	: parser::Parser<LuaTokenTypes>(grammar(), errors)
{
}

QSharedPointer<parser::ParserInterface<LuaTokenTypes>> LuaParser::grammar()
{
	QSharedPointer<core::parser::PrecedenceTable<LuaTokenTypes>> precedenceTable(new LuaPrecedenceTable());

	ParserRef<LuaTokenTypes> stat;
	ParserRef<LuaTokenTypes> explist;
	ParserRef<LuaTokenTypes> exp;
	ParserRef<LuaTokenTypes> primary;
	ParserRef<LuaTokenTypes> prefixexp;
	ParserRef<LuaTokenTypes> varpart;
	ParserRef<LuaTokenTypes> functioncallpart;
	ParserRef<LuaTokenTypes> prefixterm;
	ParserRef<LuaTokenTypes> args;
	ParserRef<LuaTokenTypes> tableconstructor;
	ParserRef<LuaTokenTypes> fieldlist;
	ParserRef<LuaTokenTypes> field;
	ParserRef<LuaTokenTypes> fieldsep;
	ParserRef<LuaTokenTypes> binop;
	ParserRef<LuaTokenTypes> unop;

	auto reportUnsupported = [this] (Token<LuaTokenTypes> const &token) {
		context().reportError(token, "This construction is not supported yet");
		return new core::ast::Node();
	};

	// Additional production that parses unqualified identifier.
	auto identifier = LuaTokenTypes::identifier
			>> [] (Token<LuaTokenTypes> const &token) { return new ast::Identifier(token.lexeme()); };

	// block ::= {stat}
	auto block = *stat
			>> [] (QSharedPointer<TemporaryList> statList) {
				QList<QSharedPointer<core::ast::Node>> result;
				for (auto const &stat : statList->list()) {
					if (stat->is<TemporaryList>()) {
						// It is a list of assignments.
						for (auto const &assignment : as<TemporaryList>(stat)->list()) {
							result << assignment;
						}
					} else {
						// It is expression.
						result << stat;
					}
				}

				if (result.size() == 1) {
					// Do not create Block node for simple expression.
					return result.first();
				} else {
					return wrap(new ast::Block(result));
				}
			};

	// stat ::= ‘;’ | explist [‘=’ explist]
	stat = (-LuaTokenTypes::semicolon | (explist & ~(-LuaTokenTypes::equals & explist)))
			>> [this] (QSharedPointer<TemporaryPair> pair) {
				if (!pair) {
					// It is semicolon, just discard it.
					return wrap(nullptr);
				}

				if (!pair->right()) {
					auto expList = as<TemporaryList>(pair->left());
					if (expList->list().size() != 1) {
						// It is a list of expressions which we translate as table constructor, to support convenient
						// lists syntax, for example, for lists of ports in robots.
						QList<QSharedPointer<core::ast::Node>> fields;
						for (auto exp : expList->list()) {
							fields << wrap(new ast::FieldInitialization(as<core::ast::Expression>(exp)));
						}

						return wrap(new ast::TableConstructor(as<ast::FieldInitialization>(fields)));
					} else {
						return expList->list().first();
					}
				}

				auto variables = as<TemporaryList>(pair->left())->list();
				auto values = as<TemporaryList>(pair->right())->list();
				if (variables.size() != values.size()) {
					context().reportError(pair, QObject::tr(
							"Number of variables in assignment shall be equal to the number of assigned values"));

					return wrap(nullptr);
				}

				auto result = QSharedPointer<TemporaryList>(new TemporaryList());

				for (int i = 0; i < variables.size(); ++i) {
					auto variable = as<core::ast::Expression>(variables[i]);

					if (variable->is<ast::FunctionCall>()) {
						context().reportError(pair, QObject::tr("Assignment to function call is impossible"));
						continue;
					}

					auto value = as<core::ast::Expression>(values[i]);
					result->list() << wrap(new ast::Assignment(variable, value));
				}

				return as<core::ast::Node>(result);
			};

	// explist ::= exp(0) {‘,’ exp(0)}
	explist = (exp & *(-LuaTokenTypes::comma & exp))
			>> [] (QSharedPointer<TemporaryPair> node) {
				auto firstExp = as<core::ast::Expression>(node->left());
				auto temporaryList = as<TemporaryList>(node->right());
				temporaryList->list().prepend(firstExp);
				return temporaryList;
			};

	// exp(precedence) ::= primary { binop exp(newPrecedence) }
	exp = ParserRef<LuaTokenTypes>(new ExpressionParser<LuaTokenTypes>(precedenceTable, primary, binop));

	// primary ::= nil | false | true | Number | String | ‘...’ | prefixexp | tableconstructor | unop exp
	primary =
			LuaTokenTypes::nilKeyword >> [] { return new ast::Nil(); }
			| LuaTokenTypes::falseKeyword >> [] { return new ast::True(); }
			| LuaTokenTypes::trueKeyword >> [] { return new ast::False(); }
			| LuaTokenTypes::integerLiteral
					>> [] (Token<LuaTokenTypes> token) { return new ast::IntegerNumber(token.lexeme()); }
			| LuaTokenTypes::floatLiteral
					>> [] (Token<LuaTokenTypes> token) { return new ast::FloatNumber(token.lexeme()); }
			| LuaTokenTypes::string >> [] (Token<LuaTokenTypes> token) {
					QString string = token.lexeme();
					// Cut off quotes.
					string.remove(0, 1);
					string.chop(1);
					return new ast::String(string);

				}
			| LuaTokenTypes::tripleDot >> reportUnsupported
			| prefixexp
			| tableconstructor
			| (unop & ParserRef<LuaTokenTypes>(new ExpressionParser<LuaTokenTypes>(
					precedenceTable, LuaTokenTypes::minus, primary, binop))
					)
					>> [] (QSharedPointer<TemporaryPair> node) {
						auto unOp = as<core::ast::UnaryOperator>(node->left());
						unOp->setOperand(node->right());
						return unOp;
					}
			;

	// prefixexp ::= prefixterm { functioncallpart | varpart }
	prefixexp = (prefixterm & *(functioncallpart | varpart))
			>> [] (QSharedPointer<TemporaryPair> node) {
				auto result = as<core::ast::Expression>(node->left());
				for (auto const part : as<TemporaryList>(node->right())->list()) {
					if (part->is<core::ast::Expression>()) {
						// It is varpart (indexing expression)
						result = QSharedPointer<core::ast::Expression>(
								new ast::IndexingExpression(result, as<core::ast::Expression>(part)));
					} else if (part->is<TemporaryPair>()) {
						// It is functioncallpart (method call)
						auto const methodName = as<ast::Identifier>(as<TemporaryPair>(part)->left());
						auto const args = as<core::ast::Expression>(
								as<TemporaryList>(as<TemporaryPair>(part)->right())->list());

						result = QSharedPointer<core::ast::Expression>(new ast::MethodCall(result, methodName, args));
					} else if (part->is<TemporaryList>()) {
						// It is functioncallpart (function call)
						result = QSharedPointer<core::ast::Expression>(
								new ast::FunctionCall(result, as<core::ast::Expression>(as<TemporaryList>(part)->list())));
					}
				}

				return result;
			};

	// varpart ::= ‘[’ exp(0) ‘]’ | ‘.’ Name
	varpart = (-LuaTokenTypes::openingSquareBracket & exp & -LuaTokenTypes::closingSquareBracket)
			| (-LuaTokenTypes::dot & LuaTokenTypes::identifier >> [] (Token<LuaTokenTypes> const &token) {
						return new ast::String(token.lexeme());
					}
			)
			;

	// functioncallpart :: = args | ‘:’ Name args
	functioncallpart = args | (-LuaTokenTypes::colon & identifier & args);

	// prefixterm ::= Name | ‘(’ exp(0) ‘)’
	prefixterm = identifier
			| (-LuaTokenTypes::openingBracket & exp & -LuaTokenTypes::closingBracket)
			;

	// args ::= ‘(’ [explist] ‘)’ | tableconstructor | String
	args = ((-LuaTokenTypes::openingBracket & ~explist & -LuaTokenTypes::closingBracket)
			| tableconstructor
			| LuaTokenTypes::string >> [] (Token<LuaTokenTypes> token) { return new ast::String(token.lexeme()); }
			) >> [] (QSharedPointer<core::ast::Node> node) {
					if (node && node->is<TemporaryList>()) {
						return node;
					} else {
						auto result = QSharedPointer<TemporaryList>(new TemporaryList());
						if (node) {
							result->list() << as<core::ast::Expression>(node);
						}

						return as<core::ast::Node>(result);
					}
				}
			;

	// tableconstructor ::= ‘{’ [fieldlist] ‘}’
	tableconstructor = (-LuaTokenTypes::openingCurlyBracket & ~fieldlist & -LuaTokenTypes::closingCurlyBracket)
			>> [] (QSharedPointer<TemporaryList> fieldList) {
				if (!fieldList) {
					return wrap(new ast::TableConstructor({}));
				} else {
					return wrap(new ast::TableConstructor(as<ast::FieldInitialization>(fieldList->list())));
				}
			};

	// fieldlist ::= field {fieldsep field} [fieldsep]
	fieldlist = (field & *(-fieldsep & field) & -~fieldsep)
			 >> [] (QSharedPointer<TemporaryPair> node) {
				auto firstField = as<ast::FieldInitialization>(node->left());
				auto temporaryList = as<TemporaryList>(node->right());
				temporaryList->list().prepend(firstField);
				return temporaryList;
			};

	// field ::= ‘[’ exp(0) ‘]’ ‘=’ exp(0) | exp(0) [ ‘=’ exp(0) ]
	field = (-LuaTokenTypes::openingSquareBracket
			& exp
			& -LuaTokenTypes::closingSquareBracket
			& -LuaTokenTypes::equals & exp)
					>> [] (QSharedPointer<TemporaryPair> pair) {
						auto initializer = as<core::ast::Expression>(pair->right());
						auto indexer = as<core::ast::Expression>(pair->left());
						return wrap(new ast::FieldInitialization(initializer, indexer));
					}
			| (exp & ~(-LuaTokenTypes::equals & exp))
					>> [] (QSharedPointer<TemporaryPair> node) {
							auto const left = as<core::ast::Expression>(node->left());
							if (!node->right()) {
								return wrap(new ast::FieldInitialization(left));
							} else {
								auto const initializer = as<core::ast::Expression>(node->right());
								/// @todo Report error if "left" is something different from Name.
								return wrap(new ast::FieldInitialization(left, initializer));
							}
						}
			;

	// fieldsep ::= ‘,’ | ‘;’
	fieldsep = -LuaTokenTypes::comma
			| -LuaTokenTypes::semicolon
			;

	// binop ::= ‘+’ | ‘-’ | ‘*’ | ‘/’ | ‘//’ | ‘^’ | ‘%’ | ‘&’ | ‘~’ | ‘|’ | ‘>>’ | ‘<<’ | ‘..’
	//           | ‘<’ | ‘<=’ | ‘>’ | ‘>=’ | ‘==’ | ‘~=’ | and | or
	binop = LuaTokenTypes::plus >> [] { return new ast::Addition(); }
			| LuaTokenTypes::minus >> [] { return new ast::Subtraction(); }
			| LuaTokenTypes::asterick >> [] { return new ast::Multiplication(); }
			| LuaTokenTypes::slash >> [] { return new ast::Division(); }
			| LuaTokenTypes::doubleSlash >> [] { return new ast::IntegerDivision(); }
			| LuaTokenTypes::hat >> [] { return new ast::Exponentiation(); }
			;

	// unop ::= ‘-’ | not | ‘#’ | ‘~’
	unop = LuaTokenTypes::minus >> [] { return new ast::UnaryMinus(); }
			| LuaTokenTypes::notKeyword >> [] { return new ast::Not(); }
			| LuaTokenTypes::sharp >> [] { return new ast::Length(); }
			| LuaTokenTypes::tilda >> [] { return new ast::BitwiseNegation(); }
			;

	return block.parser();
}
