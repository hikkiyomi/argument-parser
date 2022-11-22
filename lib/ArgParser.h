#pragma once

#include <cinttypes>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <variant>
#include <vector>

namespace ArgumentParser {
    class Argument {
    public:
        Argument(char short_name, const std::string& full_name, const std::string& description = "");
        Argument(const std::string& full_name, const std::string& description = "");

        char GetShortName() const;
        std::string GetFullName() const;
        std::string GetDescription() const;
        
        std::string GetStringValue() const;
    private:
        char short_name_;
        std::string full_name_;
        std::string description_;
        std::vector<std::variant<int32_t, std::string, bool>> values_;
    };

    class ArgParser {
    public:
        ArgParser(const std::string& parser_name);

        bool Parse(const std::vector<std::string>& args);

        Argument& AddStringArgument(const std::string& param_name);
        std::string GetStringValue(const std::string& param_name);
    private:
        std::string parser_name_;

        std::vector<Argument> arguments_;
        std::unordered_set<Argument*> required_;

        std::unordered_map<char, size_t> index_by_short_name_;
        std::unordered_map<std::string, size_t> index_by_full_name_;

        bool CheckOnAvailability(const Argument& arg);
    };
}
