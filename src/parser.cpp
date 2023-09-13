#include "filteredjson/parser.hpp"

#include "assert.h"
#include <cmath>

// #define DEBUG_PRINTF(...) printf("[JP] " __VA_ARGS__)
// #define DEBUG_PRINTF2(...) printf( __VA_ARGS__)
#define DEBUG_PRINTF(...)
#define DEBUG_PRINTF2(...)


using namespace FilteredJSON;
#define elem(x) [((int)Parser::State::x)] = #x

const char* Parser::state_strs[] = {
    elem(Start),
    elem(ObjectOpen),
    elem(ObjectKey),
    elem(ObjectColon),
    elem(ObjectValue),
    elem(ObjectComma),
    elem(ArrayOpen),
    elem(ArrayValue),
    elem(ArrayComma),
    elem(String),
    elem(Number),
    elem(TrueStart),
    elem(FalseStart),
    elem(NullStart),
    elem(Error),
    elem(Stop),
};
void Parser::printStateStack(){


    DEBUG_PRINTF("State Stack: ");
    for(auto e : state){
        DEBUG_PRINTF2("%s,", state_strs[(int)e]);
    }
    DEBUG_PRINTF2("\n");
}

Parser::Parser(){
    DEBUG_PRINTF("Parser()\n");
    reset();
}

bool Parser::isValid() const{
    return currentState() == State::Stop && !error;
}

Value &Parser::getValue() {
    assert(isValid());
    return rootValue;
}

void Parser::reset(){
    //reset the parser internal states
    DEBUG_PRINTF("reset()\n");
    while(state.size()){
        DEBUG_PRINTF("popping state\n");
        state.pop_back();
    }
    state.push_back(State::Start);
    DEBUG_PRINTF("state set\n");
    while(branch.size()){
        DEBUG_PRINTF("popping branch\n");
        branch.pop_back();
    }
    branch.push_back(&rootValue);
    DEBUG_PRINTF("root branch set\n");
    rootValue = {};
    DEBUG_PRINTF("reset() done\n");
}

void Parser::parseContinue(std::string_view data){
    //as long as there is data to parse
    //call functions based on State name/currentState()
    while(data.length()){
        // DEBUG_PRINTF("parsing remaining data, len: %d\n", data.length());
        switch(currentState()){
            case State::Start:          parseStart(data); break;
            case State::ObjectOpen:     parseObjectOpen(data); break;
            case State::ObjectKey:      parseObjectKey(data); break;
            case State::ObjectColon:    parseObjectColon(data); break;
            case State::ObjectValue:    parseObjectValue(data); break;
            case State::ObjectComma:    parseObjectComma(data); break;
            case State::ArrayOpen:      parseArrayOpen(data); break;
            case State::ArrayValue:     parseArrayValue(data); break;
            case State::ArrayComma:     parseArrayComma(data); break;
            case State::String:         parseString(data); break;
            case State::Number:         parseNumber(data); break;
            case State::TrueStart:      parseTrue(data); break;
            case State::FalseStart:     parseFalse(data); break;
            case State::NullStart:      parseNull(data); break;
            case State::Stop:           parseStop(data); break;
            default: 
            printf("missing case statement for: %s (%d)\n", state_strs[(int)currentState()], int(currentState()));
            assert(false && "Missing case statement for parsing ");
            ;
        }
    }
}

void Parser::fail(){
    //set the error state
    error = true;
    pushState(State::Error);
    DEBUG_PRINTF("Error parsing\n");
    assert(false);
}

void Parser::consumeWhitespace(std::string_view &data){
    //consume as many WS chars in sequence
    while(isspace(data[0]) && data.length())
        data = {data.begin()+1, data.end()};
}

bool Parser::consumeChar(std::string_view &data, char c){
    //conditional consume char
    //if data[0] == c then consume and return true
    //if not or no chars then return false
    bool out = data[0] == c;
    if(!out || !data.length())
        return false;
    data = {data.begin()+1, data.end()};
    return true;
}

