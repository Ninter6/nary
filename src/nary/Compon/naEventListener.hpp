//
//  naEventListener.hpp
//  nary
//
//  Created by Ninter6 on 2023/8/21.
//

#pragma once

#include "naGameObject.hpp"
#include "naWin.hpp"

#include "se_tools.h"

#include <unordered_map>
#include <functional>
#include <chrono>

namespace nary {

struct InputEvent {
    // 1:l_shift, 2:l_ctrl, 3:l_atl, 4:l_super, 5:r_shift, 6:r_ctrl, 7:r_atl, 8:r_super
    int MainKey;
    int MinorKey;
    // 32:space, 48~57:0~9, 65~90:A~Z, 256:esc, 257:entry, 258:tap, 259:backspace, 260:insert, 261:delete, →←↓↑:262~265
    int NormalKey;
    // 1:left, 2:right, 3:mid
    int MouseButton;
    bool MouseMove;
    
    size_t GetEV() const {
        size_t ev;
        ev = MouseMove ? MainKey | (MinorKey << 16) : MinorKey | (MainKey << 16);
        ev ^= (NormalKey*NormalKey << MouseButton) + MouseButton + MouseMove;
        return ev;
    }
    bool operator<(const InputEvent& o) const {
        return GetEV() < o.GetEV();
    }
    bool operator==(const InputEvent& o) const {
        return MainKey == o.MainKey && MinorKey == o.MinorKey && NormalKey == o.NormalKey && MouseButton == o.MouseButton && MouseMove == o.MouseMove;
    }
};

}

template<>
struct std::hash<nary::InputEvent> {
    size_t operator()(const nary::InputEvent& v) const {
        return v.GetEV();
    }
};

namespace nary {

class naEventListener {
public: // Global
    static std::unordered_map<int, bool> NormalKeyList; // externally defined
    static inline InputEvent EVT_STATUS;
    static void Update(const naWin& window);
    
    static int TestMouseButton(const naWin& window);
    
    static int TestSpecialKey(const naWin& window, int ignore = 0);
    static void TestNormalKey(const naWin& window);
    
    static inline bool isMouseMoved = false;
    static inline mathpls::dvec2 currMosePos{0}, lastMousePos{0};
    static mathpls::dvec2 GetMousePos(const naWin& window);
    static mathpls::dvec2 GetMousePosDelta();
    
    static inline std::chrono::high_resolution_clock::time_point currentTime, lastTime;
    static double DeltaTime();
    
public: // Instance
    naEventListener();
    
    std::unordered_multimap<InputEvent, std::function<void()>> EventList;
    
    void UpdateEvents(const naWin& window) const;
    void TestEvent(const InputEvent& evt) const;
    
};

}
