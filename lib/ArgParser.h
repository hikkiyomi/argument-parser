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
        Argument();

        Argument(const ArgumentType& type, char short_name, const std::string& full_name, const std::string& description = "");
        Argument(const ArgumentType& type, const std::string& full_name, const std::string& description = "");

        char GetShortName() const;
        std::string GetFullName() const;
        std::string GetDescription() const;
        ArgumentType GetType() const;
        
        std::string GetStringValue(size_t index = 0) const;
        int32_t GetIntValue(size_t index = 0) const;
        bool GetFlag() const;

        void AddValue(const std::string&);

        Argument& Default(const std::variant<int32_t, std::string, bool>& default_value);
        Argument& MultiValue(size_t min_args_count = 0);
        Argument& Positional();

        Argument& StoreValue(int32_t& value_storage);
        Argument& StoreValue(std::string& value_storage);
        Argument& StoreValue(bool& value_storage);
        
        Argument& StoreValues(std::vector<int32_t>& value_storage);
        Argument& StoreValues(std::vector<std::string>& value_storage);

        bool Check() const;
        void UpdateStorage() const;
        void TakePositionals(const std::vector<std::string>& positionals);
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

        bool takes_positional_;
    };

    class ArgParser {
    public:
        ArgParser(const std::string& parser_name);

        bool Parse(const std::vector<std::string>& args);

        Argument& AddStringArgument(char short_name, const std::string& full_name, const std::string& description = "");
        Argument& AddStringArgument(const std::string& full_name, const std::string& description = "");
        std::string GetStringValue(const std::string& full_name, size_t index = 0);

        Argument& AddIntArgument(char short_name, const std::string& full_name, const std::string& description = "");
        Argument& AddIntArgument(const std::string& full_name, const std::string& description = "");
        int32_t GetIntValue(const std::string& full_name, size_t index = 0);

        Argument& AddFlag(char short_name, const std::string& full_name, const std::string& description = "");
        Argument& AddFlag(const std::string& full_name, const std::string& description = "");
        bool GetFlag(const std::string& full_name);

        void AddHelp(char short_help, const std::string& full_help, const std::string& description = "");
        bool Help();
    private:
        std::string parser_name_;
        
        bool help_called_;
        char short_help_;
        std::string full_help_;
        std::string description_;

        std::string help_of_all_parser_;

        std::vector<Argument> arguments_;
        std::vector<std::string> positional_;

        std::unordered_map<char, size_t> index_by_short_name_;
        std::unordered_map<std::string, size_t> index_by_full_name_;

        size_t GetIndex(char short_name);
        size_t GetIndex(const std::string& full_name);

        bool CheckOnAvailability(const Argument& arg) const;
        bool CheckValues() const;
        void UpdateStorages() const;
        void TakePositionals();
    };
}