char Parser::consumeChar(std::string_view &data){
    //consume a single char from data
    //return that char
    //if no chars, return '\0'
    char c = data[0];
    if(data.length())
        return data = {data.begin()+1, data.end()}, c;
    return 0;
}

void Parser::parseStart(std::string_view &data){
    //parse start of FilteredJSON
    //WS first
    //then FilteredJSON::Value
    //replace Start state with Stop
    consumeWhitespace(data);
    if(!data.length())
        return;
    popState(/*Start*/);
    pushState(State::Stop);
    if(!tryParseValue(data))
        assert(false);
}

void Parser::parseStop(std::string_view &data){
    //WS at end of parsing
    //fail() if any non-WS
    consumeWhitespace(data);
    if(!data.length())
        return;
    fail();
    assert(false);
}

void Parser::parseObjectOpen(std::string_view &data){
    // '{' found, pop ObjectOpen state
    // look for '}' or parse ObjectKey string after WS
    // clear token if objectKey
    consumeWhitespace(data);
    if(!data.length())
        return;
    popState(/*ObjectOpen*/);
    if(consumeChar(data, '"')){
        DEBUG_PRINTF("parsing ObjectKey (String)\n");
        pushState(State::ObjectKey);
        pushState(State::String);
        token.clear();
    }else if(consumeChar(data, '}')){
        DEBUG_PRINTF("Empty object {}\n");
        // DEBUG_PRINTF("branch popping (%d)\n", branch.size());
        // branch.pop();
    }else{
        fail();
    }
}

void Parser::parseObjectKey(std::string_view &data){
    // ObjectKey parsed and in token
    // pop ObjectKey state
    // check nekt char is ':'
    // then push new FilteredJSON::Value to branch stack
    // assign to current value (which is an object)
    // indeked by key (token)
    consumeWhitespace(data);
    if(!data.length())
        return;
    popState(/*ObjectKey*/);
    if(consumeChar(data, ':')){
        DEBUG_PRINTF("parsing ObjectColon\n");
        assert(currentValue().isObject());
        auto &v = currentValue().toObject()[token] = {};
        branch.push_back(&v);
        DEBUG_PRINTF("branch pushed (%d) new object elem\n", branch.size());
        pushState(State::ObjectColon);
        token.clear();
    }else{
        fail();
    }
}

void Parser::parseObjectColon(std::string_view &data){
    // ':' read, parse Value next
    consumeWhitespace(data);
    if(!data.length())
        return;
    popState(/*ObjectColon*/);
    DEBUG_PRINTF("parsing ObjectValue\n");
    pushState(State::ObjectValue);
    if(tryParseValue(data));
    else{
        fail();
    }
}

void Parser::parseObjectValue(std::string_view &data){
    //pop ObjectValue state
    //current Object value has been parsed and set in currentValue()
    //pop current branch FilteredJSON value
    //check if nekt char is ',' or '}'
    //if ',', push ObjectComma state
    consumeWhitespace(data);
    if(!data.length())
        return;
    popState(/*ObjectValue*/);
    DEBUG_PRINTF("branch popping (%d) object elem done\n", branch.size());
    branch.pop_back();
    if(consumeChar(data, '}')){
        // popState(/*ObjectOpen*/);
    }else if(consumeChar(data, ',')){
        pushState(State::ObjectComma);
    }else{
        fail();
    }
}

void Parser::parseObjectComma(std::string_view &data){
    //pop ObjectComma state
    //parse next ObjectKey string (push statet)
    //ie if not '"', fail()
    consumeWhitespace(data);
    if(!data.length())
        return;
    popState(/*ObjectComma*/);
    if(consumeChar(data, '"')){
        DEBUG_PRINTF("parsing ObjectKey (String)\n");
        pushState(State::ObjectKey);
        pushState(State::String);
        token.clear();
    }else{
        fail();
    }
}

