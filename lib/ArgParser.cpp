#include "ArgParser.h"

#include <cassert>
#include <stdexcept>

ArgumentParser::Argument::Argument(const ArgumentType& type, char short_name, const std::string& full_name, const std::string& description)
    : type_(type)
    , short_name_(short_name)
    , full_name_(full_name)
    , description_(description)
{}

ArgumentParser::Argument::Argument(const ArgumentType& type, const std::string& full_name, const std::string& description)
    : type_(type)
    , short_name_('?')
    , full_name_(full_name)
    , description_(description)
{}

char ArgumentParser::Argument::GetShortName() const {
    return short_name_;
}

std::string ArgumentParser::Argument::GetFullName() const {
    return full_name_;
}

std::string ArgumentParser::Argument::GetDescription() const {
    return description_;
}

std::string ArgumentParser::Argument::GetStringValue() const {
    if (values_.size() != 1) {
        throw std::runtime_error("Argument " + full_name_ + " does not contain exactly one value.");
    }

    if (type_ != ArgumentType::kString) {
        throw std::runtime_error("Argument " + full_name_ + " does not contain string type.");
    }

    return values_[0];
}

bool ArgumentParser::Argument::Check() const {
    return !values_.empty();
}

void ArgumentParser::Argument::AddValue(const std::string& value) {
    values_.emplace_back(value);
}

ArgumentParser::Argument& ArgumentParser::Argument::Default(std::variant<int32_t, std::string, bool> default_value) {
    if (type_ == ArgumentType::kInteger) {
        values_ = {std::to_string(std::get<int32_t>(default_value))};
    } else if (type_ == ArgumentType::kString) {
        values_ = {std::get<std::string>(default_value)};
    } else {
        assert(type_ == ArgumentType::kFlag);
        
        bool value = std::get<bool>(default_value);

        if (value) {
            values_ = {"1"};
        } else {
            values_ = {"0"};
        }
    }

    return *this;
}

std::string ArgumentParser::ArgParser::GetStringValue(const std::string& full_name) {
    const Argument& arg = arguments_[index_by_full_name_[full_name]];

    return arg.GetStringValue();
}

ArgumentParser::ArgParser::ArgParser(const std::string& parser_name)
    : parser_name_(parser_name)
{}

std::vector<std::string> ParseMonoOption(const std::string& arg) {
    std::vector<std::string> result;
    std::string buffer;
    bool equal_sign_seen = false;

    for (size_t i = 0; i < arg.size(); ++i) {
        if (arg[i] == '=' && !equal_sign_seen) {
            result.push_back(buffer);
            buffer.clear();

            equal_sign_seen = true;
        } else {
            buffer += arg[i];
        }
    }

    if (!buffer.empty()) {
        result.push_back(buffer);
    }

    return result;
}

bool ArgumentParser::ArgParser::Parse(const std::vector<std::string>& args) {
    if (args.size() == 0) {
        throw std::runtime_error("Zero arguments provided.");
    }

    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i][0] == '-') {
            if (args[i].size() < 2) {
                throw std::runtime_error("Wrong argument: " + args[i]);
            }

            bool full_name_argument = true;

            if (args[i][1] != '-') {
                full_name_argument = false;
            }

            std::vector<std::string> params;

            if (i == args.size() - 1 || args[i + 1][0] == '-') {
                params = ParseMonoOption(args[i]);
            } else {
                params = {args[i], args[i + 1]};
            }

            if (full_name_argument) {
                params[0] = params[0].substr(2);

                size_t index = index_by_full_name_[params[0]];

                arguments_[index].AddValue(params[1]);
            } else {
                params[0] = params[0].substr(1);

                if (params[0].size() != 1) {
                    throw std::runtime_error("Wrong argument: -" + params[0]);
                }

                size_t index = index_by_short_name_[params[0][0]];

                arguments_[index].AddValue(params[1]);
            }
        } else {
            positional_.emplace_back(args[i]);
        }
    }

    return CheckValues();
}

ArgumentParser::Argument& ArgumentParser::ArgParser::AddStringArgument(char short_name, const std::string& full_name) {
    arguments_.emplace_back(ArgumentType::kString, short_name, full_name);

    if (!CheckOnAvailability(arguments_.back())) {
        throw std::runtime_error("There is a collision between two arguments.\n"
                                 "Use only unique short and full names.\n");
    }

    index_by_short_name_[short_name] = arguments_.size() - 1;
    index_by_full_name_[full_name] = arguments_.size() - 1;

    return arguments_.back();
}

ArgumentParser::Argument& ArgumentParser::ArgParser::AddStringArgument(const std::string& full_name) {
    arguments_.emplace_back(ArgumentType::kString, full_name);

    if (!CheckOnAvailability(arguments_.back())) {
        throw std::runtime_error("There is a collision between two arguments.\n"
                                 "Use only unique short and full names.\n");
    }

    index_by_full_name_[full_name] = arguments_.size() - 1;

    return arguments_.back();
}

bool ArgumentParser::ArgParser::CheckOnAvailability(const Argument& arg) const {
    return index_by_full_name_.find(arg.GetFullName()) == index_by_full_name_.end()
    && (arg.GetShortName() == '?' || index_by_short_name_.find(arg.GetShortName()) == index_by_short_name_.end());
}

bool ArgumentParser::ArgParser::CheckValues() const {
    for (const auto& arg: arguments_) {
        if (!arg.Check()) {
            return false;
        }
    }

    return true;
}
