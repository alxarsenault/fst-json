#pragma once

#include "fst/json/internal/node_manager.h"
#include <experimental/string_view>
#include <vector>
#include <list>
#include <string>
#include <cstring>

namespace fst {
namespace json {
    /**
     * node_view.
     */
    class node_view {
    public:
        inline node_view(std::size_t index, internal::node_manager& manager,
            std::list<std::string>& allocated_data, bool is_valid = true)
            : _index(index)
            , _manager(manager)
            , allocated_data(allocated_data)
            , _is_valid(manager[index].type == type::error ? false : is_valid)
        {
        }

        inline node_view operator[](const std::string& name)
        {
            for (auto k : _manager[_index].children) {
                if (_manager.values()[_manager[k].id] == name) {
                    return node_view(k, _manager, allocated_data);
                }
            }

            /// @todo special case.
            return node_view(0, _manager, allocated_data);
        }

        inline node_view operator[](std::size_t child_index)
        {
            internal::node_ref nr = _manager[_manager[_index].children[child_index]];
            return node_view(nr.index, _manager, allocated_data);
        }

        inline node_view add_object_node() { return add_node(); }

        inline node_view add_object_node(std::string&& name)
        {
            return add_node(std::move(name), type::object);
        }

        inline node_view add_array_node(std::string&& name) { return add_node(std::move(name), type::array); }

        inline node_view add_number_node(std::string&& name, int value)
        {
            return add_node(std::move(name), std::to_string(value), type::number);
        }

        inline node_view add_number_node(std::string&& name, double value)
        {
            return add_node(std::move(name), std::to_string(value), type::number);
        }

        inline node_view add_number_node(double value) { return add_node(std::to_string(value)); }

        inline node_view add_boolean_node(std::string&& name, bool value)
        {
            return add_node(std::move(name), value ? "true" : "false", type::boolean);
        }

        inline node_view add_string_node(std::string&& name, std::string&& value)
        {
            return add_node(std::move(name), std::move(value), type::string);
        }

        inline std::experimental::string_view name() const { return _manager.values()[_manager[_index].id]; }

        inline std::experimental::string_view value() const
        {
            return _manager.values()[_manager[_index].value];
        }
        inline type type() { return _manager[_index].type; }

        inline bool valid() const { return _is_valid; }

        template <typename T> T convert() {}

        class iterator {
        public:
            inline iterator(internal::node_manager& manager, std::list<std::string>& allocated_data,
                std::size_t node_index, std::size_t child_index)
                : _manager(manager)
                , allocated_data(allocated_data)
                , _node_index(node_index)
                , _child_index(child_index)
            {
            }

            inline iterator operator++()
            {
                return iterator(_manager, allocated_data, _node_index, _child_index++);
            }

            inline iterator operator++(int)
            {
                return iterator(_manager, allocated_data, _node_index, _child_index++);
            }

            inline bool operator==(const iterator& rhs)
            {
                return (_node_index == rhs._node_index) && (_child_index == rhs._child_index);
            }

            inline bool operator!=(const iterator& rhs) { return !(*this == rhs); }

            inline node_view operator*()
            {
                internal::node_ref nr = _manager[_manager[_node_index].children[_child_index]];
                return node_view(nr.index, _manager, allocated_data);
            }

        private:
            internal::node_manager& _manager;
            //            std::vector<std::experimental::string_view>& _values;
            std::list<std::string>& allocated_data;
            std::size_t _node_index;
            std::size_t _child_index;
        };

        inline iterator begin() { return iterator(_manager, allocated_data, _index, 0); }

        inline iterator end()
        {
            return iterator(_manager, allocated_data, _index, _manager[_index].children.size());
        }

        inline std::size_t size() const { return _manager[_index].children.size(); }

    private:
        std::size_t _index;
        internal::node_manager& _manager;
        //        std::vector<std::experimental::string_view>& _values;
        std::list<std::string>& allocated_data;
        bool _is_valid;

        inline node_view add_node()
        {
            _manager.emplace_back(internal::node(0, 0, type::object));
            std::size_t index = _manager.size() - 1;
            _manager[_index].children.push_back(index);
            return node_view(index, _manager, allocated_data);
        }

        inline node_view add_node(std::string&& name, std::string&& value, enum type type)
        {
            allocated_data.emplace_back(std::move(name));
            _manager.emplace_value(std::experimental::string_view(allocated_data.back()));
            std::size_t name_index = _manager.last_value_index();

            allocated_data.emplace_back(std::move(value));
            _manager.emplace_value(std::experimental::string_view(allocated_data.back()));
            std::size_t value_index = _manager.last_value_index();

            _manager.emplace_back(internal::node(name_index, value_index, type));
            std::size_t index = _manager.size() - 1;
            _manager[_index].children.push_back(index);
            return node_view(index, _manager, allocated_data);
        }

        inline node_view add_node(std::string&& name, enum type type)
        {
            allocated_data.emplace_back(std::move(name));
            _manager.emplace_value(std::experimental::string_view(allocated_data.back()));
            std::size_t name_index = _manager.last_value_index();

            _manager.emplace_back(internal::node(name_index, 0, type));
            std::size_t index = _manager.size() - 1;
            _manager[_index].children.push_back(index);
            return node_view(index, _manager, allocated_data);
        }

        inline node_view add_node(std::string&& value)
        {
            allocated_data.emplace_back(std::move(value));
            _manager.emplace_value(std::experimental::string_view(allocated_data.back()));
            std::size_t value_index = _manager.last_value_index();

            _manager.emplace_back(internal::node(0, value_index, type::number));
            std::size_t index = _manager.size() - 1;
            _manager[_index].children.push_back(index);
            return node_view(index, _manager, allocated_data);
        }
    };

    template <> double node_view::convert() { return std::stod(value().to_string()); }

    template <> int node_view::convert() { return std::stoi(value().to_string()); }

    template <> float node_view::convert() { return std::stof(value().to_string()); }

    template <> bool node_view::convert() { return value()[0] == 't'; }

    template <> std::string node_view::convert() { return value().to_string(); }
} // json.
} // fst.