void Parser::parseArrayOpen(std::string_view &data){
    //pop ArrayOpen state
    //check if ']' after WS
    //otherwise try parse value and push ArrayValue state
    consumeWhitespace(data);
    if(!data.length())
        return;
    popState(/*ArrayOpen*/);
    if(consumeChar(data, ']')){
        // DEBUG_PRINTF("branch popping (%d) array close?\n", branch.size());
        // branch.pop();
    }else{
        pushState(State::ArrayValue);
        assert(currentValue().isArray());
        Array & a = currentValue().toArray();
        Value &v = a.append();
        branch.push_back(&v);
        DEBUG_PRINTF("branch pushed (%d) new array elem\n", branch.size());
        if(tryParseValue(data)){
            DEBUG_PRINTF("Array got value\n");
        }else{
            fail();
        }
    }
}

void Parser::parseArrayValue(std::string_view &data){
    //array value has been parsed
    //pop state
    //pop current FilteredJSON value (branch)
    //checx if comma 
    //if comma, push new FilteredJSON value (null) and append to Array
    consumeWhitespace(data);
    if(!data.length())
        return;
    popState(/*ArrayValue*/);
    DEBUG_PRINTF("branch popping (%d) array elem done\n", branch.size());
    branch.pop_back();
    if(consumeChar(data, ',')){
        pushState(State::ArrayComma);
        assert(currentValue().isArray());
        Array & a = currentValue().toArray();
        Value &v = a.append();
        branch.push_back(&v);
        DEBUG_PRINTF("branch pushed (%d) next array elem\n", branch.size());
    }else if(consumeChar(data, ']')){
        // popState()
    }else{
        fail();
    }

}

void Parser::parseArrayComma(std::string_view &data){
    //pop ArrayComma state
    //parse next array value
    consumeWhitespace(data);
    if(!data.length())
        return;
    popState(/*ArrayComma*/);
    pushState(State::ArrayValue);
    if(!tryParseValue(data)){
        assert(false);
    }
}

void Parser::parseString(std::string_view &data){
    //parse string until '"'
    //handle escape char '\'
    //put string into tok
    //once complete, if currentValue().isString() then set it to tok
    //otherwise leave in tok
    while(data.length()){
        if(consumeChar(data, '"')){
            popState();
            DEBUG_PRINTF("Got string: ***%s***\n", token.c_str());
            if(currentValue().isString())
                currentValue() = String(token);
            // token.clear();
            break;
        }
        if(consumeChar(data, '\\')){
            if(consumeChar(data, '\\'))
                token += '\\';
            else if(consumeChar(data, '"'))
                token += '"';
            else if(consumeChar(data, '/'))
                token += '/';
            else if(consumeChar(data, 'b'))
                token += '\b';
            else if(consumeChar(data, 'f'))
                token += '\f';
            else if(consumeChar(data, 'n'))
                token += '\n';
            else if(consumeChar(data, 'r'))
                token += '\r';
            else if(consumeChar(data, 't'))
                token += '\t';
            else if(consumeChar(data, 'u')){
                assert(false &&"Unhandled unicode characters in JSON");
                int v = std::stoi(std::string{data.begin(), 4}, nullptr, 16);
                data = {&data[4], data.end()};
                token += '_';
            }
        }
        // DEBUG_PRINTF("str c '%c'\n", data[0]);
        token += consumeChar(data);
    }
}

bool Parser::tryParseValue(std::string_view &data){
    //try to parse each of the possible JSON data types
    //if none match, return false
    //if match any, return true
    //on match, push state to begin parsing of that type
    //set the current value to JSON type
    //clear token if needed
    if(!data.length())
        return false;
    if(consumeChar(data, '"')){
        pushState(State::String);
        DEBUG_PRINTF("parsing String (set %d)\n", branch.size());
        currentValue() = String{};
        token.clear();
    }else if(consumeChar(data, '{')){
        pushState(State::ObjectOpen);
        DEBUG_PRINTF("parsing Object (set %d)\n", branch.size());
        currentValue() = Object{};
    }else if(consumeChar(data, '[')){
        pushState(State::ArrayOpen);
        DEBUG_PRINTF("parsing Array (set %d)\n", branch.size());
        currentValue() = Array{};
    }else if(consumeChar(data, 't')){
        token = 't';
        pushState(State::TrueStart);
        DEBUG_PRINTF("parsing True (set %d)\n", branch.size());
        currentValue() = Boolean{};
    }else if(consumeChar(data, 'f')){
        token = 'f';
        pushState(State::FalseStart);
        DEBUG_PRINTF("parsing False (set %d)\n", branch.size());
        currentValue() = Boolean{};
    }else if(consumeChar(data, 'n')){
        token = 'n';
        pushState(State::NullStart);
        DEBUG_PRINTF("parsing Null (set %d)\n", branch.size());
        currentValue() = {};
    }else if(data[0] >= '0' && data[0] <= '9' || data[0] == '-'){
        pushState(State::Number);
        DEBUG_PRINTF("parsing Number (set %d)\n", branch.size());
        token = consumeChar(data);
        currentValue() = Number{};
    }else{
        return false;
    }
    return true;
}

