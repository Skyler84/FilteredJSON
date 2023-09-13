#include "filteredjson/filter.hpp"

using namespace FilteredJSON;

const Filter *Identity::keep(const Value&) const {
  return this;
}

const Filter *Collector::keep(const Value &value) const {
  if (value.isArray())
    return this;
  else
    return nullptr;
}

// const Filter *