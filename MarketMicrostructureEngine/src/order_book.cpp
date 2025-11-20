#include "order_book.h"

namespace MarketMicroStructure {

OrderBook::OrderBook(SymbolId symbol)
    : symbol_(std::move(symbol))
{}

void OrderBook::add_order(const BookOrder& ord) 
{
    if (ord.side == Side::Buy)
    {
        bids_[ord.price].push_back(ord);
    } 
    else 
    {
        asks_[ord.price].push_back(ord);
    }
}

bool OrderBook::remove_from_side(OrderId id, std::map<Price, Queue, std::greater<Price>>& side)
{
    for (auto it = side.begin(); it != side.end(); )
    {
        auto& q = it->second;
        for (auto oit = q.begin(); oit != q.end(); ++oit)
        {
            if (oit->id == id)
            {
                q.erase(oit);
                if (q.empty())
                {
                    it = side.erase(it);
                }
                return true;
            }
        }
        ++it;
    }
    return false;
}

bool OrderBook::remove_from_side(OrderId id, std::map<Price, Queue, std::less<Price>>& side)
{
    for (auto it = side.begin(); it != side.end(); )
    {
        auto& q = it->second;
        for (auto oit = q.begin(); oit != q.end(); ++oit)
        {
            if (oit->id == id)
            {
                q.erase(oit);
                if (q.empty())
                {
                    it = side.erase(it);
                }
                return true;
            }
        }
        ++it;
    }

    return false;
}

bool OrderBook::cancel_order(OrderId id)
{
    if (remove_from_side(id, bids_)) return true;
    if (remove_from_side(id, asks_)) return true;

    return false;
}

std::pair<std::vector<Trade>, Quantity> OrderBook::match_incoming(const BookOrder& incoming, std::uint64_t ts_ns)
{
    std::vector<Trade> trades;
    Quantity remaining = incoming.qty;

    auto exec = [&](auto& book_side, Side aggressor_side) 
    {
        while (remaining > 0 && !book_side.empty()) 
        {
            auto it = book_side.begin();         // best price
            auto& queue = it->second;
            auto& resting = queue.front();

            // Price check
            bool price_cross = false;
            if (incoming.side == Side::Buy)
            {
                price_cross = incoming.price >= resting.price;
            }
            else
            {
                price_cross = incoming.price <= resting.price;
            }
            if (!price_cross) break;

            auto traded_qty = std::min(remaining, resting.qty);
            trades.push_back(Trade{
                resting.id,
                incoming.id,
                symbol_,
                aggressor_side,
                resting.price,
                traded_qty,
                ts_ns
            });

            remaining -= traded_qty;
            resting.qty -= traded_qty;

            if (resting.qty == 0)
            {
                queue.pop_front();
                if (queue.empty())
                {
                    book_side.erase(it);
                }
            }
        }
    };

    if (incoming.side == Side::Buy)
    {
        // Buy aggresses against asks (lowest first)
        exec(asks_, Side::Buy);
    }
    else
    {
        // Sell aggresses against bids (highest first)
        exec(bids_, Side::Sell);
    }

    return {trades, remaining};
}

std::optional<BookLevel> OrderBook::best_bid() const
{
    if (bids_.empty()) return std::nullopt;
    Price p = bids_.begin()->first;
    Quantity q = 0;
    for (auto const& ord : bids_.begin()->second)
    {
        q += ord.qty;
    }

    return BookLevel{p, q};
}

std::optional<BookLevel> OrderBook::best_ask() const
{
    if (asks_.empty()) return std::nullopt;
    Price p = asks_.begin()->first;
    Quantity q = 0;
    for (auto const& ord : asks_.begin()->second)
    {
        q += ord.qty;
    }

    return BookLevel{p, q};
}

std::vector<BookLevel> OrderBook::bids(std::size_t depth) const
{
    std::vector<BookLevel> out;
    out.reserve(depth);
    std::size_t count = 0;
    for (auto const& [price, q] : bids_)
    {
        if (count++ >= depth) break;
        Quantity total = 0;
        for (auto const& o : q) total += o.qty;
        out.push_back({price, total});
    }

    return out;
}

std::vector<BookLevel> OrderBook::asks(std::size_t depth) const
{
    std::vector<BookLevel> out;
    out.reserve(depth);
    std::size_t count = 0;
    for (auto const& [price, q] : asks_)
    {
        if (count++ >= depth) break;
        Quantity total = 0;
        for (auto const& o : q) total += o.qty;
        out.push_back({price, total});
    }

    return out;
}

} // namespace MarketMicroStructure
