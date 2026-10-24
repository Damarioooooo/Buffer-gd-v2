
#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <algorithm>
#include <deque>

using namespace geode::prelude;

// iOS Predictive Input Buffer prototype.
//
// IMPORTANT:
// This is a fixed-delay input queue, NOT a complete port of the
// original trajectory-based Predictive Input Buffer mod.
// It requires compilation and in-game testing.

namespace {

struct QueuedInput {
    bool down;
    int ticksLeft;
    bool player1;
};

class InputQueue {
public:
    bool forwardingQueued = false;
    std::deque<QueuedInput> queue;

    void reset() {
        queue.clear();
        forwardingQueued = false;
    }

    int delayTicks() const {
        auto mod = Mod::get();
        if (!mod) {
            return 0;
        }

        return static_cast<int>(
            std::max<int64_t>(
                0,
                mod->getSettingValue<int64_t>("buffer-ticks")
            )
        );
    }

    bool intercept(bool down, bool player1) {
        if (forwardingQueued || !PlayLayer::get() || !player1) {
            return true;
        }

        const int delay = delayTicks();

        if (delay <= 0) {
            return true;
        }

        queue.push_back({down, delay, player1});
        return false;
    }

    void tick(PlayLayer* layer) {
        if (!layer || queue.empty()) {
            return;
        }

        for (auto& input : queue) {
            if (input.ticksLeft > 0) {
                --input.ticksLeft;
            }
        }

        while (!queue.empty() && queue.front().ticksLeft <= 0) {
            const auto input = queue.front();
            queue.pop_front();

            forwardingQueued = true;
            layer->handleButton(input.down, 1, input.player1);
            forwardingQueued = false;
        }
    }
};

InputQueue g_queue;

} // namespace

class $modify(PredictiveBufferPlayLayer, PlayLayer) {
    bool init(
        GJGameLevel* level,
        bool useReplay,
        bool dontCreateObjects
    ) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

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
        if (
            button == 1 &&
            PlayLayer::get() &&
            !g_queue.forwardingQueued
        ) {
            // Leave player 2 unchanged.
            if (!isPlayer1) {
                GJBaseGameLayer::handleButton(
                    down, button, isPlayer1
                );
                return;
            }

            // If queued, do not forward the input yet.
            if (!g_queue.intercept(down, isPlayer1)) {
                return;
            }
        }

        GJBaseGameLayer::handleButton(
            down, button, isPlayer1
        );
    }
};

class $modify(PredictiveBufferPlayerObject, PlayerObject) {
    void update(float dt) {
        auto layer = PlayLayer::get();

        if (layer && this == layer->m_player1) {
            g_queue.tick(layer);
        }

        PlayerObject::update(dt);
    }
};
