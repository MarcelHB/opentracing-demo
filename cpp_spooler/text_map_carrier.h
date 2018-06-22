#ifndef H_TEXT_MAP_CARRIER
#define H_TEXT_MAP_CARRIER

#include <opentracing/propagation.h>
#include <unordered_map>

using opentracing::TextMapReader;
using opentracing::TextMapWriter;
using opentracing::expected;
using opentracing::string_view;

class TextMapCarrier : public TextMapReader, public TextMapWriter {
  public:
    TextMapCarrier(std::unordered_map<std::string, std::string>& _map) : map(_map) {}

    expected<void> Set(string_view key, string_view value) const override {
      map[key] = value;

      return {};
    }

    expected<void> ForeachKey(std::function<expected<void>(string_view key, string_view value)> f) const override {
      for (const auto& pair : map) {
        auto result = f(pair.first, pair.second);
        if (!result) {
          return result;
        }
      }

      return {};
    }

  private:
    std::unordered_map<std::string, std::string>& map;
};

#endif
