#include "RenderManager.hpp"

namespace nary {

AssetManager::AssetManager(RenderManager& renderManager)
: pRenderManager(&renderManager) {}

UID AssetManager::loadModel(const std::string& filename) const {
    return pRenderManager->m_RenderResource->addMesh(naModel::createModelFromFile(*pRenderManager->m_Device, filename));
}

UID AssetManager::loadImage(const std::string& filename) const {
    return pRenderManager->m_RenderResource->addTexture(std::make_unique<naImage>(naImage::loadImageFromFile(*pRenderManager->m_Device, filename)));
}

}