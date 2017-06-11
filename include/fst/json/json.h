#pragma once

#include <iostream>
#include <vector>
#include <fst/ascii.h>
#include "fst/file_buffer.h"
#include <boost/utility/string_view.hpp>
#include <list>
#include <cstring>

template <class T> class ballocator {
public:
    static constexpr std::size_t BlockSize = 5000000;
    typedef T value_type;

    inline ballocator(const ballocator& a) {}

    inline ballocator()
    {
        if (_buffer == nullptr) {
            _buffer = std::unique_ptr<T>(new T[BlockSize]);
            _begin = _buffer.get();
            _end = _buffer.get() + BlockSize;
            _ptr = _begin;
        }
    }

    inline T* allocate(std::size_t n)
    {
        T* ptr = _ptr;
        if (_ptr + n >= _end) {
            throw std::bad_alloc();
        }
        _ptr += n;

        return ptr;
    }

    void deallocate(T* p, std::size_t n) {}

private:
    static T* _begin;
    static T* _end;
    static T* _ptr;
    static std::unique_ptr<T> _buffer;
};

template <typename T> T* ballocator<T>::_begin = nullptr;
template <typename T> T* ballocator<T>::_end = nullptr;
template <typename T> T* ballocator<T>::_ptr = nullptr;
template <typename T> std::unique_ptr<T> ballocator<T>::_buffer = nullptr;

namespace fst {
namespace json {
        typedef std::vector<std::size_t, ballocator<std::size_t>>
        children_vector_type;
    //    typedef std::vector<std::size_t, ballocator<std::size_t>>
    //    node_vector_type;
//    typedef std::vector<std::size_t> children_vector_type;
    typedef std::vector<std::size_t> node_vector_type;

    enum class type : std::uint8_t {
        null,
        object,
        array,
        string,
        boolean,
        number,
        error
    };

    enum class file_dimension : std::uint8_t { tiny, small, medium, big, huge };

    enum file_dimension_size {
        fds_tiny = 100,
        fds_small = 500,
        fds_medium = 1000,
        fds_big = 10000
    };

    enum file_dimension_reserve {
        fdr_tiny = 20,
        fdr_small = 100,
        fdr_medium = 500,
        fdr_big = 1000,
        fdr_huge = 500000
    };

    enum file_dimension_top_level_reserve {
        fdtlr_tiny = 3,
        fdtlr_small = 10,
        fdtlr_medium = 20,
        fdtlr_big = 50,
        fdtlr_huge = 10000
    };

    namespace adaptor {
        template <typename K, typename T> T& get(K& st, const std::string& name)
        {
        }
    }

    struct node {
        node(std::size_t id, std::size_t value, type type,
            std::size_t reserve_size = 0)
            : id(id)
            , value(value)
            , type(type)
        {
            if (reserve_size) {
                children.reserve(reserve_size);
            }
        }

        node(node&& n)
            : children(std::move(n.children))
            , id(n.id)
            , value(n.value)
            , type(n.type)
        {
        }

        node(const node&) = delete;
        node& operator=(const node&) = delete;

        children_vector_type children;
        std::size_t id;
        std::size_t value;
        type type;
    };

    struct node_ref {
        node_ref(std::size_t index, std::size_t& id, std::size_t& value,
            type& type, children_vector_type& children)
            : index(index)
            , children(children)
            , id(id)
            , value(value)
            , type(type)
        {
        }

        std::size_t index;
        children_vector_type& children;
        std::size_t& id;
        std::size_t& value;
        type& type;
    };

    struct node_const_ref {
        node_const_ref(std::size_t index, const std::size_t id,
            const std::size_t value, const type type,
            const children_vector_type& children)
            : index(index)
            , children(children)
            , id(id)
            , value(value)
            , type(type)
        {
        }

        std::size_t index;
        const children_vector_type& children;
        const std::size_t id;
        const std::size_t value;
        const type type;
    };

    class node_manager {
    public:
        inline void reserve(std::size_t size)
        {
                        id.reserve(size);
                        value.reserve(size);
                        type.reserve(size);
                        children.reserve(size);
        }

