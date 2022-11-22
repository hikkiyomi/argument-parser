#pragma once

#include <cinttypes>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <variant>
#include <vector>

namespace ArgumentParser {
    enum ArgumentType {
        kInteger,
        kString,
        kFlag
    };

    class Argument {
    public:
        Argument(const ArgumentType& type, char short_name, const std::string& full_name, const std::string& description = "");
        Argument(const ArgumentType& type, const std::string& full_name, const std::string& description = "");

        char GetShortName() const;
        std::string GetFullName() const;
        std::string GetDescription() const;
        
        std::string GetStringValue(size_t index = 0) const;
        int32_t GetIntValue(size_t index = 0) const;

        void AddValue(const std::string&);

        Argument& Default(const std::variant<int32_t, std::string, bool>& default_value);
        Argument& MultiValue(size_t min_args_count = 0);

        Argument& StoreValue(int32_t& value_storage);
        Argument& StoreValue(std::string& value_storage);
        Argument& StoreValue(bool& value_storage);
        
        Argument& StoreValues(std::vector<int32_t>& value_storage);
        Argument& StoreValues(std::vector<std::string>& value_storage);

        void UpdateStorage() const;
        bool Check() const;
    private:
        ArgumentType type_;

        char short_name_;
        std::string full_name_;
        std::string description_;

        std::vector<std::string> values_;
        
        bool storage_awaken_;

        std::variant<int32_t*, std::string*, bool*, std::nullptr_t> storage_;

        bool multi_value_;
        uint32_t min_args_count_;
        std::variant<std::vector<int32_t>*, std::vector<std::string>*, std::nullptr_t> multi_storage_;
    };

    class ArgParser {
    public:
        ArgParser(const std::string& parser_name);

        bool Parse(const std::vector<std::string>& args);

        Argument& AddStringArgument(char short_name, const std::string& full_name);
        Argument& AddStringArgument(const std::string& full_name);
        std::string GetStringValue(const std::string& full_name, size_t index = 0);

        Argument& AddIntArgument(char short_name, const std::string& full_name);
        Argument& AddIntArgument(const std::string& full_name);
        int32_t GetIntValue(const std::string& full_name, size_t index = 0);
    private:
        std::string parser_name_;

        std::vector<Argument> arguments_;
        std::vector<std::string> positional_;

        std::unordered_map<char, size_t> index_by_short_name_;
        std::unordered_map<std::string, size_t> index_by_full_name_;

        bool CheckOnAvailability(const Argument& arg) const;
        bool CheckValues() const;
        void UpdateStorages() const;
    };
}
