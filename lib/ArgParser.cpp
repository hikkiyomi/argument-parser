#include "ArgParser.h"

#include <stdexcept>

ArgumentParser::Argument::Argument(char short_name, const std::string& full_name, const std::string& description)
    : short_name_(short_name)
    , full_name_(full_name)
    , description_(description)
{}

ArgumentParser::Argument::Argument(const std::string& full_name, const std::string& description)
    : short_name_('?')
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

    return std::get<std::string>(values_[0]);
}

std::string ArgumentParser::ArgParser::GetStringValue(const std::string& param_name) {
    const Argument& arg = arguments_[index_by_full_name_[param_name]];

    return arg.GetStringValue();
}

ArgumentParser::ArgParser::ArgParser(const std::string& parser_name)
    : parser_name_(parser_name)
{}

bool ArgumentParser::ArgParser::Parse(const std::vector<std::string>& args) {
    if (args.size() == 0) {
        throw std::runtime_error("Zero arguments provided.");
    }

    for (size_t i = 1; i < args.size(); ++i) {

    }

    return required_.empty();
}

ArgumentParser::Argument& ArgumentParser::ArgParser::AddStringArgument(const std::string& param_name) {
    arguments_.emplace_back(param_name);

    if (!CheckOnAvailability(arguments_.back())) {
        throw std::runtime_error("There is a collision between two arguments.\n"
                                 "Use only unique short and full names.\n");
    }

    index_by_full_name_[param_name] = arguments_.size() - 1;
    required_.insert(&arguments_.back());

    return arguments_.back();
}

bool ArgumentParser::ArgParser::CheckOnAvailability(const Argument& arg) {
    return index_by_full_name_.find(arg.GetFullName()) == index_by_full_name_.end()
    && (arg.GetShortName() == '?' || index_by_short_name_.find(arg.GetShortName()) == index_by_short_name_.end());
}