        inline void emplace_back(node&& n)
        {
                        id.push_back(n.id);
                        value.push_back(n.value);
                        type.push_back(n.type);
                        children.emplace_back(std::move(n.children));
        }

        inline  node_ref get(std::size_t index)
        {
            return node_ref(
                index, id[index], value[index], type[index], children[index]);
        }

        inline  node_const_ref get(
            std::size_t index) const
        {
            return node_const_ref(
                index, id[index], value[index], type[index], children[index]);
        }

        inline  node_ref operator[](std::size_t index)
        {
            return get(index);
        }

        inline  node_const_ref operator[](
            std::size_t index) const
        {
            return get(index);
        }

        inline  bool empty() const
        {
            return id.size() == 0;
        }

        inline  std::size_t size() const
        {
            return id.size();
        }

    private:
        node_vector_type id;
        node_vector_type value;
        std::vector<type> type;
        std::vector<children_vector_type> children;
    };

    /**
     * node_view.
     */
    class node_view {
    public:
        inline   node_view(
            std::size_t index, node_manager& manager,
            std::vector<boost::string_view>& values,
            std::list<std::string>& allocated_data, bool is_valid = true)
            : _index(index)
            , manager(manager)
            , values(values)
            , allocated_data(allocated_data)
            , _is_valid(manager[index].type == type::error ? false : is_valid)
        {
        }

        inline   node_view
        operator[](const std::string& name)
        {
            for (auto k : manager[_index].children) {
                if (values[manager[k].id] == name) {
                    return node_view(k, manager, values, allocated_data);
                }
            }

            /// @todo special case.
            return node_view(0, manager, values, allocated_data);
        }

        inline  node_view operator[](
            std::size_t child_index)
        {
            node_ref nr = manager[manager[_index].children[child_index]];
            return node_view(nr.index, manager, values, allocated_data);
        }

        inline node_view add_object_node() { return add_node(); }

        inline node_view add_object_node(std::string&& name)
        {
            return add_node(std::move(name), type::object);
        }

        node_view add_array_node(std::string&& name)
        {
            return add_node(std::move(name), type::array);
        }

        node_view add_number_node(std::string&& name, int value)
        {
            return add_node(
                std::move(name), std::to_string(value), type::number);
        }

        node_view add_number_node(std::string&& name, double value)
        {
            return add_node(
                std::move(name), std::to_string(value), type::number);
        }

        node_view add_number_node(double value)
        {
            return add_node(std::to_string(value));
        }

        node_view add_boolean_node(std::string&& name, bool value)
        {
            return add_node(
                std::move(name), value ? "true" : "false", type::boolean);
        }

        node_view add_string_node(std::string&& name, std::string&& value)
        {
            return add_node(std::move(name), std::move(value), type::string);
        }

        boost::string_view name() const { return values[manager[_index].id]; }
        boost::string_view value() const
        {
            return values[manager[_index].value];
        }
        type type() { return manager[_index].type; }

        bool valid() const { return _is_valid; }

        template <typename T> T convert() {}

        class iterator {
        public:
            inline iterator(node_manager& manager,
                std::vector<boost::string_view>& values,
                std::list<std::string>& allocated_data, std::size_t node_index,
                std::size_t child_index)
                : manager(manager)
                , values(values)
                , allocated_data(allocated_data)
                , _node_index(node_index)
                , _child_index(child_index)
            {
            }

            inline iterator operator++()
            {
                return iterator(manager, values, allocated_data, _node_index,
                    _child_index++);
            }

            inline iterator operator++(int)
            {
                return iterator(manager, values, allocated_data, _node_index,
                    _child_index++);
            }

            inline bool operator==(const iterator& rhs)
            {
                return (_node_index == rhs._node_index)
                    && (_child_index == rhs._child_index);
            }

            inline bool operator!=(const iterator& rhs)
            {
                return !(*this == rhs);
            }

            inline node_view operator*()
            {
                node_ref nr
                    = manager[manager[_node_index].children[_child_index]];
                return node_view(nr.index, manager, values, allocated_data);
            }

