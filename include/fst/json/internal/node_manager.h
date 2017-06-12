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
                _node_id.reserve(size);
                _node_value.reserve(size);
                _node_type.reserve(size);
                _node_children.reserve(size);
                _data_values.reserve(size);

                //                for(auto& n : _node_children) {
                //                    n.reserve(size);
                //                }
            }

            inline void emplace_back(internal::node&& n)
            {
                _node_id.push_back(n.id);
                _node_value.push_back(n.value);
                _node_type.push_back(n.type);
                _node_children.emplace_back(children_container());
            }

            inline internal::node_ref node_at(std::size_t index)
            {
                return internal::node_ref(
                    index, _node_id[index], _node_value[index], _node_type[index], _node_children[index]);
            }

            inline internal::node_const_ref node_at(std::size_t index) const
            {
                return internal::node_const_ref(
                    index, _node_id[index], _node_value[index], _node_type[index], _node_children[index]);
            }

            inline internal::node_ref operator[](std::size_t index) { return node_at(index); }

            inline internal::node_const_ref operator[](std::size_t index) const { return node_at(index); }

            inline bool empty() const { return _node_id.size() == 0; }

            inline std::size_t size() const { return _node_id.size(); }

            inline std::size_t last_node_index() const { return _node_id.size() - 1; }

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
            std::vector<std::size_t> _node_id;
            std::vector<std::size_t> _node_value;
            std::vector<type> _node_type;
            std::vector<children_container> _node_children;
            std::vector<std::experimental::string_view> _data_values;
        };
    } // internal.
} // json.
} // fst.
