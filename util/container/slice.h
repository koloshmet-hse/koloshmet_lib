#pragma once

#include <util/memory/move_on_rvalue_ptr.h>

template <typename TContainer>
struct TSlicer {
    template <typename TIter>
    TContainer operator()(
        TIter begin,
        TIter end,
        const TContainer& /* delimiter */) const
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
class TSlice : private TSlicer {
    using TContainerIterator = decltype(std::cbegin(std::declval<TContainer>()));

public:
    class TIterator {
    public:
        using difference_type = ssize_t;
        using value_type = TContainer;
        using pointer = TContainer*;
        using reference = TContainer&;
        using iterator_category = std::input_iterator_tag;

    public:
        explicit TIterator(const TSlice& cont, bool end)
            : CurIt{end ? std::end(*cont.Seq) : std::begin(*cont.Seq)}
            , Slice{end ? nullptr : std::addressof(cont)}
            , Cur{}
        {
            if (end) {
                return;
            }
            Cur = Slice->Next(CurIt);
        }

        TIterator& operator++() {
            if (Slice != nullptr) {
                Cur = Slice->Next(CurIt);
                if (std::end(*Slice->Seq) == CurIt && std::empty(Cur)) {
                    Slice = nullptr;
                }
            }
            return *this;
        }

        const TIterator operator++(int) {
            auto res = *this;
            if (Slice != nullptr) {
                Cur = Slice->Next(CurIt);
                if (std::end(*Slice->Seq) == CurIt && std::empty(Cur)) {
                    Slice = nullptr;
                }
            }
            return res;
        }

        reference operator*() const {
            return Cur;
        }

        pointer operator->() const {
            return std::addressof(Cur);
        }

        bool operator==(const TIterator& it) const {
            return CurIt == it.CurIt && Slice == it.Slice;
        }

        bool operator!=(const TIterator& it) const {
            return Slice != it.Slice || CurIt != it.CurIt;
        }

    private:
        TContainerIterator CurIt;
        const TSlice* Slice;
        mutable TContainer Cur;
    };

public:
    TSlice(
        const TContainer& seq,
        const TIsDelim& isDelim,
        TSlicer slicer = TSlicer{})
        : TSlicer{std::move(slicer)}
        , Seq{seq}
        , IsDelim(isDelim)
    {}

    TSlice(
        TContainer&& seq,
        const TIsDelim& isDelim,
        TSlicer slicer = TSlicer{})
        : TSlicer{std::move(slicer)}
        , Seq{std::move(seq)}
        , IsDelim(isDelim)
    {}

    TIterator begin() {
        return TIterator{*this, false};
    }

    TIterator end() {
        return TIterator{*this, true};
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
    TContainer Next(TContainerIterator& it) const {
        if (it == std::end(*Seq)) {
            return {};
        }

        auto buffBeg = it;
        auto buffEnd = it;
        auto delimBeg = it;
        while (it != std::end(*Seq)) {
            ++it;
            if (IsDelim(delimBeg, it) < 0) {
                ++buffEnd;
                ++delimBeg;
            } else if (IsDelim(delimBeg, it) == 0) {
                return (*this)(buffBeg, buffEnd, *Seq);
            }
        }
        return (*this)(buffBeg, it, *Seq);
    }

private:
    TMoveOnRvaluePtr<TContainer> Seq;
    TIsDelim IsDelim;
};
