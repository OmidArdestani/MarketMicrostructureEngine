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

    void onTopOfBook(TopOfBookHandler cb)         { tob_handler_ = std::move(cb); }
    void onTrade(TradeHandler cb)                 { trade_handler_ = std::move(cb); }
    void onDepthSnapshot(DepthSnapshotHandler cb) { depth_handler_ = std::move(cb); }

    void publishTopOfBook(const TopOfBook& tob) const
    {
        if (tob_handler_) tob_handler_(tob);
    }

    void publishTrade(const Trade& t) const
    {
        if (trade_handler_) trade_handler_(t);
    }

    void publishDepth(const SymbolId& sym, const std::vector<BookLevel>& bids, const std::vector<BookLevel>& asks) const
    {
        if (depth_handler_) depth_handler_(sym, bids, asks);
    }

private:
    TopOfBookHandler    tob_handler_;
    TradeHandler        trade_handler_;
    DepthSnapshotHandler depth_handler_;
};

} // namespace MarketMicroStructure
