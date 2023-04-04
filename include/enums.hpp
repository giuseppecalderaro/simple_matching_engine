#pragma once

namespace SimpleMatchingEngine {
    enum Command
    {
        INSERT = 0,
        AMEND  = 1,
        PULL   = 2
    };

    enum Side
    {
        UNKNOWN = -1,
        BUY  = 0,
        SELL = 1
    };
}
