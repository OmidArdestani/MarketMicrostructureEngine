#include <order_book.h>

using namespace MarketMicroStructure;

OrderBook::OrderBook(SymbolId symbol)
    : symbol_(std::move(symbol))
{}

void OrderBook::addOrder(const BookOrder& ord) 
{
    if (ord.side == Side::Buy)
    {
        auto& queue = bids_[ord.price];
        queue.push_back(ord);
        // Store iterator to the newly added order for O(1) lookup
        order_index_[ord.id] = OrderLocation{ord.side, ord.price, std::prev(queue.end())};
    } 
    else 
    {
        auto& queue = asks_[ord.price];
        queue.push_back(ord);
        // Store iterator to the newly added order for O(1) lookup
        order_index_[ord.id] = OrderLocation{ord.side, ord.price, std::prev(queue.end())};
    }
}

bool OrderBook::cancelOrder(OrderId id)
{
    // O(1) lookup in the index
    auto idx_it = order_index_.find(id);
    if (idx_it == order_index_.end())
    {
        return false; // Order not found
    }

    const auto& loc = idx_it->second;
    
    if (loc.side == Side::Buy)
    {
        auto price_it = bids_.find(loc.price);
        if (price_it != bids_.end())
        {
            price_it->second.erase(loc.it);
            if (price_it->second.empty())
            {
                bids_.erase(price_it);
            }
        }
    }
    else
    {
        auto price_it = asks_.find(loc.price);
        if (price_it != asks_.end())
        {
            price_it->second.erase(loc.it);
            if (price_it->second.empty())
            {
                asks_.erase(price_it);
            }
        }
    }

    // Remove from index
    order_index_.erase(idx_it);
    return true;
}

std::pair<std::vector<Trade>*, Quantity> OrderBook::matchIncoming(const BookOrder& incoming, std::uint64_t ts_ns)
{
    static std::vector<Trade> trades;
    Quantity remaining = incoming.qty;

    trades.clear();

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

            remaining   -= traded_qty;
            resting.qty -= traded_qty;

            if (resting.qty == 0)
            {
                // Remove fully filled order from index
                order_index_.erase(resting.id);
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

    return {&trades, remaining};
}

std::optional<BookLevel> OrderBook::bestBid() const
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

std::optional<BookLevel> OrderBook::bestAsk() const
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
