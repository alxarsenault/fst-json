#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <cstring>
#include <experimental/string_view>

#include <fst/ascii.h>
#include <fst/print.h>
#include <fst/file_buffer.h>

#include "fst/json/internal/node_manager.h"
#include "fst/json/internal/file_dimension.h"
#include "fst/json/node_view.h"

namespace fst {
namespace json {
    class document {
    public:
        inline document() { _values.resize(1); }

        inline document(fst::buffer_view<char> file_buffer, std::size_t reserved = 0)
            : _file_buffer(file_buffer)
        {
            if (reserved == 0) {
                internal::file_dimension fs = internal::get_file_dimension_from_size(_file_buffer.size());
                std::size_t reserve_size = get_reserve_size_from_dimension(fs);
                _values.reserve(reserve_size);
                _values.emplace_back(std::experimental::string_view());
                _node_manager.reserve(reserve_size);

                parse();
                return;
            }

            _values.reserve(reserved);
            _values.emplace_back(std::experimental::string_view());
            _node_manager.reserve(reserved);
            parse();
        }

        inline node_view add_node(std::string&& name, std::string&& value, type type)
        {
            _allocated_data.emplace_back(std::move(name));
            _values.emplace_back(std::experimental::string_view(_allocated_data.back()));
            std::size_t name_index = _values.size() - 1;

            _allocated_data.emplace_back(std::move(value));
            _values.emplace_back(std::experimental::string_view(_allocated_data.back()));
            std::size_t value_index = _values.size() - 1;

            _node_manager.emplace_back(internal::node(name_index, value_index, type));

            std::size_t index = _node_manager.size() - 1;
            return node_view(index, _node_manager, _values, _allocated_data);
        }

        inline node_view add_object_node(std::string&& name)
        {
            _allocated_data.emplace_back(std::move(name));
            _values.emplace_back(std::experimental::string_view(_allocated_data.back()));
            std::size_t name_index = _values.size() - 1;

            _node_manager.emplace_back(internal::node(name_index, 0, type::object));

            std::size_t index = _node_manager.size() - 1;
            return node_view(index, _node_manager, _values, _allocated_data);
        }

        inline node_view add_array_node(std::string&& name)
        {
            _allocated_data.emplace_back(std::move(name));
            _values.emplace_back(std::experimental::string_view(_allocated_data.back()));
            std::size_t name_index = _values.size() - 1;

            _node_manager.emplace_back(internal::node(name_index, 0, type::array));

            std::size_t index = _node_manager.size() - 1;
            return node_view(index, _node_manager, _values, _allocated_data);
        }

        inline node_view add_string_node(std::string&& name, std::string&& value)
        {
            _allocated_data.emplace_back(std::move(name));
            _values.emplace_back(std::experimental::string_view(_allocated_data.back()));
            std::size_t name_index = _values.size() - 1;

            _allocated_data.emplace_back(std::move(value));
            _values.emplace_back(std::experimental::string_view(_allocated_data.back()));
            std::size_t value_index = _values.size() - 1;

            _node_manager.emplace_back(internal::node(name_index, value_index, type::string));

            std::size_t index = _node_manager.size() - 1;
            return node_view(index, _node_manager, _values, _allocated_data);
        }

        inline node_view add_number_node(std::string&& name, double value)
        {
            _allocated_data.emplace_back(std::move(name));
            _values.emplace_back(std::experimental::string_view(_allocated_data.back()));
            std::size_t name_index = _values.size() - 1;

            _allocated_data.emplace_back(std::to_string(value));
            _values.emplace_back(std::experimental::string_view(_allocated_data.back()));
            std::size_t value_index = _values.size() - 1;

            _node_manager.emplace_back(internal::node(name_index, value_index, type::number));

            std::size_t index = _node_manager.size() - 1;
            return node_view(index, _node_manager, _values, _allocated_data);
        }

        inline node_view operator[](const std::string& name)
        {
            return node_view(0, _node_manager, _values, _allocated_data)[name];
        }

        inline node_view root() { return node_view(0, _node_manager, _values, _allocated_data); }

