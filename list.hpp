#ifndef SJTU_LIST_HPP
#define SJTU_LIST_HPP

#include "exceptions.hpp"
#include "algorithm.hpp"

#include <climits>
#include <cstddef>

namespace sjtu {

template<typename T>
class list {
protected:
    class node {
    public:
        T *data;
        node *prev, *next;

        node() : data(nullptr), prev(nullptr), next(nullptr) {}

        explicit node(const T &val) : prev(nullptr), next(nullptr) {
            data = new T(val);
        }

        ~node() {
            delete data;
        }
    };

protected:
    node *_head; // sentinel head (before begin), prev=nullptr
    node *_tail; // sentinel tail (end),         next=nullptr
    size_t _size;

    // Low-level: insert node cur BEFORE pos, return cur
    node *insert(node *pos, node *cur) {
        cur->prev = pos->prev;
        cur->next = pos;
        pos->prev->next = cur;
        pos->prev = cur;
        ++_size;
        return cur;
    }

    // Low-level: unlink node pos (no delete, no size update internally needed)
    // Returns pos
    node *erase(node *pos) {
        pos->prev->next = pos->next;
        pos->next->prev = pos->prev;
        --_size;
        pos->prev = nullptr;
        pos->next = nullptr;
        return pos;
    }

public:
    class const_iterator;

    class iterator {
        friend class list<T>;
        friend class const_iterator;
    public:
        node *ptr;
        list<T> *lst;

        iterator() : ptr(nullptr), lst(nullptr) {}
        iterator(node *p, list<T> *l) : ptr(p), lst(l) {}

        // post-increment
        iterator operator++(int) {
            if (ptr == nullptr || ptr->next == nullptr)
                throw invalid_iterator();
            iterator tmp = *this;
            ptr = ptr->next;
            return tmp;
        }

        // pre-increment
        iterator &operator++() {
            if (ptr == nullptr || ptr->next == nullptr)
                throw invalid_iterator();
            ptr = ptr->next;
            return *this;
        }

        // post-decrement
        iterator operator--(int) {
            if (ptr == nullptr || ptr->prev == nullptr || ptr->prev->prev == nullptr)
                throw invalid_iterator();
            iterator tmp = *this;
            ptr = ptr->prev;
            return tmp;
        }

        // pre-decrement
        iterator &operator--() {
            if (ptr == nullptr || ptr->prev == nullptr || ptr->prev->prev == nullptr)
                throw invalid_iterator();
            ptr = ptr->prev;
            return *this;
        }

        T &operator*() const {
            if (ptr == nullptr || ptr->data == nullptr)
                throw invalid_iterator();
            return *(ptr->data);
        }

        T *operator->() const {
            if (ptr == nullptr || ptr->data == nullptr)
                throw invalid_iterator();
            return ptr->data;
        }

        bool operator==(const iterator &rhs) const { return ptr == rhs.ptr; }
        bool operator==(const const_iterator &rhs) const { return ptr == rhs.ptr; }
        bool operator!=(const iterator &rhs) const { return ptr != rhs.ptr; }
        bool operator!=(const const_iterator &rhs) const { return ptr != rhs.ptr; }
    };

    class const_iterator {
        friend class list<T>;
        friend class iterator;
    public:
        node *ptr;
        const list<T> *lst;

        const_iterator() : ptr(nullptr), lst(nullptr) {}
        const_iterator(node *p, const list<T> *l) : ptr(p), lst(l) {}

        // construct from iterator
        const_iterator(const iterator &it) : ptr(it.ptr), lst(it.lst) {}

        const_iterator operator++(int) {
            if (ptr == nullptr || ptr->next == nullptr)
                throw invalid_iterator();
            const_iterator tmp = *this;
            ptr = ptr->next;
            return tmp;
        }

        const_iterator &operator++() {
            if (ptr == nullptr || ptr->next == nullptr)
                throw invalid_iterator();
            ptr = ptr->next;
            return *this;
        }

        const_iterator operator--(int) {
            if (ptr == nullptr || ptr->prev == nullptr || ptr->prev->prev == nullptr)
                throw invalid_iterator();
            const_iterator tmp = *this;
            ptr = ptr->prev;
            return tmp;
        }

        const_iterator &operator--() {
            if (ptr == nullptr || ptr->prev == nullptr || ptr->prev->prev == nullptr)
                throw invalid_iterator();
            ptr = ptr->prev;
            return *this;
        }

        const T &operator*() const {
            if (ptr == nullptr || ptr->data == nullptr)
                throw invalid_iterator();
            return *(ptr->data);
        }

        const T *operator->() const {
            if (ptr == nullptr || ptr->data == nullptr)
                throw invalid_iterator();
            return ptr->data;
        }

        bool operator==(const iterator &rhs) const { return ptr == rhs.ptr; }
        bool operator==(const const_iterator &rhs) const { return ptr == rhs.ptr; }
        bool operator!=(const iterator &rhs) const { return ptr != rhs.ptr; }
        bool operator!=(const const_iterator &rhs) const { return ptr != rhs.ptr; }
    };

    // Default constructor
    list() : _size(0) {
        _head = new node();
        _tail = new node();
        _head->next = _tail;
        _tail->prev = _head;
    }

    // Copy constructor
    list(const list &other) : _size(0) {
        _head = new node();
        _tail = new node();
        _head->next = _tail;
        _tail->prev = _head;
        for (node *p = other._head->next; p != other._tail; p = p->next)
            push_back(*(p->data));
    }

