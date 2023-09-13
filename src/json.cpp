#include "filteredjson/json.hpp"

#define INDENT 2

#include <cassert>

// #define DEBUG_PRINTF(...) printf("[FilteredJSON] " __VA_ARGS__)
#define DEBUG_PRINTF(...) 


using namespace FilteredJSON;

Value::Value() : m_type{Type::Null} {
    DEBUG_PRINTF("null Value()\n");
}
Value::Value(String s) : m_type{Type::String}, s{s} { DEBUG_PRINTF("Value(String)\n"); }
Value::Value(Number n) : m_type{Type::Number}, n{n} { DEBUG_PRINTF("Value(Number)\n"); }
Value::Value(Object o) : m_type{Type::Object}, o{o} { DEBUG_PRINTF("Value(Object)\n"); }
Value::Value(Array a) : m_type{Type::Array}, a{a} { DEBUG_PRINTF("Value(Array)\n"); }
Value::Value(Boolean b) : m_type{Type::Boolean}, b{b} { DEBUG_PRINTF("Value(Boolean)\n"); }
Value::Value(const Value &v) { *this = v; DEBUG_PRINTF("Value(const Value&)\n"); }
Value::Value(Value &&v) { *this = std::move(v); DEBUG_PRINTF("Value(Value&&)\n"); }
Value::~Value(){ DEBUG_PRINTF("~Value()\n"); }
Value &Value::operator=(const Value& v){
    DEBUG_PRINTF("Value=(const Value&)\n");
    nullify();
    m_type = v.m_type;
    switch(m_type){
        case Type::Boolean: b = v.b; break;
        case Type::Number:  n = v.n; break;
        case Type::String:  s = v.s; break;
        case Type::Array:   a = v.a; break;
        case Type::Object:  o = v.o; break;
    }
    return *this;
}

Value &Value::operator=(Value&& v){
    DEBUG_PRINTF("Value=(Value&&)\n");
    nullify();
    m_type = v.m_type;
    switch(m_type){
        case Type::Boolean: b = std::move(v.b); break;
        case Type::Number:  n = std::move(v.n); break;
        case Type::String:  s = std::move(v.s); break;
        case Type::Array:   a = std::move(v.a); break;
        case Type::Object:  o = std::move(v.o); break;
    }
    return *this;
}

void Value::nullify(){
    DEBUG_PRINTF("nullify()\n");
    s = {};
    n = {};
    o = {};
    a = {};
    b = {};
    DEBUG_PRINTF("1nullify() done\n");
}

std::string Value::stringify(int indent) const{
    switch(m_type){
        case Type::Object:  return o.stringify(indent);
        case Type::Array:   return a.stringify(indent);
        case Type::String:  return s.stringify(indent);
        case Type::Number:  return n.stringify(indent);
        case Type::Boolean: return b.stringify(indent);
        case Type::Null: {
            if(indent == -1)
                return "null";
            else
                return std::string(' ', indent) + "null";
        }
    }
    assert(false);
}

Object &Value::toObject() { assert(m_type == Type::Object); return o; }
const Object &Value::toObject() const { assert(m_type == Type::Object); return o; }
Array &Value::toArray() { assert(m_type == Type::Array); return a; }
const Array &Value::toArray() const { assert(m_type == Type::Array); return a; }
String &Value::toString() { assert(m_type == Type::String); return s; }
const String &Value::toString() const { assert(m_type == Type::String); return s; }
Number &Value::toNumber() { assert(m_type == Type::Number); return n; }
const Number &Value::toNumber() const { assert(m_type == Type::Number); return n; }
Boolean &Value::toBoolean() { assert(m_type == Type::Boolean); return b; }
const Boolean &Value::toBoolean() const { assert(m_type == Type::Boolean); return b; }