        private:
            node_manager& manager;
            std::vector<boost::string_view>& values;
            std::list<std::string>& allocated_data;
            std::size_t _node_index;
            std::size_t _child_index;
        };

        iterator begin()
        {
            return iterator(manager, values, allocated_data, _index, 0);
        }

        iterator end()
        {
            return iterator(manager, values, allocated_data, _index,
                manager[_index].children.size());
        }

        std::size_t size() const { return manager[_index].children.size(); }

    private:
        std::size_t _index;
        node_manager& manager;
        std::vector<boost::string_view>& values;
        std::list<std::string>& allocated_data;
        bool _is_valid;

        node_view add_node()
        {
            manager.emplace_back(node(0, 0, type::object));
            std::size_t index = manager.size() - 1;
            manager[_index].children.push_back(index);
            return node_view(index, manager, values, allocated_data);
        }

        node_view add_node(
            std::string&& name, std::string&& value, enum type type)
        {
            allocated_data.emplace_back(std::move(name));
            values.emplace_back(boost::string_view(allocated_data.back()));
            std::size_t name_index = values.size() - 1;

            allocated_data.emplace_back(std::move(value));
            values.emplace_back(boost::string_view(allocated_data.back()));
            std::size_t value_index = values.size() - 1;

            manager.emplace_back(node(name_index, value_index, type));
            std::size_t index = manager.size() - 1;
            manager[_index].children.push_back(index);
            return node_view(index, manager, values, allocated_data);
        }

        node_view add_node(std::string&& name, enum type type)
        {
            allocated_data.emplace_back(std::move(name));
            values.emplace_back(boost::string_view(allocated_data.back()));
            std::size_t name_index = values.size() - 1;

            manager.emplace_back(node(name_index, 0, type));
            std::size_t index = manager.size() - 1;
            manager[_index].children.push_back(index);
            return node_view(index, manager, values, allocated_data);
        }

        node_view add_node(std::string&& value)
        {
            allocated_data.emplace_back(std::move(value));
            values.emplace_back(boost::string_view(allocated_data.back()));
            std::size_t value_index = values.size() - 1;

            manager.emplace_back(node(0, value_index, type::number));
            std::size_t index = manager.size() - 1;
            manager[_index].children.push_back(index);
            return node_view(index, manager, values, allocated_data);
        }
    };

    template <> double node_view::convert()
    {
        return std::stod(value().to_string());
    }

    template <> int node_view::convert()
    {
        return std::stoi(value().to_string());
    }

    template <> float node_view::convert()
    {
        return std::stof(value().to_string());
    }

    template <> bool node_view::convert() { return value()[0] == 't'; }

    template <> std::string node_view::convert() { return value().to_string(); }

    file_dimension get_file_dimension_from_size(std::size_t size)
    {
        if (size < fds_tiny) {
            return file_dimension::tiny;
        } else if (size < fds_small) {
            return file_dimension::small;
        } else if (size < fds_medium) {
            return file_dimension::medium;
        } else if (size < fds_big) {
            return file_dimension::big;
        }

        return file_dimension::huge;
    }

    std::size_t get_reserve_size_from_dimension(file_dimension fs)
    {
        switch (fs) {
        case file_dimension::tiny:
            return fdr_tiny;
        case file_dimension::small:
            return fdr_small;
        case file_dimension::medium:
            return fdr_medium;
        case file_dimension::big:
            return fdr_big;
        case file_dimension::huge:
            return fdr_huge;
        }
    }

    std::size_t get_top_level_reserve_size_from_dimension(file_dimension fs)
    {
        switch (fs) {
        case file_dimension::tiny:
            return fdtlr_tiny;
        case file_dimension::small:
            return fdtlr_small;
        case file_dimension::medium:
            return fdtlr_medium;
        case file_dimension::big:
            return fdtlr_big;
        case file_dimension::huge:
            return fdtlr_huge;
        }
    }

    class document {
    public:
        inline document()
        {
            _values.resize(2);
            _value_size = 1;
            //            _allocated_data.reserve(100);
        }

