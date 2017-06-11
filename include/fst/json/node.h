#pragma once

#include "fst/json/types.h"
#include <vector>

namespace fst {
namespace json {
    struct node {
        inline node(std::size_t id, std::size_t value, type type,
            std::size_t reserve_size = 0)
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

        std::vector<std::size_t> children;
        std::size_t id;
        std::size_t value;
        type type;
    };

    struct node_ref {
        inline node_ref(std::size_t index, std::size_t& id, std::size_t& value,
            type& type, std::vector<std::size_t>& children)
            : index(index)
            , children(children)
            , id(id)
            , value(value)
            , type(type)
        {
        }

        std::size_t index;
        std::vector<std::size_t>& children;
        std::size_t& id;
        std::size_t& value;
        type& type;
    };

    struct node_const_ref {
        inline node_const_ref(std::size_t index, const std::size_t id,
            const std::size_t value, const type type,
            const std::vector<std::size_t>& children)
            : index(index)
            , children(children)
            , id(id)
            , value(value)
            , type(type)
        {
        }

        std::size_t index;
        const std::vector<std::size_t>& children;
        const std::size_t id;
        const std::size_t value;
        const type type;
    };
} // json.
} // fst.
