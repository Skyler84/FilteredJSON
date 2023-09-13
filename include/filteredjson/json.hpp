#pragma once

#include <string>
#include <string_view>
#include <map>
#include <vector>
#include <variant>

namespace FilteredJSON
{
    enum class Type{
        String,
        Number,
        Object,
        Array,
        Boolean,
        Null,
    };
    class Value;
    class String;
    class Number;
    class Object;
    class Array;
    class Boolean;
    class Null;

    class Object final : public std::map<std::string, Value>{
    public:
        using Super = std::map<std::string, Value>;
        Object(){}
        ~Object();
        Object(const Object &o);
        Object(Object &&o);
        Value &operator[](std::string_view);
        const Value &operator[](std::string_view) const;
        Object &operator=(const Object &o);
        Object &operator=(Object &&o);
        std::string stringify(int indent) const;
    private:
        // std::map<std::string, Value> m_values;
    };

    class Array final{
    public:
        Array(){}
        ~Array();
        Array(const Array &a);
        Array(Array &&a);
        Value &operator[](int i);
        const Value &operator[](int i) const;
        Array &operator=(const Array &a);
        Array &operator=(Array &&a);
        Value& append(const Value &v);
        Value& append();
        std::string stringify(int indent) const;
    private:
        std::vector<Value> m_values;
    };

    class String final{
    public:
        String() {}
        String(const String &s) : m_value{s.m_value}{}
        String(String &&s) : m_value{std::move(s.m_value)}{}
        String(std::string &&s) : m_value{std::move(s)}{}
        String(std::string_view s) : m_value{s}{}
        std::string_view getValue() const { return m_value; }
        void setValue(std::string_view);
        String& operator=(const String &s) { m_value = s.m_value; return *this; }
        String& operator=(String &&s) { m_value = std::move(s.m_value); return *this; }
        std::string stringify(int indent) const;
        operator std::string_view() const { return m_value; }
    private:
        std::string m_value;
    };

    class Number final{
    public:
        using IntType = long long;
        using FloatType = double;
        Number() : Number{0} {};
        Number(int i) : m_intNotDouble{true}, i{i} {}
        Number(IntType i) : m_intNotDouble{true}, i{i} {}
        Number(FloatType d) : m_intNotDouble{false}, d{d} {}
        bool isInteger() const { return m_intNotDouble; }
        bool isDouble() const { return !m_intNotDouble; }
        IntType asInteger() const;
        FloatType asDouble() const;
        std::string stringify(int indent) const;
    private:
        bool m_intNotDouble;
        union{
            IntType i;
            FloatType d;
        };
    };

    class Boolean final{
    public:
        Boolean(){}
        Boolean(bool v);
        Boolean(const Boolean& b);
        bool getValue() const;
        operator bool() const;
        Boolean& operator=(bool);
        Boolean& operator=(const Boolean &b);
        std::string stringify(int indent) const;
    private:
        bool m_value;
    };

    class Value{
    public:
        bool isString() const { return m_type == Type::String; }
        bool isNumber() const { return m_type == Type::Number; }
        bool isObject() const { return m_type == Type::Object; }
        bool isArray() const { return m_type == Type::Array; }
        bool isBoolean() const { return m_type == Type::Boolean; }
        bool isNull() const { return m_type == Type::Null; }

        String &toString();
        Number &toNumber();
        Object &toObject();
        Array &toArray();
        Boolean &toBoolean();
        const String &toString() const;
        const Number &toNumber() const;
        const Object &toObject() const;
        const Array &toArray() const;
        const Boolean &toBoolean() const;

        Type getType() const { return m_type; }

        Value(String s);
        Value(Number n);
        Value(Object o);
        Value(Array a);
        Value(Boolean b);
        Value();
        Value(const Value &v);
        Value(Value &&v);
        ~Value();
        Value& operator=(const Value& v);
        Value& operator=(Value&& v);
        operator bool() const { return !isNull(); }

        std::string stringify(int indent) const;
    protected:
        Value(Type t);
    private:
        Type m_type;
        // union Values{
            String s;
            Number n;
            Object o;
            Array a;
            Boolean b;
        //     ~Values(){}
        // } u;
        void nullify();
    };
    
} // namespace FilteredJSON
