#pragma once

#include <string>
#include <unordered_map>

namespace nary {

class RenderManager;

using UID = size_t;
constexpr UID invaild_uid = 0;
constexpr bool IsVaildUID(UID id) {return id != invaild_uid;}

template <class T>
class UidMap {
public:
    UidMap() = default;
    
    UID insert(const T& e) {
        m_Map.emplace(++currID, e);
        return currID;
    }
    UID insert(T&& e) {
        m_Map.emplace(++currID, std::move(e));
        return currID;
    }
    void erase(UID id) {
        m_Map.erase(id);
    }
    bool contains(UID id) {
        return m_Map.contains(id);
    } 

    T& operator[](UID id) {
        return m_Map[id];
    }
    const T& operator[](UID id) const {
        return m_Map.at(id);
    }

    auto begin() {return m_Map.begin();}
    auto end() {return m_Map.end();}
    auto begin() const {return m_Map.cbegin();}
    auto end() const {return m_Map.cend();}
    
private:
    UID currID = 0;
    std::unordered_map<UID, T> m_Map;
};

class AssetManager {
public:
    AssetManager(RenderManager& renderManager);

    UID loadModel(const std::string& filename) const;
    UID loadImage(const std::string& filename) const;

private:
    RenderManager* pRenderManager;
};

}