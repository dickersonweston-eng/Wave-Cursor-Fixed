#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/ui/OverlayManager.hpp>
#include <Geode/cocos/textures/CCTextureCache.h>
#include <Geode/utils/cocos.hpp>

#ifdef GEODE_IS_MACOS
#include <CoreGraphics/CoreGraphics.h>
#endif

void refreshCursorAppearance() {
    auto cursor = static_cast<SimplePlayer*>(OverlayManager::get()->getChildByIDRecursive("cursor-icon"_spr));
    if (!cursor) return;

    auto gm = GameManager::get();
    cursor->updatePlayerFrame(gm->getPlayerDart(), IconType::Wave);
    cursor->setColor(gm->colorForIdx(gm->getPlayerColor()));
    cursor->setSecondColor(gm->colorForIdx(gm->getPlayerColor2()));
    if (gm->getPlayerGlow()) {
        cursor->setGlowOutline(gm->colorForIdx(gm->getPlayerGlowColor()));
    } else {
        cursor->disableGlowOutline();
    }

    cursor->setScale(Mod::get()->getSettingValue<int>("cursor-size") / 100.f);
}
void refreshTrail() {
    auto overlay = OverlayManager::get();
    auto existing = overlay->getChildByID("cursor-trail"_spr);
    if (existing) {
        existing->removeFromParent();
    }

    if (!Mod::get()->getSettingValue<bool>("enable-trail")) return;

    auto gm = GameManager::get();
    auto streakTexture = "streak_0" + std::to_string(gm->getPlayerStreak()) + "_001.png";
    auto trail = CCMotionStreak::create(
        0.3f, 2.f, 10.f, ccWHITE,
        CCTextureCache::get()->addImage(streakTexture.c_str(), true)
    );
    trail->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
    trail->setID("cursor-trail"_spr);
    trail->setZOrder(10066);
    overlay->addChild(trail);
}

class BasicScheduler : public CCObject {
    int m_hideCounter = 0;
public:
    void update(float dt) {
        auto cursor = static_cast<CCNode*>(OverlayManager::get()->getChildByID("cursor"_spr));
        auto trail = static_cast<CCMotionStreak*>(OverlayManager::get()->getChildByID("cursor-trail"_spr));
        if (!cursor) return;

        bool showCursor = PlayLayer::get() == nullptr;

        cursor->setPosition(geode::cocos::getMousePos());
        cursor->setVisible(showCursor);

        if (trail) {
            auto icon = static_cast<SimplePlayer*>(OverlayManager::get()->getChildByIDRecursive("cursor-icon"_spr));
            if (icon) {
                trail->setPosition(cursor->convertToWorldSpace(icon->getPosition()));
            }
            trail->setVisible(showCursor);
        }

    #ifdef GEODE_IS_MACOS
        m_hideCounter++;
        if (m_hideCounter >= 300) {
            m_hideCounter = 0;
            CGDisplayShowCursor(kCGDirectMainDisplay);
            if (!Mod::get()->getSettingValue<bool>("show-cursor")) {
                CGDisplayHideCursor(kCGDirectMainDisplay);
            }
        }
    #endif
    }
};

$execute {
    auto cursorIcon = SimplePlayer::create(0);
    cursorIcon->setID("cursor-icon"_spr);

    auto cursor = CCNode::create();
    cursor->addChild(cursorIcon);
    cursor->setContentSize(cursorIcon->m_outlineSprite->getScaledContentSize());
    cursorIcon->setPosition(cursor->getContentSize() / 2);
    cursor->setAnchorPoint({0.77f, 0.5f});
    cursor->setRotation(240.f);
    cursor->setID("cursor"_spr);
    cursor->setZOrder(10067);
    OverlayManager::get()->addChild(cursor);

    refreshCursorAppearance();
    refreshTrail();

#ifdef GEODE_IS_MACOS
    if (!Mod::get()->getSettingValue<bool>("show-cursor")) {
        CGDisplayHideCursor(kCGDirectMainDisplay);
    }
#endif

    Loader::get()->queueInMainThread([] {
        CCScheduler::get()->scheduleUpdateForTarget(new BasicScheduler(), 2000, false);
    });

    listenForSettingChanges<int>("cursor-size", [](int) {
        refreshCursorAppearance();
    });
    listenForSettingChanges<bool>("enable-trail", [](bool) {
        refreshTrail();
    });

    listenForSettingChanges<bool>("show-cursor", [](bool value) {
    #ifdef GEODE_IS_MACOS
        if (value) {
            CGDisplayShowCursor(kCGDirectMainDisplay);
        } else {
            CGDisplayHideCursor(kCGDirectMainDisplay);
        }
    #endif
    });
}

$on_game(Loaded) {
    refreshCursorAppearance();
}