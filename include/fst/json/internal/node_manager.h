#pragma once

#include "fst/json/internal/node.h"

namespace fst {
namespace json {
    namespace internal {
        class node_manager {
        public:
            inline void reserve(std::size_t size)
            {
                id.reserve(size);
                value.reserve(size);
                type.reserve(size);
                children.reserve(size);
            }

            inline void emplace_back(internal::node&& n)
            {
                id.push_back(n.id);
                value.push_back(n.value);
                type.push_back(n.type);
                children.emplace_back(std::move(n.children));
            }

            inline internal::node_ref get(std::size_t index)
            {
                return internal::node_ref(index, id[index], value[index], type[index], children[index]);
            }

            inline internal::node_const_ref get(std::size_t index) const
            {
                return internal::node_const_ref(index, id[index], value[index], type[index], children[index]);
            }

            inline internal::node_ref operator[](std::size_t index) { return get(index); }

            inline internal::node_const_ref operator[](std::size_t index) const { return get(index); }

            inline bool empty() const { return id.size() == 0; }

            inline std::size_t size() const { return id.size(); }

        private:
            std::vector<std::size_t> id;
            std::vector<std::size_t> value;
            std::vector<type> type;
            std::vector<std::vector<std::size_t>> children;
        };
    } // internal.
} // json.
} // fst.