void Parser::parseNumber(std::string_view &data){
    //keep accepting chars as long as they match [0-9\.eE]+
    //otherwise fail()
    //pop state and set current value (converted from string to int/double as appropriate) on success
    while(data.length()){
        if(!(data[0] >= '0' && data[0] <= '9' || data[0] == '.' || data[0] == 'e' || data[0] == 'E')){
            popState();
            //check if integer
            bool isInt = true;
            for(auto c : token){
                if(!isdigit(c))
                    isInt = false;
            }
            if(isInt){
                printf("Parsing nuber: %s\n", token.c_str());
                currentValue() = Number{std::stoll(token)};
                DEBUG_PRINTF("Int parsed (set %d)\n", branch.size());
            }else{
                auto sv = token.operator std::basic_string_view<char, std::char_traits<char>>();
                size_t eIdx = token.find_first_of("eE");
                std::string mantissa;
                int exp = 0;
                if(eIdx == -1){
                    mantissa = token;
                }
                else{
                    mantissa = {sv.begin(), &sv[eIdx]};
                    std::string exp_str = {&sv[eIdx], sv.end()};
                    if(exp_str.find_first_of("eE.") != -1)
                        fail();
                    printf("Parsing exponent: %s\n", token.c_str());
                    exp = std::stoi(exp_str);

                }
                // std::string_view exponent = {token.begin(), eIdx};
                double d = std::stod(mantissa);
                d *= std::pow(10, exp);
                currentValue() = Number{d};
                DEBUG_PRINTF("Double parsed (set %d)\n", branch.size());
            }
            
            // DEBUG_PRINTF("Number not parsed! (%s)\n", token.c_str());
            break;
        }
        token += consumeChar(data);
    }
}

void Parser::parseTrue(std::string_view &data){
    //keep accepting chars as long as they match "true"
    //otherwise fail()
    //pop state and set current value on success
    auto _true = std::string_view{"true"};
    while(data.length()){
        if(token == _true){
            currentValue() = Boolean{true};
                DEBUG_PRINTF("True parsed (set %d)\n", branch.size());
            popState(/*TrueStart*/);
            break;
        } else if(_true.starts_with(token)) {
            token += consumeChar(data);
        }else{
            assert(false);
            fail();
        }
    }
}

void Parser::parseFalse(std::string_view &data){
    //keep accepting chars as long as they match "false"
    //otherwise fail()
    //pop state and set current value on success
    auto _false = std::string_view{"false"};
    while(data.length()){
        if(token == _false){
            currentValue() = Boolean{false};
            DEBUG_PRINTF("False parsed (set %d)\n", branch.size());
            popState(/*FalseStart*/);
            break;
        } else if(_false.starts_with(token)) {
            token += consumeChar(data);
        }else{
            assert(false);
            fail();
        }
    }
}

void Parser::parseNull(std::string_view &data){
    //keep accepting chars as long as they match "null"
    //otherwise fail()
    //pop state and set current value on success
    auto _null = std::string_view{"null"};
    while(data.length()){
        if(token == _null){
            currentValue() = {};
            DEBUG_PRINTF("Null parsed (set %d)\n", branch.size());
            popState(/*NullStart*/);
            break;
        } else if(_null.starts_with(token)) {
            token += consumeChar(data);
        }else{
            assert(false);
            fail();
        }
    }
}