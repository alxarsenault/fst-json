#pragma once

#include "fst/json/internal/node.h"

namespace fst {
namespace json {
    namespace internal {
        class node_manager {
        public:
            inline node_manager()
            {
                // Index 0 is use as an empty value.
                _data_values.resize(1);
            }

            inline void reserve(std::size_t size)
            {
                _nodes.reserve(size);
                _node_children.reserve(size);
                _data_values.reserve(size);
            }

            inline void emplace_back(internal::node&& n)
            {
                _nodes.emplace_back(std::move(n));
                _node_children.emplace_back(children_container());
            }

            inline internal::node_ref node_at(std::size_t index)
            {
                return internal::node_ref(index, _nodes[index], _node_children[index]);
            }

            inline internal::node_const_ref node_at(std::size_t index) const
            {
                return internal::node_const_ref(index, _nodes[index], _node_children[index]);
            }

            inline children_container& node_children_at(std::size_t index) { return _node_children[index]; }

            inline const children_container& node_children_at(std::size_t index) const
            {
                return _node_children[index];
            }

            inline internal::node_ref operator[](std::size_t index) { return node_at(index); }

            inline internal::node_const_ref operator[](std::size_t index) const { return node_at(index); }

            inline bool empty() const { return _nodes.size() == 0; }

            inline std::size_t size() const { return _nodes.size(); }

            inline std::size_t last_node_index() const { return _nodes.size() - 1; }

            inline void emplace_value(std::experimental::string_view&& value)
            {
                _data_values.emplace_back(std::move(value));
            }

            inline void add_value(const char* data, std::size_t size)
            {
                _data_values.emplace_back(std::experimental::string_view(data, size));
            }

            inline std::size_t value_size() const { return _data_values.size(); }

            inline std::size_t last_value_index() const { return _data_values.size() - 1; }

            inline std::experimental::string_view& value_at(std::size_t index) { return _data_values[index]; }

            inline const std::experimental::string_view& value_at(std::size_t index) const
            {
                return _data_values[index];
            }

            inline std::vector<std::experimental::string_view>& values() { return _data_values; }

            inline const std::vector<std::experimental::string_view>& values() const { return _data_values; }

        private:
            std::vector<node> _nodes;
            std::vector<children_container> _node_children;
            std::vector<std::experimental::string_view> _data_values;
        };
    } // internal.
} // json.
} // fst.
