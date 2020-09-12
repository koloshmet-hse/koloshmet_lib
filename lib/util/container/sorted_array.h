#include <functional>
#include <algorithm>
#include <vector>
#include <compare>


template <typename T, typename TCmp = std::less<T>, typename TAllocator = std::allocator<T>>
class TSortedArray : private TCmp {
public:
    using const_iterator = typename std::vector<T, TAllocator>::const_iterator;

public:
    TSortedArray(TCmp cmp, const TAllocator& alloc)
        : TCmp(std::move(cmp))
        , Array(alloc)
    {}

    explicit TSortedArray(const TAllocator& alloc)
        : TSortedArray(TCmp(), alloc)
    {}

    explicit TSortedArray(TCmp cmp)
        : TSortedArray(std::move(cmp), TAllocator())
    {}

    TSortedArray()
        : TSortedArray(TCmp(), TAllocator())
    {}


    TSortedArray(std::initializer_list<T> lst, TCmp cmp, const TAllocator& alloc)
        : TCmp(std::move(cmp))
        , Array(lst, alloc)
    {
        Sort();
    }

    TSortedArray(std::initializer_list<T> lst, TCmp cmp)
        : TSortedArray(lst, std::move(cmp), TAllocator())
    {}

    TSortedArray(std::initializer_list<T> lst, const TAllocator& alloc)
        : TSortedArray(lst, TCmp(), alloc)
    {}

    TSortedArray(std::initializer_list<T> lst)
        : TSortedArray(lst, TCmp(), TAllocator())
    {}

    template <typename TIt>
    TSortedArray(TIt first, TIt last, TCmp cmp, const TAllocator& alloc)
        : TCmp(std::move(cmp))
        , Array(first, last, alloc)
    {
        Sort();
    }

    template <typename TIt>
    TSortedArray(TIt first, TIt last, TCmp cmp)
        : TSortedArray(first, last, std::move(cmp), TAllocator())
    {}

    template <typename TIt>
    TSortedArray(TIt first, TIt last, const TAllocator& alloc)
        : TSortedArray(first, last, TCmp(), alloc)
    {}

    template <typename TIt>
    TSortedArray(TIt first, TIt last)
        : TSortedArray(first, last, TCmp(), TAllocator())
    {}

    inline const_iterator insert(const T& val) {
        Array.push_back(val);
        return SiftUp();
    }

    inline const_iterator insert(T&& val) {
        Array.push_back(std::move(val));
        return SiftUp();
    }

    template <typename... TArgs>
    inline const_iterator emplace(TArgs&&... args) {
        Array.emplace_back(std::forward<TArgs>(args)...);
        return SiftUp();
    }

    template <typename TVal>
    [[nodiscard]]
    const_iterator find(const TVal& val) const {
        auto res = std::lower_bound(Array.begin(), Array.end(), val, GetCmp());
        if (res == Array.end() || GetCmp()(val, *res)) {
            return Array.end();
        }
        return res;
    }

    template <typename TVal>
    [[nodiscard]]
    const T& Get(const TVal& val) const {
        auto res = std::lower_bound(Array.begin(), Array.end(), val, GetCmp());
        if (res != Array.end() && !GetCmp()(val, *res)) {
            return *res;
        }
        throw std::out_of_range{"No such element"};
    }

    [[nodiscard]]
    inline const_iterator begin() const {
        return Array.begin();
    }

    [[nodiscard]]
    inline const_iterator end() const {
        return Array.end();
    }

    [[nodiscard]]
    inline std::size_t size() const {
        return Array.size();
    }

    [[nodiscard]]
    inline bool empty() const {
        return Array.empty();
    }

    template <typename TVal>
    inline bool contains(const TVal& val) const {
        return find(val) != end();
    }

    auto operator<=>(const TSortedArray& other) const = default;

private:
    inline void Sort() {
        std::sort(Array.begin(), Array.end(), GetCmp());
    }

    inline const TCmp& GetCmp() const {
        return *this;
    }

    const_iterator SiftUp() {
        auto curIt = std::prev(Array.end());
        decltype(curIt) prevIt;
        while (curIt != Array.begin() && GetCmp()(*curIt, *(prevIt = std::prev(curIt)))) {
            using std::swap;
            swap(*prevIt, *curIt);
            curIt = prevIt;
        }
        return curIt;
    }

private:
    std::vector<T, TAllocator> Array;
};
