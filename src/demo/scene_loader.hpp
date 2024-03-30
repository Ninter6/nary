#include "se_tools.h"
#include "sejson.h"

#include "resource_path.h"

#include <fstream>
#include <queue>
#include <vector>

#include "Scene.hpp"
#include "ResourceManager.hpp"
#include "RenderResource.hpp"

class SceneLoader {
public:
    static void load(nary::Scene& scene, nary::AssetManager& am, nary::RenderResource& rr, std::string_view filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            WARNING_LOG("Failed to open file: {}", filename);
            return;
        }
        std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        st::JsonParser parser{str};
        st::Json json{parser.Parse()};

        if (!json.element.IsObject()) {
            WARNING_LOG("Failed to parse json: {}", filename);
            return;
        }

        auto& tex = json["texture"].AsArray();
        auto& mtl = json["material"].AsArray();
        auto& mesh = json["mesh"].AsArray();

        std::vector<uint32_t> tex_ids(tex.size());
        std::vector<uint32_t> mtl_ids(mtl.size());
        std::vector<uint32_t> mesh_ids(mesh.size());

        for (uint32_t i = 0; i < tex.size(); i++) {
            tex_ids[i] = am.loadImage(tex[i].AsString());
        }
        for (uint32_t i = 0; i < mtl.size(); i++) {
            auto& m = mtl[i];
            auto& base_color = m.AsObject()["base_color"].AsArray();
            uint32_t base_color_tex = m.AsObject().contains("base_color_tex") ? tex_ids[(size_t)m.AsObject()["base_color_tex"].AsNumber()] : 0;

            nary::Material mat;
            mat.baseColorFactor = mathpls::vec4(base_color[0].AsNumber(), base_color[1].AsNumber(), base_color[2].AsNumber(), base_color[3].AsNumber());
            mat.base_color_texture = base_color_tex;
            mat.metallicFactor = m.AsObject()["metallic"].AsNumber();
            mat.roughnessFactor = m.AsObject()["roughness"].AsNumber();

            mtl_ids[i] = rr.addMaterial(mat);
        }
        for (uint32_t i = 0; i < mesh.size(); i++) {
            mesh_ids[i] = am.loadModel(mesh[i].AsString());
        }

        std::queue<std::pair<nary::naGameObject::id_t, st::Json>> que;
        que.emplace(-1, json["root"]);
        while (!que.empty()) {
            st::Json& jobj = que.front().second;
            nary::naGameObject::id_t ID = -1;

            if (jobj.element.AsObject().contains("transform")) {
                auto go = nary::naGameObject::createGameObject();

                {
                    auto& transform = jobj["transform"].AsObject();
                    auto& translation = transform["translation"].AsArray();
                    auto& scale = transform["scale"].AsArray();
                    auto& rotation = transform["rotation"].AsArray();
                    go.transform().translation = mathpls::vec3(translation[0].AsNumber(), translation[1].AsNumber(), translation[2].AsNumber()),
                    go.transform().scale = mathpls::vec3(scale[0].AsNumber(), scale[1].AsNumber(), scale[2].AsNumber()),
                    go.transform().rotation = mathpls::vec3(rotation[0].AsNumber(), rotation[1].AsNumber(), rotation[2].AsNumber());
                }

                if (jobj.element.AsObject().contains("mesh")) {
                    go.addComponent<nary::MeshComponent>(mesh_ids[(size_t)jobj["mesh"].AsNumber()]);
                }
                if (jobj.element.AsObject().contains("material")) {
                    go.addComponent<nary::MaterialComponent>(mtl_ids[(size_t)jobj["material"].AsNumber()]);
                }

                if (que.front().first == -1)
                    ID = scene.addGameObject(std::move(go));
                else
                    ID = scene.addGameObject(std::move(go), que.front().first);
            }

            if (jobj.element.AsObject().contains("children")) {
                auto& children = jobj["children"].AsArray();
                for (auto& child : children) {
                    que.emplace(ID, child);
                }
            }

            que.pop();
        }
    }

};