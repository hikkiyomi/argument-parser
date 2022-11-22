#include "ArgParser.h"

#include <cassert>
#include <stdexcept>

ArgumentParser::Argument::Argument() {}

ArgumentParser::Argument::Argument(const ArgumentType& type, char short_name, const std::string& full_name, const std::string& description)
    : type_(type)
    , short_name_(short_name)
    , full_name_(full_name)
    , description_(description)
    , storage_awaken_(false)
    , storage_(nullptr)
    , multi_value_(false)
    , min_args_count_(0)
    , multi_storage_(nullptr)
{}

ArgumentParser::Argument::Argument(const ArgumentType& type, const std::string& full_name, const std::string& description)
    : type_(type)
    , short_name_('?')
    , full_name_(full_name)
    , description_(description)
    , storage_awaken_(false)
    , storage_(nullptr)
    , multi_value_(false)
    , min_args_count_(0)
    , multi_storage_(nullptr)
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

ArgumentParser::ArgumentType ArgumentParser::Argument::GetType() const {
    return type_;
}

std::string ArgumentParser::Argument::GetStringValue(size_t index) const {
    if (type_ != ArgumentType::kString) {
        throw std::runtime_error("Argument " + full_name_ + " does not contain string type.");
    }

    return values_[index];
}

int32_t ArgumentParser::Argument::GetIntValue(size_t index) const {
    if (type_ != ArgumentType::kInteger) {
        throw std::runtime_error("Argument " + full_name_ + " does not contain integer type.");
    }

    return std::stoi(values_[index]);
}

bool ArgumentParser::Argument::GetFlag() const {
    if (type_ != ArgumentType::kFlag) {
        throw std::runtime_error("Argument " + full_name_ + " does not contain boolean type.");
    }

    if (values_.size() != 1) {
        throw std::runtime_error("Flag " + full_name_ + " does not have a value.");
    }

    return (values_[0] == "1" ? true : false);
}

bool ArgumentParser::Argument::Check() const {
    return values_.size() > min_args_count_;
}

void ArgumentParser::Argument::AddValue(const std::string& value) {
    if (type_ == ArgumentType::kFlag && values_.size() == 1) {
        values_[0] = value;

        return;
    }

    values_.emplace_back(value);
}