    virtual ~list() {
        clear();
        delete _head;
        delete _tail;
    }

    list &operator=(const list &other) {
        if (this == &other) return *this;
        clear();
        for (node *p = other._head->next; p != other._tail; p = p->next)
            push_back(*(p->data));
        return *this;
    }

    const T &front() const {
        if (_size == 0) throw container_is_empty();
        return *(_head->next->data);
    }

    const T &back() const {
        if (_size == 0) throw container_is_empty();
        return *(_tail->prev->data);
    }

    iterator begin() { return iterator(_head->next, this); }
    const_iterator cbegin() const { return const_iterator(_head->next, this); }

    iterator end() { return iterator(_tail, this); }
    const_iterator cend() const { return const_iterator(_tail, this); }

    virtual bool empty() const { return _size == 0; }
    virtual size_t size() const { return _size; }

    virtual void clear() {
        node *cur = _head->next;
        while (cur != _tail) {
            node *nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        _head->next = _tail;
        _tail->prev = _head;
        _size = 0;
    }

    // Insert value before pos; throw if iterator invalid or from another list
    virtual iterator insert(iterator pos, const T &value) {
        if (pos.lst != this || pos.ptr == nullptr)
            throw invalid_iterator();
        node *newNode = new node(value);
        insert(pos.ptr, newNode);
        return iterator(newNode, this);
    }

    // Remove element at pos; return iterator to next element
    virtual iterator erase(iterator pos) {
        if (pos.lst != this || pos.ptr == nullptr || pos.ptr->data == nullptr)
            throw invalid_iterator();
        node *nxt = pos.ptr->next;
        node *removed = erase(pos.ptr);
        delete removed;
        return iterator(nxt, this);
    }

    void push_back(const T &value) {
        insert(_tail, new node(value));
    }

    void pop_back() {
        if (_size == 0) throw container_is_empty();
        node *removed = erase(_tail->prev);
        delete removed;
    }

    void push_front(const T &value) {
        insert(_head->next, new node(value));
    }

    void pop_front() {
        if (_size == 0) throw container_is_empty();
        node *removed = erase(_head->next);
        delete removed;
    }

    // Sort in ascending order (may copy/move data)
    void sort() {
        if (_size <= 1) return;

        // Collect node pointers into array, sort by data value, relink
        node **arr = new node *[_size];
        size_t i = 0;
        for (node *p = _head->next; p != _tail; p = p->next)
            arr[i++] = p;

        sjtu::sort<node *>(arr, arr + _size,
            std::function<bool(node *const &, node *const &)>(
                [](node *const &a, node *const &b) {
                    return *(a->data) < *(b->data);
                }
            )
        );

        // Relink in sorted order
        _head->next = arr[0];
        arr[0]->prev = _head;
        for (size_t j = 0; j + 1 < _size; j++) {
            arr[j]->next = arr[j + 1];
            arr[j + 1]->prev = arr[j];
        }
        arr[_size - 1]->next = _tail;
        _tail->prev = arr[_size - 1];

        delete[] arr;
    }

    // Merge two sorted lists (no copy/move of data, only pointer manipulation)
    void merge(list &other) {
        if (this == &other) return;

        node *p = _head->next; // current position in this
        node *q = other._head->next; // current position in other

        while (p != _tail && q != other._tail) {
            if (*(q->data) < *(p->data)) {
                // Move q to before p in this list
                node *qnext = q->next;

                // Unlink q from other (no size update yet)
                q->prev->next = q->next;
                q->next->prev = q->prev;

                // Link q before p in this
                q->prev = p->prev;
                q->next = p;
                p->prev->next = q;
                p->prev = q;

                q = qnext;
                _size++;
                other._size--;
            } else {
                p = p->next;
            }
        }

        // Append remaining from other (all go to end of this)
        if (q != other._tail) {
            node *other_first = q;
            node *other_last = other._tail->prev;
            size_t remaining = other._size - 0;
            // Count remaining nodes
            size_t cnt = 0;
            for (node *t = q; t != other._tail; t = t->next) cnt++;

            // Recount remaining properly
            // (other._size was decremented as we moved nodes, so compute remaining)
            // Actually: cnt is correct count of remaining nodes
            remaining = cnt;

            // Detach remaining from other
            other_first->prev->next = other._tail;
            other._tail->prev = other_first->prev;
            other._size -= remaining;

            // Attach to end of this (before _tail)
            _tail->prev->next = other_first;
            other_first->prev = _tail->prev;
            other_last->next = _tail;
            _tail->prev = other_last;
            _size += remaining;
        }

        // other is now empty
        other._head->next = other._tail;
        other._tail->prev = other._head;
        other._size = 0;
    }

    // Reverse (no copy/move of data)
    void reverse() {
        if (_size <= 1) return;

        node *cur = _head;
        while (cur != nullptr) {
            // Swap prev and next
            node *tmp = cur->prev;
            cur->prev = cur->next;
            cur->next = tmp;
            // Move to what was originally next (now stored in prev after swap)
            cur = cur->prev;
        }
        // Swap head and tail
        node *tmp = _head;
        _head = _tail;
        _tail = tmp;
    }

    // Remove consecutive duplicates
    void unique() {
        if (_size <= 1) return;
        node *cur = _head->next;
        while (cur != _tail && cur->next != _tail) {
            if (*(cur->data) == *(cur->next->data)) {
                node *dup = erase(cur->next);
                delete dup;
            } else {
                cur = cur->next;
            }
        }
    }
};

} // namespace sjtu

#endif //SJTU_LIST_HPP