        inline document(
            fst::buffer_view<char> file_buffer, std::size_t reserved = 0)
            : _file_buffer(file_buffer)
        {
            if (reserved == 0) {
                file_dimension fs
                    = get_file_dimension_from_size(_file_buffer.size());
                std::size_t reserve_size = get_reserve_size_from_dimension(fs);
                std::size_t tl_reserve_size
                    = get_top_level_reserve_size_from_dimension(fs);
                _values.resize(reserve_size);
                _value_size = 1;
                _node_manager.reserve(reserved);

                parse();
                return;
            }

            _values.resize(reserved);
            _value_size = 1;
            _node_manager.reserve(reserved);
            parse();
        }

        inline document(fst::buffer_view<char> file_buffer, file_dimension fs)
            : _file_buffer(file_buffer)
        {
            std::size_t reserve_size = get_reserve_size_from_dimension(fs);
            std::size_t tl_reserve_size
                = get_top_level_reserve_size_from_dimension(fs);
            _values.resize(reserve_size);
            _value_size = 1;
            _node_manager.reserve(reserve_size);
            parse();
        }

        node_view add_node(std::string&& name, std::string&& value, type type)
        {
            _allocated_data.emplace_back(std::move(name));
            _values.emplace_back(boost::string_view(_allocated_data.back()));
            std::size_t name_index = _values.size() - 1;

            _allocated_data.emplace_back(std::move(value));
            _values.emplace_back(boost::string_view(_allocated_data.back()));
            std::size_t value_index = _values.size() - 1;

            _node_manager.emplace_back(node(name_index, value_index, type));
            //            _top_level.push_back(_node_manager.size() - 1);

            std::size_t index = _node_manager.size() - 1;
            return node_view(index, _node_manager, _values, _allocated_data);
        }

        node_view add_object_node(std::string&& name)
        {
            _allocated_data.emplace_back(std::move(name));
            _values.emplace_back(boost::string_view(_allocated_data.back()));
            std::size_t name_index = _values.size() - 1;

            _node_manager.emplace_back(node(name_index, 0, type::object));
            //            _top_level.push_back(_node_manager.size() - 1);

            std::size_t index = _node_manager.size() - 1;
            return node_view(index, _node_manager, _values, _allocated_data);
        }

        node_view add_array_node(std::string&& name)
        {
            _allocated_data.emplace_back(std::move(name));
            _values.emplace_back(boost::string_view(_allocated_data.back()));
            std::size_t name_index = _values.size() - 1;

            _node_manager.emplace_back(node(name_index, 0, type::array));
            //            _top_level.push_back(_node_manager.size() - 1);

            std::size_t index = _node_manager.size() - 1;
            return node_view(index, _node_manager, _values, _allocated_data);
        }

        node_view add_string_node(std::string&& name, std::string&& value)
        {
            _allocated_data.emplace_back(std::move(name));
            _values.emplace_back(boost::string_view(_allocated_data.back()));
            std::size_t name_index = _values.size() - 1;

            _allocated_data.emplace_back(std::move(value));
            _values.emplace_back(boost::string_view(_allocated_data.back()));
            std::size_t value_index = _values.size() - 1;

            _node_manager.emplace_back(
                node(name_index, value_index, type::string));
            //            _top_level.push_back(_node_manager.size() - 1);

            std::size_t index = _node_manager.size() - 1;
            return node_view(index, _node_manager, _values, _allocated_data);
        }

        node_view add_number_node(std::string&& name, double value)
        {
            _allocated_data.emplace_back(std::move(name));
            _values.emplace_back(boost::string_view(_allocated_data.back()));
            std::size_t name_index = _values.size() - 1;

            _allocated_data.emplace_back(std::to_string(value));
            _values.emplace_back(boost::string_view(_allocated_data.back()));
            std::size_t value_index = _values.size() - 1;

            _node_manager.emplace_back(
                node(name_index, value_index, type::number));
            //            _top_level.push_back(_node_manager.size() - 1);

            //            return _node_manager[_node_manager.size() - 1];
            std::size_t index = _node_manager.size() - 1;
            return node_view(index, _node_manager, _values, _allocated_data);
        }

