
#include <market_data_publisher.h>

using namespace MarketMicroStructure;

void MarketDataPublisher::onTopOfBook(TopOfBookHandler cb)
{
    tob_handler_ = std::move(cb); 
}

void MarketDataPublisher::onTrade(TradeHandler cb)
{
    trade_handler_ = std::move(cb); 
}

void MarketDataPublisher::onDepthSnapshot(DepthSnapshotHandler cb)
{
    depth_handler_ = std::move(cb);
}

void MarketDataPublisher::publishTopOfBook(const TopOfBook& tob) const
{
    if (tob_handler_) tob_handler_(tob);
}

void MarketDataPublisher::publishTrade(const Trade& t) const
{
    if (trade_handler_) trade_handler_(t);
}

void MarketDataPublisher::publishDepth(const SymbolId& sym, const std::vector<BookLevel>& bids, const std::vector<BookLevel>& asks) const
{
    if (depth_handler_) depth_handler_(sym, bids, asks);
}