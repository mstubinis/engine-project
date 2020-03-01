#include "BuiltInMeshes.h"

#include <core/engine/mesh/Mesh.h>

#include <string>

using namespace std;
using namespace Engine::priv;

BuiltInMeshses::BuiltInMeshses() {
    m_BuiltInMeshes.resize(BuiltInMeshEnum::_TOTAL, nullptr);
}
BuiltInMeshses::~BuiltInMeshses() {
    cleanup();
}
const bool BuiltInMeshses::cleanup() {
    if (m_BuiltInMeshes.size() == 0) {
        return false;
    }
    SAFE_DELETE_VECTOR(m_BuiltInMeshes);
    return true;
}
 Mesh& BuiltInMeshses::getPointLightBounds() {
    return *m_BuiltInMeshes[BuiltInMeshEnum::PointLight];
}
 Mesh& BuiltInMeshses::getSpotLightBounds() {
    return *m_BuiltInMeshes[BuiltInMeshEnum::SpotLight];
}
 Mesh& BuiltInMeshses::getRodLightBounds() {
    return *m_BuiltInMeshes[BuiltInMeshEnum::RodLight];
}
 Mesh& BuiltInMeshses::getTriangleMesh() {
    return *m_BuiltInMeshes[BuiltInMeshEnum::Triangle];
}
 Mesh& BuiltInMeshses::getCubeMesh() {
    return *m_BuiltInMeshes[BuiltInMeshEnum::Cube];
}
 Mesh& BuiltInMeshses::getPlaneMesh() {
    return *m_BuiltInMeshes[BuiltInMeshEnum::Plane];
}
 Mesh& BuiltInMeshses::getFontMesh() {
    return *m_BuiltInMeshes[BuiltInMeshEnum::Font];
}
 Mesh& BuiltInMeshses::getParticleMesh() {
     return *m_BuiltInMeshes[BuiltInMeshEnum::Particle];
 }

