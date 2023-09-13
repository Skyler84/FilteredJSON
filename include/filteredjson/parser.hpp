#pragma once

#include "json.hpp"

#include <string_view>
#include <stack>

namespace FilteredJSON
{
    class Parser{
    public:
        Parser();
        bool isValid() const;
        Value &getValue();
        void reset();
        void parseContinue(std::string_view data);
    protected:
    private:
        enum class State{
            Start,
            ObjectOpen,     //> '{' found, eat WS until '"' or '}
            ObjectKey,      //> '"' found, parse string until '"' and eat WS
            ObjectColon,    //> ':' found, eat WS until or '"'
            ObjectValue,    //> Parsing value. once complete read ',' or '}'
            ObjectComma,    //> ',' found, eat WS until ObjectKey
            ArrayOpen,      //> '[' found, eat WS until parse value or ']' 
            ArrayValue,     //> Parsing value. once complete read ',' or ']'
            ArrayComma,     //> Read ',', parse value after WS
            String,         //> '"' found, parse until '"' and eat WS
            Number,         //> Reading Int/Float number into token
            TrueStart,      //> Reading "true" into token
            FalseStart,     //> Reading "false" into token
            NullStart,      //> Reading "null"
            Error,          //> Error in parsing
            Stop,           //> Root value parsed, only whitespace left
        };
        std::vector<State> state;
        std::vector<Value*> branch;
        bool error = false;
        Value rootValue;
        std::string token;

        State popState() { State s = state.back(); state.pop_back(); printStateStack(); return s; }
        void pushState(State s) { state.push_back(s); printStateStack(); }

        State currentState() const { return state.back(); }
        Value &currentValue() const { return *branch.back(); }
        void fail();
        void consumeWhitespace(std::string_view &data);
        bool consumeChar(std::string_view &data, char c);
        char consumeChar(std::string_view &data);
        void parseStart(std::string_view &data);
        void parseStop(std::string_view &data);
        void parseObjectOpen(std::string_view &data);
        void parseObjectKey(std::string_view &data);
        void parseObjectColon(std::string_view &data);
        void parseObjectValue(std::string_view &data);
        void parseObjectComma(std::string_view &data);
        void parseArrayOpen(std::string_view &data);
        void parseArrayValue(std::string_view &data);
        void parseArrayComma(std::string_view &data);
        void parseString(std::string_view &data);
        bool tryParseValue(std::string_view &data);
        void parseNumber(std::string_view &data);
        void parseTrue(std::string_view &data);
        void parseFalse(std::string_view &data);
        void parseNull(std::string_view &data);

        void printStateStack();

        static const char* state_strs[];
    };
} // namespace FilteredJSON
