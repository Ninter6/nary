//
//  naEventListener.cpp
//  nary
//
//  Created by Ninter6 on 2023/8/21.
//

#include "naEventListener.hpp"

namespace nary {

std::unordered_map<int, bool> naEventListener::NormalKeyList = {
    {32, false}, {39, false}, {44, false}, {45, false}, {46, false}, {47, false}, {48, false}, {49, false}, {50, false}, {51, false}, {52, false}, {53, false}, {54, false}, {55, false}, {56, false}, {57, false}, {65, false}, {66, false}, {67, false}, {68, false}, {69, false}, {70, false}, {71, false}, {72, false}, {73, false}, {74, false}, {75, false}, {76, false}, {77, false}, {78, false}, {79, false}, {80, false}, {81, false}, {82, false}, {83, false}, {84, false}, {85, false}, {86, false}, {87, false}, {88, false}, {89, false}, {90, false}, {91, false}, {92, false}, {93, false}, {96, false}, {256, false}, {257, false}, {258, false}, {259, false}, {260, false}, {261, false}, {262, false}, {263, false}, {264, false}, {265, false}
};

naEventListener::naEventListener() {
    lastTime = currentTime = std::chrono::high_resolution_clock::now();
}

void naEventListener::Update(const naWin& window) {
    EVT_STATUS.MainKey = TestSpecialKey(window);
    EVT_STATUS.MinorKey = TestSpecialKey(window, EVT_STATUS.MainKey);
    
    TestNormalKey(window);
    
    lastMousePos = currMosePos;
    GetMousePos(window);
    isMouseMoved = abs(GetMousePosDelta().x) > 1e-7 || abs(GetMousePosDelta().y) > 1e-7;
    
    lastTime = currentTime;
    currentTime = std::chrono::high_resolution_clock::now();
}

int naEventListener::TestMouseButton(const naWin& window) {
    for (int i = 0; i < 3; i++) {
        if (window.getMouseButtom(i) == KeyState::Press) {
            return i + 1;
        }
    }
    return 0; // none pressed
}

int naEventListener::TestSpecialKey(const naWin& window, int ignore) {
    for (int i = 0; i < 8; i++) {
        if (i + 1 == ignore) continue;
        if (window.getKey(i + 340) == KeyState::Press) {
            return i + 1;
        }
    }
    return 0; // none pressed
}

void naEventListener::TestNormalKey(const naWin& window) {
    for (auto& i : NormalKeyList) {
        i.second = window.getKey(i.first) == KeyState::Press;
    }
}

mathpls::dvec2 naEventListener::GetMousePos(const naWin& window) {
    currMosePos = window.getMousePos();
    return currMosePos;
}

mathpls::dvec2 naEventListener::GetMousePosDelta() {
    return currMosePos - lastMousePos;
}

double naEventListener::DeltaTime() {
    return std::chrono::duration<double, std::chrono::seconds::period>(currentTime - lastTime).count();
}

void naEventListener::UpdateEvents(const naWin& window) const {
    if (EventList.empty()) return;
    
    InputEvent evt = EVT_STATUS;
    for (const auto& i : NormalKeyList) {
        if (i.second) {
            evt.NormalKey = i.first;
            TestEvent(evt);
            if (TestMouseButton(window)) {
                evt.MouseButton = TestMouseButton(window);
                TestEvent(evt);
                if (isMouseMoved) {
                    evt.MouseMove = true;
                    TestEvent(evt);
                }
            }
            evt.MouseButton = 0;
            if (isMouseMoved) {
                evt.MouseMove = true;
                TestEvent(evt);
            }
            evt.MouseMove = false;
        }
    }
    
    evt = {};
    TestEvent(evt);
    if (TestMouseButton(window)) {
        evt.MouseButton = TestMouseButton(window);
        TestEvent(evt);
        if (isMouseMoved) {
            evt.MouseMove = true;
            TestEvent(evt);
        }
    }
    evt = {};
    if (isMouseMoved) {
        evt.MouseMove = true;
        TestEvent(evt);
    }
}

void naEventListener::TestEvent(const InputEvent& evt) const {
    auto [abegin, aend] = EventList.equal_range(evt);
    for (auto i = abegin; i != aend; ++i) {
        i->second();
    }
}

}
