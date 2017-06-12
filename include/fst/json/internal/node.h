#pragma once

#include "fst/json/types.h"
#include <fst/vector.h>
#include <vector>
#include <list>

namespace fst {
namespace json {
    namespace internal {
        // template <typename T, std::size_t N = 8, bool Destruct = true, std::size_t ResizeMultiplicator = 2>
        typedef fst::vector<std::size_t, 8, false, 4> children_container;
        // typedef std::vector<std::size_t> children_container;

        /**
         * node.
         */
        struct node {
            inline node(std::size_t id, std::size_t value, type type)
                : id(id)
                , value(value)
                , type(type)
            {
                //                if (reserve_size) {
                //                    children.reserve(reserve_size);
                //                }
            }

            inline node(node&& n)
                : id(n.id)
                , value(n.value)
                , type(n.type)
            {
            }

            node(const node&) = delete;
            node& operator=(const node&) = delete;

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
