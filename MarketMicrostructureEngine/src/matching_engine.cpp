#include <matching_engine.h>
#include <ScopeTimer.hpp>

using namespace MarketMicroStructure;

MatchingEngine::MatchingEngine(MarketDataPublisher& md_pub)
    : md_pub_(md_pub)
{}

void MatchingEngine::addSymbol(SymbolId symbol) 
{
    books_.push_back(OrderBook{ symbol });
}

void MatchingEngine::handleNewOrder(const NewOrder& o, std::uint64_t ts_ns) 
{
    OrderBook* book{ nullptr };
    for(auto& item : books_)
    {
        if(item.symbol() == o.symbol)
        {
            book = &item;

            break;
        }
    }

    if(!book)
        return;

    BookOrder incoming{
        o.id,
        o.trader,
        o.qty,
        o.price,
        o.side,
        ts_ns
    };

    // Market order: treat price as crossing “infinite”
    if (o.type == OrderType::Market) 
    {
        static Price MaxPrice{ std::numeric_limits<Price>::max() };
        static Price MinPrice{ std::numeric_limits<Price>::min() };

        if (o.side == Side::Buy)
        {
            // big price to cross all asks
            incoming.price = MaxPrice;
        } 
        else 
        {
            incoming.price = MinPrice;
        }
    }

    // Match against book
    auto [trades, remaining] = book->matchIncoming(incoming, ts_ns);

    // Publish trades
    for (auto const& t : *trades)
    {
        md_pub_.publishTrade(t);
    }

    // If limit order has remaining qty & is allowed to rest
    if (o.type == OrderType::Limit && remaining > 0)
    {
        incoming.qty = remaining;
        book->addOrder(incoming);
    }

    // Publish top-of-book after each change
    TopOfBook tob;
    tob.symbol = o.symbol;
    if (auto best_bid = book->bestBid())
    {
        tob.best_bid = *best_bid;
        tob.valid = true;
    }
    if (auto best_ask = book->bestAsk())
    {
        tob.best_ask = *best_ask;
        tob.valid = tob.valid && true;
    }
    if (tob.valid) 
    {
        md_pub_.publishTopOfBook(tob);
    }
}

void MatchingEngine::handleCancel(const CancelOrder& c) 
{
    NScopeTimers::start("MatchingEngine::handleCancel");

    // Very naive: scan all books and cancel first match
    for (auto& book : books_)
    {
        if (book.cancelOrder(c.id)) 
        {
            // After cancel we could publish new top-of-book
            TopOfBook tob;
            tob.symbol = book.symbol();
            if (auto best_bid = book.bestBid()) 
            {
                tob.best_bid = *best_bid;
                tob.valid = true;
            }
            if (auto best_ask = book.bestAsk()) 
            {
                tob.best_ask = *best_ask;
                tob.valid = tob.valid && true;
            }
            if (tob.valid) 
            {
                md_pub_.publishTopOfBook(tob);
            }
            break;
        }
    }
    NScopeTimers::endAndLog("MatchingEngine::handleCancel");
}
