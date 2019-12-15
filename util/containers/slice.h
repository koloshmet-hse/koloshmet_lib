#pragma once

#include <util/memory/move_on_rvalue_ptr.h>

#include <vector>

template <typename TContainer>
struct TSlicer {
    template <typename TIter>
    TContainer operator()(
        TIter begin,
        TIter end,
        const TContainer& /*delimiter*/) const
    {
        return TContainer(begin, end);
    }
};

/*
    TIsDelim must return
    + if given subsequence satisfies the prefix of delimiter condition
    0 if given subsequence satisfies the delimiter condition
    - if given subsequence doesn't satisfy the prefix of delimiter condition
*/

template <
    typename TContainer,
    typename TIsDelim,
    typename TSlicer = TSlicer<TContainer>>
class TSlice {
public:
    class TIterator {
    public:
        using difference_type = ssize_t;
        using value_type = TContainer;
        using pointer = const TContainer*;
        using reference = const TContainer&;
        using iterator_category = std::input_iterator_tag;

    public:
        TIterator()
            : Elems{nullptr}
            , Iter{0}
        {}

        TIterator(long long pos, TSlice* elems)
            : Elems{elems}
            , Iter{pos}
        {
            while (static_cast<long long>(Elems->Elements.size()) <= Iter) {
                if (!Elems->Next()) {
                    Iter = -1;
                    break;
                }
            }
        }

        TIterator& operator++() {
            while (Elems->Elements.size() <= static_cast<std::size_t>(Iter + 1)) {
                if (!Elems->Next()) {
                    Iter = -1;
                    return *this;
                }
            }
            ++Iter;
            return *this;
        }

        const TIterator operator++(int) {
            auto res = *this;
            while (Elems->Elements.size() <= static_cast<std::size_t>(Iter + 1)) {
                if (!Elems->Next()) {
                    Iter = -1;
                    return res;
                }
            }
            ++Iter;
            return res;
        }

        reference operator*() const {
            return Elems->Elements[Iter];
        }

        pointer operator->() const {
            return std::addressof(Elems->Elements[Iter]);
        }

        bool operator==(const TIterator& it) const {
            return Elems == it.Elems && Iter == it.Iter;
        }

        bool operator!=(const TIterator& it) const {
            return Elems != it.Elems || Iter != it.Iter;
        }

    private:
        TSlice* Elems;
        long long Iter;
    };

public:
    TSlice(
        const TContainer& seq,
        const TIsDelim& isDelim,
        const TSlicer& slicer = TSlicer{})
        : Elements{}
        , Seq{seq}
        , CurIt{std::begin(seq)}
        , Slicer(slicer)
        , IsDelim(isDelim)
    {}

    TSlice(
        TContainer&& seq,
        const TIsDelim& isDelim,
        const TSlicer& slicer = TSlicer{})
        : Elements{}
        , Seq{std::move(seq)}
        , CurIt{std::begin(*Seq)}
        , Slicer(slicer)
        , IsDelim(isDelim)
    {}

    bool Next() {
        if (CurIt == std::end(*Seq)) {
            return false;
        }

        auto buffBeg = CurIt;
        auto buffEnd = CurIt;
        auto delimBeg = CurIt;
        while (CurIt != std::end(*Seq)) {
            ++CurIt;
            if (IsDelim(delimBeg, CurIt) < 0) {
                buffEnd = CurIt;
                delimBeg = CurIt;
            } else if (IsDelim(delimBeg, CurIt) == 0) {
                Elements.emplace_back(Slicer(buffBeg, buffEnd, *Seq));
                if (CurIt == std::end(*Seq)) {
                    Elements.emplace_back();
                }
                return true;
            }
        }
        if (IsDelim(delimBeg, CurIt) != 0) {
            buffEnd = CurIt;
            Elements.emplace_back(Slicer(buffBeg, buffEnd, *Seq));
        }
        return true;
    }

    TIterator begin() {
        return TIterator{0, this};
    }

    TIterator end() {
        return TIterator{-1, this};
    }

    template <typename TCont>
    /* implicit */ operator TCont() & {
        return TCont(begin(), end());
    }

    template <typename TCont>
    /* implicit */ operator TCont() && {
        return TCont(std::move_iterator{begin()}, std::move_iterator{end()});
    }

private:
    using TContainerIt = decltype(std::begin(std::declval<TContainer>()));

private:
    std::vector<TContainer> Elements;
    TMoveOnRvaluePtr<TContainer> Seq;
    TContainerIt CurIt;
    TSlicer Slicer;
    TIsDelim IsDelim;
};