        void parse()
        {
            fst::buffer_view<char> data = _file_buffer;
            parse_data pdata = { data.data(), 0, 0, looking_for_id_begin, {} };
            pdata.stack.reserve(10);

            for (std::size_t i = 0; i < data.size(); i++) {
                switch (pdata.current_state) {
                case looking_for_id_begin: {

                    /// @todo Is this skip white space and endline ???
                    go_to_next_char(data, i, [](char c) {
                        return c == '"' || c == '-' || fst::ascii::is_digit(c) || c == '{' || c == 't'
                            || c == 'f' || c == 'n' || c == '[';
                    });

                    find_id_begin(pdata, i, data[i]);
                } break;

                case looking_for_id_end: {
                    go_to_next_char(data, i, [](char c) { return c == '"'; });
                    _values.emplace_back(
                        std::experimental::string_view(pdata.raw_data + pdata.begin, i - pdata.begin));
                    pdata.current_state = looking_for_obj_type;
                    add_node(pdata, internal::node(_values.size() - 1, 0, type::error));
                } break;

                case looking_for_obj_type: {
                    go_to_next_char(data, i, [](char c) {
                        return c == '{' || c == '"' || c == '[' || c == '-' || fst::ascii::is_digit(c)
                            || c == 't' || c == 'f' || c == 'n';
                    });
                    find_obj_type(pdata, i, data[i]);
                } break;

                case looking_for_string_value_end: {
                    go_to_next_char(data, i, [](char c) { return c == '"'; });
                    _values.emplace_back(
                        std::experimental::string_view(pdata.raw_data + pdata.begin, i - pdata.begin));
                    get_current_node(pdata).value = _values.size() - 1;
                    pdata.current_state = looking_for_next_object;
                } break;

                case looking_for_next_object: {
                    go_to_next_char(
                        data, i, [](char c) { return c == ',' || c == '}' || c == '{' || c == ']'; });
                    find_next_object(pdata, i, data[i]);
                } break;

                case looking_for_number_value_end: {
                    go_to_next_char(data, i, [](char c) { return !fst::ascii::is_digit(c) && c != '.'; });
                    /// @todo Are these the only possible char after a number and would it be faster?
                    /// c == ' ' || c == ']' || c == '}' || c == ','
                    _values.emplace_back(
                        std::experimental::string_view(pdata.raw_data + pdata.begin, i - pdata.begin));
                    get_current_node(pdata).value = _values.size() - 1;
                    pdata.current_state = looking_for_next_object;
                    find_next_object(pdata, i, data[i]);
                } break;

                case looking_for_null_value_end:
                case looking_for_bool_value_end: {
                    go_to_next_char(data, i, [](char c) { return !fst::ascii::is_letter(c); });

                    _values.emplace_back(
                        std::experimental::string_view(pdata.raw_data + pdata.begin, i - pdata.begin));
                    get_current_node(pdata).value = _values.size() - 1;
                    pdata.current_state = looking_for_next_object;
                    find_next_object(pdata, i, data[i]);
                } break;
                }
            }
        }

        std::string to_string() const
        {
            if (_node_manager.empty()) {
                return "{}";
            }

            std::string data;
            node_to_string(_node_manager[0], data, 0, true);
            return data;
        }

        void node_to_string(internal::node_const_ref n, std::string& data, int level, bool is_last) const
        {
            switch (n.type) {
            case type::object: {
                if (_values[n.id].size()) {
                    data.push_back('"');
                    data += _values[n.id].to_string();
                    data += "\":";
                }

                data.push_back('{');

                std::size_t last_element_index = n.children.size() - 1;
                for (std::size_t i = 0; i < n.children.size(); i++) {
                    node_to_string(_node_manager[n.children[i]], data, level + 1, i == last_element_index);
                }

                data.push_back('}');

                if (!is_last) {
                    data += ",";
                }
            } break;

            case type::array: {
                if (_values[n.id].size()) {
                    data.push_back('"');
                    data += _values[n.id].to_string();
                    data += "\":";
                }

                data.push_back('[');

                std::size_t last_element_index = n.children.size() - 1;
                for (std::size_t i = 0; i < n.children.size(); i++) {
                    node_to_string(_node_manager[n.children[i]], data, level + 1, i == last_element_index);
                }

                data.push_back(']');

                if (!is_last) {
                    data += ",";
                }
            } break;

            case type::string: {
                if (_values[n.id].empty()) {
                    return;
                }

                data.push_back('"');
                data += _values[n.id].to_string();
                data += "\":\"";
                data += _values[n.value].to_string();
                data.push_back('"');

                if (!is_last) {
                    data += ",";
                }

            } break;

            case type::number: {
                if (!_values[n.id].empty()) {
                    data.push_back('"');
                    data += _values[n.id].to_string();
                    data += "\":";
                }

                data += _values[n.value].to_string();

                if (!is_last) {
                    data += ",";
                }
            } break;

            case type::boolean: {
                if (!_values[n.id].empty()) {
                    data.push_back('"');
                    data += _values[n.id].to_string();
                    data += "\":";
                }

                data += _values[n.value].to_string();

                if (!is_last) {
                    data += ",";
                }
            } break;

            case type::null: {
                if (!_values[n.id].empty()) {
                    data.push_back('"');
                    data += _values[n.id].to_string();
                    data += "\":";
                }

                data += _values[n.value].to_string();

                if (!is_last) {
                    data += ",";
                }
            } break;

            case type::error: {
                /// @todo .... ????
            }
            }
        }

        void print() { print_node(_node_manager[0], 0); }

        void print_node(internal::node_ref n, int level)
        {
            if (n.type == type::object) {
                std::cout << std::string(level * 4, ' ') << _values[n.id] << std::endl;
                for (auto& k : n.children) {
                    print_node(_node_manager[k], level + 1);
                }
            } else if (n.type == type::array) {
                std::cout << std::string(level * 4, ' ') << _values[n.id] << std::endl;
                for (auto& k : n.children) {
                    print_node(_node_manager[k], level + 1);
                }
            } else if (n.type == type::string) {
                std::cout << std::string(level * 4, ' ') << _values[n.id] << " -> " << _values[n.value]
                          << std::endl;
            } else if (n.type == type::number) {
                std::cout << std::string(level * 4, ' ') << _values[n.id] << " -> " << _values[n.value]
                          << std::endl;
            }
        }