const bool BuiltInMeshses::init() {
    if (m_BuiltInMeshes[0]) {
        return false;
    }
    if (m_BuiltInMeshes.size() == 0) {
        m_BuiltInMeshes.resize(BuiltInMeshEnum::_TOTAL, nullptr);
    }

    if (!build_point_light_mesh())
        return false;
    if (!build_spot_light_mesh())
        return false;
    if (!build_rod_light_mesh())
        return false;
    if (!build_triangle_mesh())
        return false;
    if (!build_cube_mesh())
        return false;
    if (!build_plane_mesh())
        return false;
    if (!build_font_mesh())
        return false;
    if (!build_particle_mesh())
        return false;

    return true;
}
const bool BuiltInMeshses::build_point_light_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::PointLight])
        return false;
    string pointLightMesh;
    {
        pointLightMesh =
            "v 0.000000 -1.000000 0.000000\n"
            "v 0.723607 -0.447220 0.525725\n"
            "v -0.276388 -0.447220 0.850649\n"
            "v -0.894426 -0.447216 0.000000\n"
            "v -0.276388 -0.447220 -0.850649\n"
            "v 0.723607 -0.447220 -0.525725\n"
            "v 0.276388 0.447220 0.850649\n"
            "v -0.723607 0.447220 0.525725\n"
            "v -0.723607 0.447220 -0.525725\n"
            "v 0.276388 0.447220 -0.850649\n"
            "v 0.894426 0.447216 0.000000\n"
            "v 0.000000 1.000000 0.000000\n"
            "v -0.232822 -0.657519 0.716563\n"
            "v -0.162456 -0.850654 0.499995\n"
            "v -0.077607 -0.967950 0.238853\n"
            "v 0.203181 -0.967950 0.147618\n"
            "v 0.425323 -0.850654 0.309011\n"
            "v 0.609547 -0.657519 0.442856\n"
            "v 0.531941 -0.502302 0.681712\n"
            "v 0.262869 -0.525738 0.809012\n"
            "v -0.029639 -0.502302 0.864184\n"
            "v 0.812729 -0.502301 -0.295238\n"
            "v 0.850648 -0.525736 0.000000\n"
            "v 0.812729 -0.502301 0.295238\n"
            "v 0.203181 -0.967950 -0.147618\n"
            "v 0.425323 -0.850654 -0.309011\n"
            "v 0.609547 -0.657519 -0.442856\n"
            "v -0.753442 -0.657515 0.000000\n"
            "v -0.525730 -0.850652 0.000000\n"
            "v -0.251147 -0.967949 0.000000\n"
            "v -0.483971 -0.502302 0.716565\n"
            "v -0.688189 -0.525736 0.499997\n"
            "v -0.831051 -0.502299 0.238853\n"
            "v -0.232822 -0.657519 -0.716563\n"
            "v -0.162456 -0.850654 -0.499995\n"
            "v -0.077607 -0.967950 -0.238853\n"
            "v -0.831051 -0.502299 -0.238853\n"
            "v -0.688189 -0.525736 -0.499997\n"
            "v -0.483971 -0.502302 -0.716565\n"
            "v -0.029639 -0.502302 -0.864184\n"
            "v 0.262869 -0.525738 -0.809012\n"
            "v 0.531941 -0.502302 -0.681712\n"
            "v 0.956626 0.251149 0.147618\n"
            "v 0.951058 -0.000000 0.309013\n"
            "v 0.860698 -0.251151 0.442858\n"
            "v 0.860698 -0.251151 -0.442858\n"
            "v 0.951058 0.000000 -0.309013\n"
            "v 0.956626 0.251149 -0.147618\n"
            "v 0.155215 0.251152 0.955422\n"
            "v 0.000000 -0.000000 1.000000\n"
            "v -0.155215 -0.251152 0.955422\n"
            "v 0.687159 -0.251152 0.681715\n"
            "v 0.587786 0.000000 0.809017\n"
            "v 0.436007 0.251152 0.864188\n"
            "v -0.860698 0.251151 0.442858\n"
            "v -0.951058 -0.000000 0.309013\n"
            "v -0.956626 -0.251149 0.147618\n"
            "v -0.436007 -0.251152 0.864188\n"
            "v -0.587786 0.000000 0.809017\n"
            "v -0.687159 0.251152 0.681715\n"
            "v -0.687159 0.251152 -0.681715\n"
            "v -0.587786 -0.000000 -0.809017\n"
            "v -0.436007 -0.251152 -0.864188\n"
            "v -0.956626 -0.251149 -0.147618\n"
            "v -0.951058 0.000000 -0.309013\n"
            "v -0.860698 0.251151 -0.442858\n"
            "v 0.436007 0.251152 -0.864188\n"
            "v 0.587786 -0.000000 -0.809017\n"
            "v 0.687159 -0.251152 -0.681715\n"
            "v -0.155215 -0.251152 -0.955422\n"
            "v 0.000000 0.000000 -1.000000\n"
            "v 0.155215 0.251152 -0.955422\n"
            "v 0.831051 0.502299 0.238853\n"
            "v 0.688189 0.525736 0.499997\n"
            "v 0.483971 0.502302 0.716565\n"
            "v 0.029639 0.502302 0.864184\n"
            "v -0.262869 0.525738 0.809012\n"
            "v -0.531941 0.502302 0.681712\n"
            "v -0.812729 0.502301 0.295238\n"
            "v -0.850648 0.525736 0.000000\n"
            "v -0.812729 0.502301 -0.295238\n"
            "v -0.531941 0.502302 -0.681712\n"
            "v -0.262869 0.525738 -0.809012\n"
            "v 0.029639 0.502302 -0.864184\n"
            "v 0.483971 0.502302 -0.716565\n"
            "v 0.688189 0.525736 -0.499997\n"
            "v 0.831051 0.502299 -0.238853\n"
            "v 0.077607 0.967950 0.238853\n"
            "v 0.162456 0.850654 0.499995\n"
            "v 0.232822 0.657519 0.716563\n"
            "v 0.753442 0.657515 0.000000\n"
            "v 0.525730 0.850652 0.000000\n"
            "v 0.251147 0.967949 0.000000\n"
            "v -0.203181 0.967950 0.147618\n"
            "v -0.425323 0.850654 0.309011\n"
            "v -0.609547 0.657519 0.442856\n"
            "v -0.203181 0.967950 -0.147618\n"
            "v -0.425323 0.850654 -0.309011\n"
            "v -0.609547 0.657519 -0.442856\n"
            "v 0.077607 0.967950 -0.238853\n"
            "v 0.162456 0.850654 -0.499995\n"
            "v 0.232822 0.657519 -0.716563\n"
            "v 0.361800 0.894429 -0.262863\n"
            "v 0.638194 0.723610 -0.262864\n"
            "v 0.447209 0.723612 -0.525728\n"
            "v -0.138197 0.894430 -0.425319\n"
            "v -0.052790 0.723612 -0.688185\n"
            "v -0.361804 0.723612 -0.587778\n"
            "v -0.447210 0.894429 0.000000\n"
            "v -0.670817 0.723611 -0.162457\n"
            "v -0.670817 0.723611 0.162457\n"
            "v -0.138197 0.894430 0.425319\n"
            "v -0.361804 0.723612 0.587778\n"
            "v -0.052790 0.723612 0.688185\n"
            "v 0.361800 0.894429 0.262863\n"
            "v 0.447209 0.723612 0.525728\n"
            "v 0.638194 0.723610 0.262864\n"
            "v 0.861804 0.276396 -0.425322\n"
            "v 0.809019 0.000000 -0.587782\n"
            "v 0.670821 0.276397 -0.688189\n"
            "v -0.138199 0.276397 -0.951055\n"
            "v -0.309016 -0.000000 -0.951057\n"
            "v -0.447215 0.276397 -0.850649\n"
            "v -0.947213 0.276396 -0.162458\n"
            "v -1.000000 0.000001 0.000000\n"
            "v -0.947213 0.276397 0.162458\n"
            "v -0.447216 0.276397 0.850648\n"
            "v -0.309017 -0.000001 0.951056\n"
            "v -0.138199 0.276397 0.951055\n"
            "v 0.670820 0.276396 0.688190\n"
            "v 0.809019 -0.000002 0.587783\n"
            "v 0.861804 0.276394 0.425323\n"
            "v 0.309017 -0.000000 -0.951056\n"
            "v 0.447216 -0.276398 -0.850648\n"
            "v 0.138199 -0.276398 -0.951055\n"
            "v -0.809018 -0.000000 -0.587783\n"
            "v -0.670819 -0.276397 -0.688191\n"
            "v -0.861803 -0.276396 -0.425324\n"
            "v -0.809018 0.000000 0.587783\n"
            "v -0.861803 -0.276396 0.425324\n"
            "v -0.670819 -0.276397 0.688191\n"
            "v 0.309017 0.000000 0.951056\n"
            "v 0.138199 -0.276398 0.951055\n"
            "v 0.447216 -0.276398 0.850648\n"
            "v 1.000000 0.000000 0.000000\n"
            "v 0.947213 -0.276396 0.162458\n"
            "v 0.947213 -0.276396 -0.162458\n"
            "v 0.361803 -0.723612 -0.587779\n"
            "v 0.138197 -0.894429 -0.425321\n"
            "v 0.052789 -0.723611 -0.688186\n"
            "v -0.447211 -0.723612 -0.525727\n"
            "v -0.361801 -0.894429 -0.262863\n"
            "v -0.638195 -0.723609 -0.262863\n"
            "v -0.638195 -0.723609 0.262864\n"
            "v -0.361801 -0.894428 0.262864\n"
            "v -0.447211 -0.723610 0.525729\n"
            "v 0.670817 -0.723611 -0.162457\n"
            "v 0.670818 -0.723610 0.162458\n"
            "v 0.447211 -0.894428 0.000001\n"
            "v 0.052790 -0.723612 0.688185\n"
            "v 0.138199 -0.894429 0.425321\n"
            "v 0.361805 -0.723611 0.587779\n"
            "f 1 16 15\n"
            "f 2 18 24\n"
            "f 1 15 30\n"
            "f 1 30 36\n"
            "f 1 36 25\n"
            "f 2 24 45\n"
            "f 3 21 51\n"
            "f 4 33 57\n"
            "f 5 39 63\n"
            "f 6 42 69\n"
            "f 2 45 52\n"
            "f 3 51 58\n"
            "f 4 57 64\n"
            "f 5 63 70\n"
            "f 6 69 46\n"
            "f 7 75 90\n"
            "f 8 78 96\n"
            "f 9 81 99\n"
            "f 10 84 102\n"
            "f 11 87 91\n"
            "f 93 100 12\n"
            "f 92 103 93\n"
            "f 91 104 92\n"
            "f 93 103 100\n"
            "f 103 101 100\n"
            "f 92 104 103\n"
            "f 104 105 103\n"
            "f 103 105 101\n"
            "f 105 102 101\n"
            "f 91 87 104\n"
            "f 87 86 104\n"
            "f 104 86 105\n"
            "f 86 85 105\n"
            "f 105 85 102\n"
            "f 85 10 102\n"
            "f 100 97 12\n"
            "f 101 106 100\n"
            "f 102 107 101\n"
            "f 100 106 97\n"
            "f 106 98 97\n"
            "f 101 107 106\n"
            "f 107 108 106\n"
            "f 106 108 98\n"
            "f 108 99 98\n"
            "f 102 84 107\n"
            "f 84 83 107\n"
            "f 107 83 108\n"
            "f 83 82 108\n"
            "f 108 82 99\n"
            "f 82 9 99\n"
            "f 97 94 12\n"
            "f 98 109 97\n"
            "f 99 110 98\n"
            "f 97 109 94\n"
            "f 109 95 94\n"
            "f 98 110 109\n"
            "f 110 111 109\n"
            "f 109 111 95\n"
            "f 111 96 95\n"
            "f 99 81 110\n"
            "f 81 80 110\n"
            "f 110 80 111\n"
            "f 80 79 111\n"
            "f 111 79 96\n"
            "f 79 8 96\n"
            "f 94 88 12\n"
            "f 95 112 94\n"
            "f 96 113 95\n"
            "f 94 112 88\n"
            "f 112 89 88\n"
            "f 95 113 112\n"
            "f 113 114 112\n"
            "f 112 114 89\n"
            "f 114 90 89\n"
            "f 96 78 113\n"
            "f 78 77 113\n"
            "f 113 77 114\n"
            "f 77 76 114\n"
            "f 114 76 90\n"
            "f 76 7 90\n"
            "f 88 93 12\n"
            "f 89 115 88\n"
            "f 90 116 89\n"
            "f 88 115 93\n"
            "f 115 92 93\n"
            "f 89 116 115\n"
            "f 116 117 115\n"
            "f 115 117 92\n"
            "f 117 91 92\n"
            "f 90 75 116\n"
            "f 75 74 116\n"
            "f 116 74 117\n"
            "f 74 73 117\n"
            "f 117 73 91\n"
            "f 73 11 91\n"
            "f 48 87 11\n"
            "f 47 118 48\n"
            "f 46 119 47\n"
            "f 48 118 87\n"
            "f 118 86 87\n"
            "f 47 119 118\n"
            "f 119 120 118\n"
            "f 118 120 86\n"
            "f 120 85 86\n"
            "f 46 69 119\n"
            "f 69 68 119\n"
            "f 119 68 120\n"
            "f 68 67 120\n"
            "f 120 67 85\n"
            "f 67 10 85\n"
            "f 72 84 10\n"
            "f 71 121 72\n"
            "f 70 122 71\n"
            "f 72 121 84\n"
            "f 121 83 84\n"
            "f 71 122 121\n"
            "f 122 123 121\n"
            "f 121 123 83\n"
            "f 123 82 83\n"
            "f 70 63 122\n"
            "f 63 62 122\n"
            "f 122 62 123\n"
            "f 62 61 123\n"
            "f 123 61 82\n"
            "f 61 9 82\n"
            "f 66 81 9\n"
            "f 65 124 66\n"
            "f 64 125 65\n"
            "f 66 124 81\n"
            "f 124 80 81\n"
            "f 65 125 124\n"
            "f 125 126 124\n"
            "f 124 126 80\n"
            "f 126 79 80\n"
            "f 64 57 125\n"
            "f 57 56 125\n"
            "f 125 56 126\n"
            "f 56 55 126\n"
            "f 126 55 79\n"
            "f 55 8 79\n"
            "f 60 78 8\n"
            "f 59 127 60\n"
            "f 58 128 59\n"
            "f 60 127 78\n"
            "f 127 77 78\n"
            "f 59 128 127\n"
            "f 128 129 127\n"
            "f 127 129 77\n"
            "f 129 76 77\n"
            "f 58 51 128\n"
            "f 51 50 128\n"
            "f 128 50 129\n"
            "f 50 49 129\n"
            "f 129 49 76\n"
            "f 49 7 76\n"
            "f 54 75 7\n"
            "f 53 130 54\n"
            "f 52 131 53\n"
            "f 54 130 75\n"
            "f 130 74 75\n"
            "f 53 131 130\n"
            "f 131 132 130\n"
            "f 130 132 74\n"
            "f 132 73 74\n"
            "f 52 45 131\n"
            "f 45 44 131\n"
            "f 131 44 132\n"
            "f 44 43 132\n"
            "f 132 43 73\n"
            "f 43 11 73\n"
            "f 67 72 10\n"
            "f 68 133 67\n"
            "f 69 134 68\n"
            "f 67 133 72\n"
            "f 133 71 72\n"
            "f 68 134 133\n"
            "f 134 135 133\n"
            "f 133 135 71\n"
            "f 135 70 71\n"
            "f 69 42 134\n"
            "f 42 41 134\n"
            "f 134 41 135\n"
            "f 41 40 135\n"
            "f 135 40 70\n"
            "f 40 5 70\n"
            "f 61 66 9\n"
            "f 62 136 61\n"
            "f 63 137 62\n"
            "f 61 136 66\n"
            "f 136 65 66\n"
            "f 62 137 136\n"
            "f 137 138 136\n"
            "f 136 138 65\n"
            "f 138 64 65\n"
            "f 63 39 137\n"
            "f 39 38 137\n"
            "f 137 38 138\n"
            "f 38 37 138\n"
            "f 138 37 64\n"
            "f 37 4 64\n"
            "f 55 60 8\n"
            "f 56 139 55\n"
            "f 57 140 56\n"
            "f 55 139 60\n"
            "f 139 59 60\n"
            "f 56 140 139\n"
            "f 140 141 139\n"
            "f 139 141 59\n"
            "f 141 58 59\n"
            "f 57 33 140\n"
            "f 33 32 140\n"
            "f 140 32 141\n"
            "f 32 31 141\n"
            "f 141 31 58\n"
            "f 31 3 58\n"
            "f 49 54 7\n"
            "f 50 142 49\n"
            "f 51 143 50\n"
            "f 49 142 54\n"
            "f 142 53 54\n"
            "f 50 143 142\n"
            "f 143 144 142\n"
            "f 142 144 53\n"
            "f 144 52 53\n"
            "f 51 21 143\n"
            "f 21 20 143\n"
            "f 143 20 144\n"
            "f 20 19 144\n"
            "f 144 19 52\n"
            "f 19 2 52\n"
            "f 43 48 11\n"
            "f 44 145 43\n"
            "f 45 146 44\n"
            "f 43 145 48\n"
            "f 145 47 48\n"
            "f 44 146 145\n"
            "f 146 147 145\n"
            "f 145 147 47\n"
            "f 147 46 47\n"
            "f 45 24 146\n"
            "f 24 23 146\n"
            "f 146 23 147\n"
            "f 23 22 147\n"
            "f 147 22 46\n"
            "f 22 6 46\n"
            "f 27 42 6\n"
            "f 26 148 27\n"
            "f 25 149 26\n"
            "f 27 148 42\n"
            "f 148 41 42\n"
            "f 26 149 148\n"
            "f 149 150 148\n"
            "f 148 150 41\n"
            "f 150 40 41\n"
            "f 25 36 149\n"
            "f 36 35 149\n"
            "f 149 35 150\n"
            "f 35 34 150\n"
            "f 150 34 40\n"
            "f 34 5 40\n"
            "f 34 39 5\n"
            "f 35 151 34\n"
            "f 36 152 35\n"
            "f 34 151 39\n"
            "f 151 38 39\n"
            "f 35 152 151\n"
            "f 152 153 151\n"
            "f 151 153 38\n"
            "f 153 37 38\n"
            "f 36 30 152\n"
            "f 30 29 152\n"
            "f 152 29 153\n"
            "f 29 28 153\n"
            "f 153 28 37\n"
            "f 28 4 37\n"
            "f 28 33 4\n"
            "f 29 154 28\n"
            "f 30 155 29\n"
            "f 28 154 33\n"
            "f 154 32 33\n"
            "f 29 155 154\n"
            "f 155 156 154\n"
            "f 154 156 32\n"
            "f 156 31 32\n"
            "f 30 15 155\n"
            "f 15 14 155\n"
            "f 155 14 156\n"
            "f 14 13 156\n"
            "f 156 13 31\n"
            "f 13 3 31\n"
            "f 22 27 6\n"
            "f 23 157 22\n"
            "f 24 158 23\n"
            "f 22 157 27\n"
            "f 157 26 27\n"
            "f 23 158 157\n"
            "f 158 159 157\n"
            "f 157 159 26\n"
            "f 159 25 26\n"
            "f 24 18 158\n"
            "f 18 17 158\n"
            "f 158 17 159\n"
            "f 17 16 159\n"
            "f 159 16 25\n"
            "f 16 1 25\n"
            "f 13 21 3\n"
            "f 14 160 13\n"
            "f 15 161 14\n"
            "f 13 160 21\n"
            "f 160 20 21\n"
            "f 14 161 160\n"
            "f 161 162 160\n"
            "f 160 162 20\n"
            "f 162 19 20\n"
            "f 15 16 161\n"
            "f 16 17 161\n"
            "f 161 17 162\n"
            "f 17 18 162\n"
            "f 162 18 19\n"
            "f 18 2 19";
    }

    m_BuiltInMeshes[BuiltInMeshEnum::PointLight] = NEW Mesh(pointLightMesh, 0.0005f);
    return true;
}
const bool BuiltInMeshses::build_spot_light_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::SpotLight])
        return false;
    string spotLightData;
    {
        spotLightData =
            "v 0.000000 1.000000 -1.000000\n"
            "v 0.222521 0.974928 -1.000000\n"
            "v 0.433884 0.900969 -1.000000\n"
            "v 0.623490 0.781831 -1.000000\n"
            "v 0.781831 0.623490 -1.000000\n"
            "v 0.900969 0.433884 -1.000000\n"
            "v 0.000000 -0.000000 0.000000\n"
            "v 0.974928 0.222521 -1.000000\n"
            "v 1.000000 -0.000000 -1.000000\n"
            "v 0.974928 -0.222521 -1.000000\n"
            "v 0.900969 -0.433884 -1.000000\n"
            "v 0.781831 -0.623490 -1.000000\n"
            "v 0.623490 -0.781832 -1.000000\n"
            "v 0.433884 -0.900969 -1.000000\n"
            "v 0.222521 -0.974928 -1.000000\n"
            "v -0.000000 -1.000000 -1.000000\n"
            "v -0.222521 -0.974928 -1.000000\n"
            "v -0.433884 -0.900969 -1.000000\n"
            "v -0.623490 -0.781831 -1.000000\n"
            "v -0.781832 -0.623489 -1.000000\n"
            "v -0.900969 -0.433883 -1.000000\n"
            "v -0.974928 -0.222520 -1.000000\n"
            "v -1.000000 0.000001 -1.000000\n"
            "v -0.974928 0.222522 -1.000000\n"
            "v -0.900968 0.433885 -1.000000\n"
            "v -0.781831 0.623491 -1.000000\n"
            "v -0.623489 0.781832 -1.000000\n"
            "v -0.433882 0.900969 -1.000000\n"
            "v -0.222519 0.974928 -1.000000\n"
            "f 1 7 2\n"
            "f 2 7 3\n"
            "f 3 7 4\n"
            "f 4 7 5\n"
            "f 5 7 6\n"
            "f 6 7 8\n"
            "f 8 7 9\n"
            "f 9 7 10\n"
            "f 10 7 11\n"
            "f 11 7 12\n"
            "f 12 7 13\n"
            "f 13 7 14\n"
            "f 14 7 15\n"
            "f 15 7 16\n"
            "f 16 7 17\n"
            "f 17 7 18\n"
            "f 18 7 19\n"
            "f 19 7 20\n"
            "f 20 7 21\n"
            "f 21 7 22\n"
            "f 22 7 23\n"
            "f 23 7 24\n"
            "f 24 7 25\n"
            "f 25 7 26\n"
            "f 26 7 27\n"
            "f 27 7 28\n"
            "f 28 7 29\n"
            "f 29 7 1\n"
            "f 17 1 16\n"
            "f 1 15 16\n"
            "f 3 15 2\n"
            "f 4 14 3\n"
            "f 18 29 17\n"
            "f 19 28 18\n"
            "f 19 26 27\n"
            "f 20 25 26\n"
            "f 22 25 21\n"
            "f 23 24 22\n"
            "f 5 13 4\n"
            "f 6 12 5\n"
            "f 8 11 6\n"
            "f 9 10 8\n"
            "f 17 29 1\n"
            "f 1 2 15\n"
            "f 3 14 15\n"
            "f 4 13 14\n"
            "f 18 28 29\n"
            "f 19 27 28\n"
            "f 19 20 26\n"
            "f 20 21 25\n"
            "f 22 24 25\n"
            "f 5 12 13\n"
            "f 6 11 12\n"
            "f 8 10 11";
    }

    m_BuiltInMeshes[BuiltInMeshEnum::SpotLight] = NEW Mesh(spotLightData, 0.0005f);
    return true;
}
const bool BuiltInMeshses::build_rod_light_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::RodLight])
        return false;
    string rodLightData;
    {
        rodLightData =
            "v -0.000000 1.000000 -1.000000\n"
            "v -0.000000 1.000000 1.000000\n"
            "v 0.284630 0.959493 -1.000000\n"
            "v 0.284630 0.959493 1.000000\n"
            "v 0.546200 0.841254 -1.000000\n"
            "v 0.546200 0.841254 1.000000\n"
            "v 0.763521 0.654861 -1.000000\n"
            "v 0.763521 0.654861 1.000000\n"
            "v 0.918986 0.415415 -1.000000\n"
            "v 0.918986 0.415415 1.000000\n"
            "v 1.000000 0.142315 -1.000000\n"
            "v 1.000000 0.142315 1.000000\n"
            "v 1.000000 -0.142315 -1.000000\n"
            "v 1.000000 -0.142315 1.000000\n"
            "v 0.918986 -0.415415 -1.000000\n"
            "v 0.918986 -0.415415 1.000000\n"
            "v 0.763521 -0.654860 -1.000000\n"
            "v 0.763521 -0.654861 1.000000\n"
            "v 0.546200 -0.841253 -1.000000\n"
            "v 0.546200 -0.841253 1.000000\n"
            "v 0.284630 -0.959493 -1.000000\n"
            "v 0.284630 -0.959493 1.000000\n"
            "v 0.000000 -1.000000 -1.000000\n"
            "v 0.000000 -1.000000 1.000000\n"
            "v -0.284629 -0.959493 -1.000000\n"
            "v -0.284629 -0.959493 1.000000\n"
            "v -0.546200 -0.841253 -1.000000\n"
            "v -0.546200 -0.841254 1.000000\n"
            "v -0.763521 -0.654861 -1.000000\n"
            "v -0.763521 -0.654861 1.000000\n"
            "v -0.918986 -0.415415 -1.000000\n"
            "v -0.918986 -0.415415 1.000000\n"
            "v -1.000000 -0.142315 -1.000000\n"
            "v -1.000000 -0.142315 1.000000\n"
            "v -1.000000 0.142314 -1.000000\n"
            "v -1.000000 0.142314 1.000000\n"
            "v -0.918986 0.415415 -1.000000\n"
            "v -0.918986 0.415414 1.000000\n"
            "v -0.763522 0.654860 -1.000000\n"
            "v -0.763522 0.654860 1.000000\n"
            "v -0.546201 0.841253 -1.000000\n"
            "v -0.546201 0.841253 1.000000\n"
            "v -0.284631 0.959493 -1.000000\n"
            "v -0.284631 0.959493 1.000000\n"
            "f 2 3 1\n"
            "f 4 5 3\n"
            "f 6 7 5\n"
            "f 8 9 7\n"
            "f 10 11 9\n"
            "f 12 13 11\n"
            "f 14 15 13\n"
            "f 16 17 15\n"
            "f 18 19 17\n"
            "f 20 21 19\n"
            "f 21 24 23\n"
            "f 24 25 23\n"
            "f 26 27 25\n"
            "f 28 29 27\n"
            "f 29 32 31\n"
            "f 32 33 31\n"
            "f 34 35 33\n"
            "f 35 38 37\n"
            "f 38 39 37\n"
            "f 39 42 41\n"
            "f 25 1 23\n"
            "f 42 43 41\n"
            "f 44 1 43\n"
            "f 3 23 1\n"
            "f 27 43 25\n"
            "f 29 41 27\n"
            "f 31 39 29\n"
            "f 33 37 31\n"
            "f 5 21 3\n"
            "f 7 19 5\n"
            "f 9 17 7\n"
            "f 11 15 9\n"
            "f 2 22 4\n"
            "f 24 44 26\n"
            "f 26 42 28\n"
            "f 28 40 30\n"
            "f 30 38 32\n"
            "f 32 36 34\n"
            "f 6 22 20\n"
            "f 8 20 18\n"
            "f 8 16 10\n"
            "f 12 16 14\n"
            "f 2 4 3\n"
            "f 4 6 5\n"
            "f 6 8 7\n"
            "f 8 10 9\n"
            "f 10 12 11\n"
            "f 12 14 13\n"
            "f 14 16 15\n"
            "f 16 18 17\n"
            "f 18 20 19\n"
            "f 20 22 21\n"
            "f 21 22 24\n"
            "f 24 26 25\n"
            "f 26 28 27\n"
            "f 28 30 29\n"
            "f 29 30 32\n"
            "f 32 34 33\n"
            "f 34 36 35\n"
            "f 35 36 38\n"
            "f 38 40 39\n"
            "f 39 40 42\n"
            "f 25 43 1\n"
            "f 42 44 43\n"
            "f 44 2 1\n"
            "f 3 21 23\n"
            "f 27 41 43\n"
            "f 29 39 41\n"
            "f 31 37 39\n"
            "f 33 35 37\n"
            "f 5 19 21\n"
            "f 7 17 19\n"
            "f 9 15 17\n"
            "f 11 13 15\n"
            "f 2 24 22\n"
            "f 24 2 44\n"
            "f 26 44 42\n"
            "f 28 42 40\n"
            "f 30 40 38\n"
            "f 32 38 36\n"
            "f 6 4 22\n"
            "f 8 6 20\n"
            "f 8 18 16\n"
            "f 12 10 16";
    }

    m_BuiltInMeshes[BuiltInMeshEnum::RodLight] = NEW Mesh(rodLightData, 0.0005f);
    return true;
}
const bool BuiltInMeshses::build_triangle_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::Triangle])
        return false;
    string triangleMesh;
    {
        triangleMesh =
            "v 0.0 -0.948008 0.0\n"
            "v -1.0 0.689086 0.0\n"
            "v 1.0 0.689086 0.0\n"
            "vt 0.5 0.0\n"
            "vt 1.0 1.0\n"
            "vt 0.0 1.0\n"
            "vn 0.0 0.0 1.0\n"
            "f 1/1/1 3/2/1 2/3/1";
    }

    m_BuiltInMeshes[BuiltInMeshEnum::Triangle] = NEW Mesh(triangleMesh, 0.0005f);
    return true;
}
const bool BuiltInMeshses::build_cube_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::Cube])
        return false;
    string cubeMesh;
    {
        cubeMesh =
            "v 1.0 -1.0 -1.0\n"
            "v 1.0 -1.0 1.0\n"
            "v -1.0 -1.0 1.0\n"
            "v -1.0 -1.0 -1.0\n"
            "v 1.0 1.0 -1.0\n"
            "v 1.0 1.0 1.0\n"
            "v -1.0 1.0 1.0\n"
            "v -1.0 1.0 -1.0\n"
            "vt 1.0 0.0\n"
            "vt 0.0 1.0\n"
            "vt 0.0 0.0\n"
            "vt 1.0 0.0\n"
            "vt 0.0 1.0\n"
            "vt 0.0 0.0\n"
            "vt 1.0 0.0\n"
            "vt 0.0 1.0\n"
            "vt 1.0 0.0\n"
            "vt 0.0 1.0\n"
            "vt 0.0 0.0\n"
            "vt 0.0 0.0\n"
            "vt 1.0 1.0\n"
            "vt 1.0 0.0\n"
            "vt 0.0 1.0\n"
            "vt 1.0 1.0\n"
            "vt 1.0 1.0\n"
            "vt 1.0 1.0\n"
            "vt 1.0 0.0\n"
            "vt 1.0 1.0\n"
            "vn 0.0 -1.0 0.0\n"
            "vn 0.0 1.0 0.0\n"
            "vn 1.0 -0.0 0.0\n"
            "vn 0.0 -0.0 1.0\n"
            "vn -1.0 -0.0 -0.0\n"
            "vn 0.0 0.0 -1.0\n"
            "f 2/1/1 4/2/1 1/3/1\n"
            "f 8/4/2 6/5/2 5/6/2\n"
            "f 5/7/3 2/8/3 1/3/3\n"
            "f 6/9/4 3/10/4 2/11/4\n"
            "f 3/12/5 8/13/5 4/2/5\n"
            "f 1/14/6 8/15/6 5/6/6\n"
            "f 2/1/1 3/16/1 4/2/1\n"
            "f 8/4/2 7/17/2 6/5/2\n"
            "f 5/7/3 6/18/3 2/8/3\n"
            "f 6/9/4 7/17/4 3/10/4\n"
            "f 3/12/5 7/19/5 8/13/5\n"
            "f 1/14/6 4/20/6 8/15/6";
    }

    m_BuiltInMeshes[BuiltInMeshEnum::Cube] = NEW Mesh(cubeMesh, 0.0005f);
    return true;
}
const bool BuiltInMeshses::build_plane_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::Plane])
        return false;
    m_BuiltInMeshes[BuiltInMeshEnum::Plane] = NEW Mesh("Plane", 1.0f, 1.0f, 0.0005f);
    return true;
}
const bool BuiltInMeshses::build_font_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::Font])
        return false;
    m_BuiltInMeshes[BuiltInMeshEnum::Font] = NEW Mesh("FontPlane", 1.0f, 1.0f, 0.0005f);
    return true;
}
const bool BuiltInMeshses::build_particle_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::Particle])
        return false;
    m_BuiltInMeshes[BuiltInMeshEnum::Particle] = NEW Mesh("ParticlePlane", 1.0f, 1.0f, 0.0005f);
    return true;
}