ArgumentParser::Argument& ArgumentParser::Argument::Default(const std::variant<int32_t, std::string, bool>& default_value) {
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

ArgumentParser::Argument& ArgumentParser::Argument::MultiValue(size_t min_args_count) {
    multi_value_ = true;
    min_args_count_ = min_args_count;

    return *this;
}

ArgumentParser::Argument& ArgumentParser::Argument::StoreValue(int32_t& value_storage) {
    if (type_ != ArgumentType::kInteger) {
        throw std::runtime_error("Cannot put integer value into non-integer variable.");
    }

    storage_ = &value_storage;
    storage_awaken_ = true;

    return *this;
}

ArgumentParser::Argument& ArgumentParser::Argument::StoreValue(std::string& value_storage) {
    if (type_ != ArgumentType::kString) {
        throw std::runtime_error("Cannot put string value into non-string variable.");
    }

    storage_ = &value_storage;
    storage_awaken_ = true;

    return *this;
}

ArgumentParser::Argument& ArgumentParser::Argument::StoreValue(bool& value_storage) {
    if (type_ != ArgumentType::kFlag) {
        throw std::runtime_error("Cannot put boolean value into non-flag variable.");
    }

    storage_ = &value_storage;
    storage_awaken_ = true;

    return *this;
}

ArgumentParser::Argument& ArgumentParser::Argument::StoreValues(std::vector<int32_t>& value_storage) {
    if (type_ != ArgumentType::kInteger) {
        throw std::runtime_error("Cannot put integer value into non-integer variable.");
    }

    multi_storage_ = &value_storage;
    storage_awaken_ = true;

    return *this;
}

ArgumentParser::Argument& ArgumentParser::Argument::StoreValues(std::vector<std::string>& value_storage) {
    if (type_ != ArgumentType::kString) {
        throw std::runtime_error("Cannot put string value into non-string variable.");
    }

    multi_storage_ = &value_storage;
    storage_awaken_ = true;

    return *this;
}

void ArgumentParser::Argument::UpdateStorage() const {
    if (!storage_awaken_) {
        return;
    }

    if (type_ == ArgumentType::kInteger) {
        if (multi_value_) {
            std::vector<int32_t>* storage_pointer = std::get<std::vector<int32_t>*>(multi_storage_);
            
            for (const auto& value: values_) {
                (*storage_pointer).emplace_back(std::stoi(value));
            }
        } else {
            int32_t* storage_pointer = std::get<int32_t*>(storage_);
            
            *storage_pointer = std::stoi(values_[0]);
        }
    } else if (type_ == ArgumentType::kString) {
        if (multi_value_) {
            std::vector<std::string>* storage_pointer = std::get<std::vector<std::string>*>(multi_storage_);

            *storage_pointer = values_;
        } else {
            std::string* storage_pointer = std::get<std::string*>(storage_);
        
            *storage_pointer = values_[0];
        }
    } else {
        assert(type_ == ArgumentType::kFlag);
        
        if (multi_value_) {
            throw std::runtime_error("Flag cannot have multiple values.");
        } else {
            bool* storage_pointer = std::get<bool*>(storage_);
        
            if (values_[0] == "0") {
                *storage_pointer = false;
            } else {
                assert(values_[0] == "1");

                *storage_pointer = true;
            }
        }
    }
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

    if (result.size() < 2) {
        result.emplace_back("");
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

            params = ParseMonoOption(args[i]);

            if (params[1].empty()) {
                params[1] = "1";
            }

            if (full_name_argument) {
                params[0] = params[0].substr(2);
                arguments_[GetIndex(params[0])].AddValue(params[1]);
            } else {
                params[0] = params[0].substr(1);

                for (auto short_name: params[0]) {
                    arguments_[GetIndex(short_name)].AddValue(params[1]);
                }
            }
        } else {
            positional_.emplace_back(args[i]);
        }
    }

    if (!CheckValues()) {
        return false;
    }

    UpdateStorages();

    return true;
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

std::string ArgumentParser::ArgParser::GetStringValue(const std::string& full_name, size_t index) {
    const Argument& arg = arguments_[GetIndex(full_name)];

    return arg.GetStringValue(index);
}

ArgumentParser::Argument& ArgumentParser::ArgParser::AddIntArgument(char short_name, const std::string& full_name) {
    arguments_.emplace_back(ArgumentType::kInteger, short_name, full_name);

    if (!CheckOnAvailability(arguments_.back())) {
        throw std::runtime_error("There is a collision between two arguments.\n"
                                 "Use only unique short and full names.\n");
    }

    index_by_short_name_[short_name] = arguments_.size() - 1;
    index_by_full_name_[full_name] = arguments_.size() - 1;

    return arguments_.back();
}

ArgumentParser::Argument& ArgumentParser::ArgParser::AddIntArgument(const std::string& full_name) {
    arguments_.emplace_back(ArgumentType::kInteger, full_name);

    if (!CheckOnAvailability(arguments_.back())) {
        throw std::runtime_error("There is a collision between two arguments.\n"
                                 "Use only unique short and full names.\n");
    }

    index_by_full_name_[full_name] = arguments_.size() - 1;

    return arguments_.back();
}

int32_t ArgumentParser::ArgParser::GetIntValue(const std::string& full_name, size_t index) {    
    const Argument& arg = arguments_[GetIndex(full_name)];

    return arg.GetIntValue(index);
}

ArgumentParser::Argument& ArgumentParser::ArgParser::AddFlag(char short_name, const std::string& full_name) {
    arguments_.emplace_back(ArgumentType::kFlag, short_name, full_name);

    if (!CheckOnAvailability(arguments_.back())) {
        throw std::runtime_error("There is a collision between two arguments.\n"
                                 "Use only unique short and full names.\n");
    }

    index_by_short_name_[short_name] = arguments_.size() - 1;
    index_by_full_name_[full_name] = arguments_.size() - 1;
    arguments_.back().Default(false);

    return arguments_.back();
}

ArgumentParser::Argument& ArgumentParser::ArgParser::AddFlag(const std::string& full_name) {
    arguments_.emplace_back(ArgumentType::kFlag, full_name);

    if (!CheckOnAvailability(arguments_.back())) {
        throw std::runtime_error("There is a collision between two arguments.\n"
                                 "Use only unique short and full names.\n");
    }

    index_by_full_name_[full_name] = arguments_.size() - 1;
    arguments_.back().Default(false);

    return arguments_.back();
}

bool ArgumentParser::ArgParser::GetFlag(const std::string& full_name) {
    const Argument& arg = arguments_[GetIndex(full_name)];

    return arg.GetFlag();
}

size_t ArgumentParser::ArgParser::GetIndex(char short_name) {
    if (index_by_short_name_.find(short_name) == index_by_short_name_.end()) {
        throw std::runtime_error("No such argument as " + std::string(1, short_name));
    }

    return index_by_short_name_[short_name];
}

size_t ArgumentParser::ArgParser::GetIndex(const std::string& full_name) {
    if (index_by_full_name_.find(full_name) == index_by_full_name_.end()) {
        throw std::runtime_error("No such argument as " + full_name);
    }
    
    return index_by_full_name_[full_name];
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

void ArgumentParser::ArgParser::UpdateStorages() const {
    for (const auto& arg: arguments_) {
        arg.UpdateStorage();
    }
}