    private:
        fst::buffer_view<char> _file_buffer;
        std::vector<std::experimental::string_view> _values;
        internal::node_manager _node_manager;
        std::list<std::string> _allocated_data;

        enum state {
            looking_for_id_begin,
            looking_for_id_end,
            looking_for_obj_type,
            looking_for_string_value_end,
            looking_for_next_object,
            looking_for_number_value_end,
            looking_for_bool_value_end,
            looking_for_null_value_end
        };

        struct parse_data {
            const char* raw_data;
            std::size_t begin;
            std::size_t level;
            state current_state;
            std::vector<std::size_t> stack;
        };

        template <typename Op>
        inline static void go_to_next_char(fst::buffer_view<char>& data, std::size_t& i, Op op)
        {
            for (; !op(data[i]) && i < data.size(); i++) {
            }
        }

        inline void add_node(const parse_data& pdata, internal::node&& n)
        {
            if (pdata.stack.empty()) {
                _node_manager.emplace_back(std::move(n));
                return;
            }

            _node_manager.emplace_back(std::move(n));
            _node_manager[pdata.stack.back()].children.push_back(_node_manager.size() - 1);
        }

        inline internal::node_ref get_current_node(const parse_data& pdata)
        {
            if (pdata.stack.empty()) {
                return _node_manager[_node_manager.size() - 1];
            }

            return _node_manager[_node_manager[pdata.stack.back()].children.back()];
        }

        inline void find_id_begin(parse_data& pdata, std::size_t index, char c)
        {
            switch (c) {
            case '"': {
                pdata.begin = index + 1;
                pdata.current_state = looking_for_id_end;
                return;
            }

            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                pdata.begin = index;
                pdata.current_state = looking_for_number_value_end;
                add_node(pdata, internal::node(0, 0, type::number));
                return;
            }

            case '{': {
                pdata.begin = index;
                pdata.current_state = looking_for_id_begin;
                add_node(pdata, internal::node(0, 0, type::object));
                pdata.stack.push_back(_node_manager.size() - 1);
                return;
            }

            case 't':
            case 'f': {
                pdata.begin = index;
                pdata.current_state = looking_for_bool_value_end;
                add_node(pdata, internal::node(0, 0, type::boolean));
                return;
            }

            case 'n': {
                pdata.begin = index;
                pdata.current_state = looking_for_null_value_end;
                add_node(pdata, internal::node(0, 0, type::null));
                return;
            }

            //// --------- ????????????????????????
            case '[': {
                pdata.current_state = looking_for_id_begin;
                add_node(pdata, internal::node(0, 0, type::array, 50));
                pdata.stack.push_back(_node_manager.size() - 1);
                return;
            }
            }
        }

        inline void find_obj_type(parse_data& pdata, std::size_t index, char c)
        {
            switch (c) {
            case '{': {
                internal::node_ref n = get_current_node(pdata);
                n.type = type::object;
                pdata.stack.push_back(n.index);
                pdata.current_state = looking_for_id_begin;
                return;
            }

            case '"': {
                get_current_node(pdata).type = type::string;
                pdata.begin = index + 1;
                pdata.current_state = looking_for_string_value_end;
                return;
            }

            case '[': {
                internal::node_ref n = get_current_node(pdata);
                n.type = type::array;
                pdata.stack.push_back(n.index);
                pdata.current_state = looking_for_id_begin;
                return;
            }

            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                get_current_node(pdata).type = type::number;
                pdata.begin = index;
                pdata.current_state = looking_for_number_value_end;
                return;
            }

            case 't':
            case 'f': {
                get_current_node(pdata).type = type::boolean;
                pdata.begin = index;
                pdata.current_state = looking_for_bool_value_end;
                return;
            }

            case 'n': {
                get_current_node(pdata).type = type::null;
                pdata.begin = index;
                pdata.current_state = looking_for_null_value_end;
                return;
            }
            }
        }

        inline void find_next_object(parse_data& pdata, std::size_t /*index*/, char c)
        {
            switch (c) {
            case ',': {
                pdata.current_state = looking_for_id_begin;
            }
                return;

            case '}': {
                pdata.current_state = looking_for_next_object;
                // Calling pop_back on an empty container is undefined.
                if (!pdata.stack.empty()) {
                    pdata.stack.pop_back();
                }
            }
                return;

            case '{': {
                pdata.current_state = looking_for_id_begin;
            }
                return;

            case ']': {
                pdata.current_state = looking_for_next_object;
                // Calling pop_back on an empty container is undefined.
                if (!pdata.stack.empty()) {
                    pdata.stack.pop_back();
                }
            }
                return;
            }
        }
    };
} // json.
} // fst
