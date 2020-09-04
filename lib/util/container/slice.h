#pragma once

#include <algorithm>
#include <functional>

#include <optional>

template <typename TContainer>
struct TSlicer {
    template <typename TIter>
    TContainer operator()(
        TIter begin,
        TIter end,
        const TContainer& /* seq */) const
    {
        return TContainer(begin, end);
    }
};

template <
    typename TContainer,
    typename TDelim,
    typename TSearcher = std::default_searcher<decltype(std::cbegin(std::declval<TDelim>()))>,
    typename TSlicer = TSlicer<TContainer>,
    typename TContainerHolder = TContainer,
    typename TDelimHolder = TDelim>
class TSlice : private TSlicer {
    static_assert(std::is_convertible_v<TContainerHolder, const TContainer&>);
    static_assert(std::is_convertible_v<TDelimHolder, const TDelim&>);

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
            : CurIt{end ? std::end(cont.Cont()) : std::begin(cont.Cont())}
            , Slice{end ? nullptr : std::addressof(cont)}
            , Cur{}
        {
            if (end) {
                return;
            }
            if (auto next = Slice->Next(CurIt)) {
                Cur = std::move(*next);
            } else {
                Slice = nullptr;
            }
        }

        TIterator& operator++() {
            if (Slice != nullptr) {
                if (auto next = Slice->Next(CurIt)) {
                    Cur = std::move(*next);
                } else {
                    Slice = nullptr;
                }
            }
            return *this;
        }

        const TIterator operator++(int) {
            auto res = *this;
            if (Slice != nullptr) {
                if (auto next = Slice->Next(CurIt)) {
                    Cur = std::move(*next);
                } else {
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
        TContainerHolder seq,
        TDelimHolder subSeq,
        TSlicer slicer = TSlicer{})
        : TSlicer{std::move(slicer)}
        , Seq{std::move(seq)}
        , SubSeq{std::move(subSeq)}
        , Searcher{std::cbegin(Delim()), std::cend(Delim())}
    {}

    template <typename TBinaryPredicate>
    TSlice(
        TContainerHolder seq,
        TDelimHolder subSeq,
        TSlicer slicer,
        TBinaryPredicate comparator)
        : TSlicer{std::move(slicer)}
        , Seq{std::move(seq)}
        , SubSeq{std::move(subSeq)}
        , Searcher{std::cbegin(Delim()), std::cend(Delim()), std::move(comparator)}
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
    std::optional<TContainer> Next(TContainerIterator& it) const {
        auto seq = Cont();
        if (it == std::end(seq)) {
            return {};
        }

        auto start = it;
        auto end = std::search(start, std::end(seq), Searcher);
        it = end;
        if (it != std::end(seq)) {
            std::advance(it, std::size(Delim()));
        }
        return (*this)(start, end, seq);
    }

    inline const TContainer& Cont() const {
        return static_cast<const TContainer&>(Seq);
    }

    inline const TDelim& Delim() const {
        return static_cast<const TDelim&>(SubSeq);
    }

private:
    TContainerHolder Seq;
    TDelimHolder SubSeq;
    TSearcher Searcher;
};
