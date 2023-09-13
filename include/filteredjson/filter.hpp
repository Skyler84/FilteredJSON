#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "json.hpp"

namespace FilteredJSON
{

  enum FilterState{
    DISCARD,
    KEEP,
    CONTINUE,
  };

  class Filter{
  public:
    // static Filter from_string(const std::string &str);
    virtual const Filter *keep(const Value &) const = 0;
    virtual const Filter *keepKey(const std::string &s) const { return nullptr; };
    virtual const Filter *keepIdx(int idx) const { return nullptr; };

  protected:
  private:
  };

  class Identity : public Filter {
  public:
    const Filter *keep(const Value &) const override;
  protected:
  private:
  };

  /**
   * @brief Operates on an array, executing the same filter on each element.
   * The filtered value is an array of each filtered element.
  */
  class Collector final : public Filter {
  public:
    Collector() : m_filter{ std::make_unique<Identity>() } {}
    Collector(std::unique_ptr<Filter> && filter) : m_filter{ std::move(filter) } {}
    const Filter *keep(const Value &value) const override;
    const Filter *keepIdx(int idx) const override { return m_filter.get(); }
    const Filter &filter() const;
  protected:
  private:
    std::unique_ptr<Filter> m_filter;
  };

  class ObjectFilter final : public Filter {
  public:
    bool containsKey(const std::string &s) const;
    const Filter &at(const std::string &s) const;
    const Filter *keepKey(const std::string &key) const override;
  protected:
  private:
    std::unordered_map<std::string, std::unique_ptr<Filter>> m_key_filters;
  };

  class ArrayFilter final : public Filter {
  public:
    bool containsIdx(int i) const;
    const Filter &at(int i) const;
    const Filter *keep(const Value &) const override;
    const Filter *keepIdx(int idx) const override;
  protected:
  private:
    std::unordered_map<int, std::unique_ptr<Filter>> m_idx_filters;
  };
} // namespace FilteredJSON
