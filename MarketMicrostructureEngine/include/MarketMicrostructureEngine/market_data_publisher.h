#pragma once

#include "types.h"
#include <vector>
#include <functional>

namespace MarketMicroStructure {

class MarketDataPublisher {
public:
    using TopOfBookHandler   = std::function<void(const TopOfBook&)>;
    using TradeHandler       = std::function<void(const Trade&)>;
    using DepthSnapshotHandler = std::function<void(const SymbolId&, const std::vector<BookLevel>& bids, const std::vector<BookLevel>& asks)>;

    void on_top_of_book(TopOfBookHandler cb)        { tob_handler_ = std::move(cb); }
    void on_trade(TradeHandler cb)                  { trade_handler_ = std::move(cb); }
    void on_depth_snapshot(DepthSnapshotHandler cb) { depth_handler_ = std::move(cb); }

    void publish_top_of_book(const TopOfBook& tob) const
    {
        if (tob_handler_) tob_handler_(tob);
    }

    void publish_trade(const Trade& t) const
    {
        if (trade_handler_) trade_handler_(t);
    }

    void publish_depth(const SymbolId& sym, const std::vector<BookLevel>& bids, const std::vector<BookLevel>& asks) const
    {
        if (depth_handler_) depth_handler_(sym, bids, asks);
    }

private:
    TopOfBookHandler    tob_handler_;
    TradeHandler        trade_handler_;
    DepthSnapshotHandler depth_handler_;
};

} // namespace MarketMicroStructure