        node_view operator[](const std::string& name)
        {
            // for (auto& k : _top_level) {
            //                if (_values[_node_manager[k].id] == name) {
            //                    return node_view(k, _node_manager, _values,
            //                    _allocated_data);
            //                }
            //            }

            /// @todo special case.
            return node_view(0, _node_manager, _values, _allocated_data);
        }

        void parse()
        {
            fst::buffer_view<char> data = _file_buffer;
            parse_data pdata = { data.data(), 0, 0, looking_for_id_begin };
            pdata.stack.reserve(10);

            for (std::size_t i = 0; i < data.size(); i++) {
                switch (pdata.current_state) {
                case looking_for_id_begin: {
                    go_to_next_char(data, i, [](char c) {
                        return c == '"' || c == '-' || fst::ascii::is_digit(c)
                            || c == '{' || c == 't' || c == 'f' || c == 'n'
                            || c == '[';
                    });
                    find_id_begin(pdata, i, data[i]);
                } break;

                case looking_for_id_end: {
                    go_to_next_char(data, i, [](char c) { return c == '"'; });
                    add_value(boost::string_view(
                        pdata.raw_data + pdata.begin, i - pdata.begin));
                    pdata.current_state = looking_for_obj_type;
                    add_node(pdata, node(_value_size - 1, 0, type::error));
                } break;

                case looking_for_obj_type: {
                    go_to_next_char(data, i, [](char c) {
                        return c == '{' || c == '"' || c == '[' || c == '-'
                            || fst::ascii::is_digit(c) || c == 't' || c == 'f'
                            || c == 'n';
                    });
                    find_obj_type(pdata, i, data[i]);
                } break;

                case looking_for_string_value_end: {
                    go_to_next_char(data, i, [](char c) { return c == '"'; });
                    add_value(boost::string_view(
                        pdata.raw_data + pdata.begin, i - pdata.begin));
                    get_current_node(pdata).value = _value_size - 1;
                    pdata.current_state = looking_for_next_object;
                } break;

                case looking_for_next_object: {
                    go_to_next_char(data, i, [](char c) {
                        return c == ',' || c == '}' || c == '{' || c == ']';
                    });
                    find_next_object(pdata, i, data[i]);
                } break;

                case looking_for_number_value_end: {
                    go_to_next_char(data, i, [](char c) {
                        return !fst::ascii::is_digit(c) && c != '.';
                    });
                    add_value(boost::string_view(
                        pdata.raw_data + pdata.begin, i - pdata.begin));
                    get_current_node(pdata).value = _value_size - 1;
                    pdata.current_state = looking_for_next_object;
                    find_next_object(pdata, i, data[i]);
                } break;

                case looking_for_null_value_end:
                case looking_for_bool_value_end: {
                    go_to_next_char(data, i,
                        [](char c) { return !fst::ascii::is_letter(c); });

                    add_value(boost::string_view(
                        pdata.raw_data + pdata.begin, i - pdata.begin));
                    get_current_node(pdata).value = _value_size - 1;
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

        void node_to_string(
            node_const_ref n, std::string& data, int level, bool is_last) const
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
                for (int i = 0; i < n.children.size(); i++) {
                    node_to_string(_node_manager[n.children[i]], data,
                        level + 1, i == last_element_index);
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
                for (int i = 0; i < n.children.size(); i++) {
                    node_to_string(_node_manager[n.children[i]], data,
                        level + 1, i == last_element_index);
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
            }
        }

        void print()
        {
            //            for (auto& t : _top_level) {
            print_node(_node_manager[0], 0);
            //            }
        }

        void print_node(node_ref n, int level)
        {
            if (n.type == type::object) {
                std::cout << std::string(level * 4, ' ') << _values[n.id]
                          << std::endl;
                for (auto& k : n.children) {
                    print_node(_node_manager[k], level + 1);
                }
            } else if (n.type == type::array) {
                std::cout << std::string(level * 4, ' ') << _values[n.id]
                          << std::endl;
                for (auto& k : n.children) {
                    print_node(_node_manager[k], level + 1);
                }
            } else if (n.type == type::string) {
                std::cout << std::string(level * 4, ' ') << _values[n.id]
                          << " -> " << _values[n.value] << std::endl;
            } else if (n.type == type::number) {
                std::cout << std::string(level * 4, ' ') << _values[n.id]
                          << " -> " << _values[n.value] << std::endl;
            }
        }

        template <typename T> void fill(T& st) const
        {
            //            for (auto& t : _top_level) {
            //                auto n = _node_manager[t];
            //
            //                switch (n.type) {
            //                case type::number: {
            //                    adaptor::get<T, double>(st,
            //                    _values[n.id].to_string())
            //                        = std::stod(_values[n.value].to_string());
            //                } break;
            //                case type::string: {
            //                    adaptor::get<T, std::string>(st,
            //                    _values[n.id].to_string())
            //                        = _values[n.value].to_string();
            //                }
            //                }
            //            }
        }

    private:
        fst::buffer_view<char> _file_buffer;
        std::vector<boost::string_view> _values;
        std::size_t _value_size;
        node_manager _node_manager;
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
        inline static void go_to_next_char(
            fst::buffer_view<char>& data, std::size_t& i, Op op)
        {
            for (; !op(data[i]) && i < data.size(); i++) {
            }
        }

        inline static void next_char(
            fst::buffer_view<char>& data, std::size_t& i, const char* check)
        {
            char* r = std::strpbrk(&data[i], check);
            if (r == nullptr) {
                i = data.size();
                return;
            }

            i += r - &data[i];
        }

        inline void add_value(boost::string_view&& s)
        {
            if (_value_size >= _values.capacity()) {
                _values.resize(_value_size * 2);
            }

            _values[_value_size] = std::move(s);
            _value_size++;
        }

        inline void add_node(
            parse_data& pdata, node&& n)
        {
            //            if (_node_manager.empty() || pdata.stack.empty()) {
            //                _node_manager.emplace_back(std::move(n));
            //                _top_level.push_back(_node_manager.size() - 1);
            //                return;
            //            }

            if (pdata.stack.empty()) {
                _node_manager.emplace_back(std::move(n));
                return;
            }

            _node_manager.emplace_back(std::move(n));
            _node_manager[pdata.stack.back()].children.push_back(
                _node_manager.size() - 1);
        }

        inline node_ref get_current_node(
            parse_data& pdata)
        {
            if (pdata.stack.empty()) {
                std::size_t index = _node_manager.size() - 1;
                return _node_manager[index];
            }

            std::size_t index
                = _node_manager[pdata.stack.back()].children.back();
            return _node_manager[index];
        }

        inline void find_id_begin(
            parse_data& pdata, std::size_t index, char c)
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
                add_node(pdata, node(0, 0, type::number));
                return;
            }

            case '{': {
                pdata.begin = index;
                pdata.current_state = looking_for_id_begin;
                add_node(pdata, node(0, 0, type::object));
                pdata.stack.push_back(_node_manager.size() - 1);
                return;
            }

            case 't':
            case 'f': {
                pdata.begin = index;
                pdata.current_state = looking_for_bool_value_end;
                add_node(pdata, node(0, 0, type::boolean));
                return;
            }

            case 'n': {
                pdata.begin = index;
                pdata.current_state = looking_for_null_value_end;
                add_node(pdata, node(0, 0, type::null));
                return;
            }

            //// --------- ????????????????????????
            case '[': {
                pdata.current_state = looking_for_id_begin;
                add_node(pdata, node(0, 0, type::array, 50));
                pdata.stack.push_back(_node_manager.size() - 1);
                return;
            }
            }
        }

        inline void find_obj_type(
            parse_data& pdata, std::size_t index, char c)
        {
            switch (c) {
            case '{': {
                node_ref n = get_current_node(pdata);
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
                node_ref n = get_current_node(pdata);
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

        inline void find_next_object(
            parse_data& pdata, std::size_t index, char c)
        {
            switch (c) {
            case ',': {
                pdata.current_state = looking_for_id_begin;
            }
                return;

            case '}': {
                pdata.current_state = looking_for_next_object;
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
