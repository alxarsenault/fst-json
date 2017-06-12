#pragma once

#include "fst/json/types.h"
#include <fst/vector.h>
#include <vector>
#include <list>

namespace fst {
namespace json {
    namespace internal {
        typedef fst::vector<std::uint32_t, 4, false, 8> children_container;

        /**
         * node.
         */
        struct node {
            inline node(std::size_t id, std::size_t value, type type)
                : id(id)
                , value(value)
                , type(static_cast<std::size_t>(type))
            {
            }

            inline node(node&& n)
                : id(n.id)
                , value(n.value)
                , type(n.type)
            {
            }

            inline node(const node& n)
                : id(n.id)
                , value(n.value)
                , type(n.type)
            {
            }

            std::size_t id : 29; // 536870912
            std::size_t value : 32; // 4294967296
            std::size_t type : 3; // 8
        };

        /**
         * node_ref.
         */
        struct node_ref {
            inline node_ref(std::size_t index, node& node, children_container& children)
                : index(index)
                , children(children)
                , node(node)
            {
            }

            std::size_t index;
            children_container& children;
            node& node;
        };

        /**
         * node_const_ref.
         */
        struct node_const_ref {
            inline node_const_ref(
                const std::size_t index, const node node, const children_container& children)
                : index(index)
                , children(children)
                , node(node)
            {
            }

            const std::size_t index;
            const children_container& children;
            const node node;
        };
    } // internal.
} // json.
} // fst.
