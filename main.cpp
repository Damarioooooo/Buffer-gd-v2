#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <algorithm>\n#include <deque>

using namespace geode::prelude;

// IMPORTANT:
// This is an iOS-targeted porting prototype, not a feature-complete port.
// It deliberately avoids Windows.h, Win32 console functions, and keyboard-only
// controls. It currently demonstrates the portable input interception point
// and a fixed tick queue. The original mod's trajectory simulation must be
// ported and tested before this can be considered equivalent.

namespace {
struct QueuedInput {
    bool down;
    int ticksLeft;
    bool player1;
};

class InputQueue {
public:
    bool active = false;
    bool forwardingQueued = false;
    bool lastPhysicalDown = false;
    std::deque<QueuedInput> queue;

    void reset() {
        queue.clear();
        active = false;
        forwardingQueued = false;
        lastPhysicalDown = false;
    }

    int delayTicks() const {
        auto mod = Mod::get();
        if (!mod) return 0;
        return std::max(0, mod->getSettingValue<int64_t>("buffer-ticks"));
    }

    // Returns true when the original input should pass through immediately.
    bool intercept(bool down, bool player1) {
        if (forwardingQueued || !PlayLayer::get() || !player1)
            return true;

        lastPhysicalDown = down;
        const int delay = delayTicks();
        if (delay <= 0)
            return true;

        // Keep press/release order by queueing each transition in FIFO order.
        queue.push_back({down, delay, player1});
        active = true;
        return false;
    }

    void tick(PlayLayer* layer) {
        if (!layer || queue.empty()) {
            active = !queue.empty();
            return;
        }

        for (auto& item : queue) {
            if (item.ticksLeft > 0)
                --item.ticksLeft;
        }

        while (!queue.empty() && queue.front().ticksLeft <= 0) {
            auto item = queue.front();
            queue.pop_front();
            forwardingQueued = true;
            layer->handleButton(item.down, 1, item.player1);
            forwardingQueued = false;
        }
        active = !queue.empty();
    }
};

InputQueue g_queue;
}

class $modify(PredictiveBufferPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;
        g_queue.reset();
        return true;
    }

    void onQuit() {
        g_queue.reset();
        PlayLayer::onQuit();
    }
};

class $modify(PredictiveBufferBaseLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        // Only intercept the standard jump input in normal gameplay.
        if (button == 1 && PlayLayer::get() && !g_queue.forwardingQueued) {
            // Leave player-2 input unchanged.
            if (!isPlayer1) {
                GJBaseGameLayer::handleButton(down, button, isPlayer1);
                return;
            }

            // A false return means the input was queued and must not reach
            // the game until its scheduled tick.
            if (!g_queue.intercept(down, isPlayer1))
                return;
        }

        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }
};

class $modify(PredictiveBufferPlayerObject, PlayerObject) {
    void update(float dt) {
        // Tick the queue from the live player update. Do not run this while
        // the original game is replaying queued input.
        if (PlayLayer::get() && this == PlayLayer::get()->m_player1)
            g_queue.tick(PlayLayer::get());

        PlayerObject::update(dt);
    }
};
