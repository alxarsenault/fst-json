#pragma once

#include "fst/json/types.h"
#include <vector>

namespace fst {
namespace json {
    namespace internal {
        typedef std::vector<std::size_t> children_container;

        /**
         * node.
         */
        struct node {
            inline node(std::size_t id, std::size_t value, type type, std::size_t reserve_size = 0)
                : id(id)
                , value(value)
                , type(type)
            {
                if (reserve_size) {
                    children.reserve(reserve_size);
                }
            }

            inline node(node&& n)
                : children(std::move(n.children))
                , id(n.id)
                , value(n.value)
                , type(n.type)
            {
            }

            node(const node&) = delete;
            node& operator=(const node&) = delete;

            children_container children;
            std::size_t id;
            std::size_t value;
            type type;
        };

        /**
         * node_ref.
         */
        struct node_ref {
            inline node_ref(std::size_t index, std::size_t& id, std::size_t& value, type& type,
                children_container& children)
                : index(index)
                , children(children)
                , id(id)
                , value(value)
                , type(type)
            {
            }

            std::size_t index;
            children_container& children;
            std::size_t& id;
            std::size_t& value;
            type& type;
        };

        /**
         * node_const_ref.
         */
        struct node_const_ref {
            inline node_const_ref(std::size_t index, const std::size_t id, const std::size_t value,
                const type type, const children_container& children)
                : index(index)
                , children(children)
                , id(id)
                , value(value)
                , type(type)
            {
            }

            std::size_t index;
            const children_container& children;
            const std::size_t id;
            const std::size_t value;
            const type type;
        };
    } // internal.
} // json.
} // fst.