Object::~Object(){}
Object::Object(const Object &o) : Super{o} { DEBUG_PRINTF("Object(const Object&)\n"); }
Object::Object(Object &&o) : Super{std::move(o)} { DEBUG_PRINTF("Object(Object&&)\n"); }
Value &Object::operator[](std::string_view k) { return Super::operator[](std::string{k}); }
const Value &Object::operator[](std::string_view k) const { return Super::at(std::string{k}); }
Object &Object::operator=(const Object &o) { Super::operator=(o); return *this; }
Object &Object::operator=(Object &&o) { Super::operator=(std::move(o)); return *this; }
std::string Object::stringify(int indent) const{
    DEBUG_PRINTF("Object stringify(%d)\n", indent);
    std::string out;
    if(indent != -1){
        out += std::string(' ', indent);
        out += "{";
        bool b = false;
        if(Super::size())
            out += '\n';
        for(auto &[k, v] : *this){
            if(b) {
                out += ",\n";
            }
            out += v.stringify(indent+INDENT);
            b = true;
        }
        out += "\n";
        out += std::string(' ', indent);
        out += "}";
    } else{
        out += "{";
        bool b = false;
        for(auto &[k, v] : *this){
            DEBUG_PRINTF("Stringifying key: (%s)\n", k.begin());
            if(b) out += ',';
            out += '"';
            out += k;
            out += "\":";
            out += v.stringify(-1);
            b = true;
        }
        out += "}";
    }
    return out;
}

Array::~Array(){};
Array::Array(const Array &a) : m_values{a.m_values} { DEBUG_PRINTF("Array(const Array&)\n"); }
Array::Array(Array &&a) : m_values{std::move(a.m_values)} { DEBUG_PRINTF("Array(Array&&)\n"); }
Value &Array::operator[](int i) { return m_values[i]; }
const Value &Array::operator[](int i) const { return m_values.at(i); }
Array &Array::operator=(const Array &a) { m_values = a.m_values; return *this; }
Array &Array::operator=(Array &&a) { m_values = std::move(a.m_values); return *this; }
Value& Array::append(const Value &v) { m_values.push_back(v); return m_values.back(); }
Value& Array::append() { m_values.push_back({}); return m_values.back(); }
std::string Array::stringify(int indent) const{
    DEBUG_PRINTF("Array stringify(%d)\n", indent);
    std::string out;
    if(indent != -1){
        out += std::string(' ', indent);
        out += "[";
        bool b = false;
        if(m_values.size())
            out += " \n";
        for(auto &v : m_values){
            if(b) {
                out += ",\n";
            }
            out += v.stringify(indent+INDENT);
            b = true;
        }
        if(m_values.size()){
            out += "\n";
            out += std::string(' ', indent);
        }
        out += "]";
    } else{
        out += "[";
        bool b = false;
        for(auto &v : m_values){
            if(b) out += ',';
            out += v.stringify(-1);
            b = true;
        }
        out += "]";
    }
    return out;
}

Boolean::Boolean(bool b) : m_value{b} { DEBUG_PRINTF("Boolean(bool)\n"); }
Boolean::Boolean(const Boolean &b) : m_value{b} { DEBUG_PRINTF("Boolean(const Boolean&)\n"); }
bool Boolean::getValue() const { return m_value; }
Boolean::operator bool() const { return m_value; }
Boolean &Boolean::operator=(bool b) { m_value = b; return *this;}
Boolean &Boolean::operator=(const Boolean &b) { m_value = b; return *this;}
std::string Boolean::stringify(int indent) const{
    DEBUG_PRINTF("Boolean stringify(%d)\n", indent);
    std::string out = m_value?"true":"false";
    if(indent == -1)
        return out;
    else
        return std::string(' ', indent) + out;
}

std::string String::stringify(int indent) const{
    DEBUG_PRINTF("String stringify(%d)\n", indent);
    std::string out;
    out += '"';
    out += m_value;
    out += '"';
    if(indent == -1)
        return out;
    else
        return std::string(' ', indent) + out;
}

Number::IntType Number::asInteger() const { assert(isInteger()); return i; }
Number::FloatType Number::asDouble() const { assert(isDouble()); return d; }
std::string Number::stringify(int indent) const{
    DEBUG_PRINTF("Number stringify(%d)\n", indent);
    std::string out;
    if(m_intNotDouble){
        out = std::to_string(i);
    }else{
        out = std::to_string(d);
    }
    if(indent == -1)
        return out;
    else
        return std::string(' ', indent) + out;
}
