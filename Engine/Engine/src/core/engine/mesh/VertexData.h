#pragma once
#ifndef ENGINE_VERTEX_DATA_H_INCLUDE_GUARD
#define ENGINE_VERTEX_DATA_H_INCLUDE_GUARD

#include <core/engine/mesh/VertexBufferObject.h>
#include <core/engine/mesh/VertexDataFormat.h>
#include "core/engine/renderer/Engine_Renderer.h"
#include <memory>


typedef unsigned short ushort;

struct VertexData {
    VertexDataFormat&                              format;
    std::vector<char*>                             data;
    std::vector<size_t>                            dataSizes;
    std::vector<ushort>                            indices;
    GLuint                                         vao;
    std::vector<std::unique_ptr<BufferObject>>     buffers;

    VertexData() = delete;
    VertexData(const VertexData& other) = delete;
    VertexData& operator=(const VertexData& other) = delete;
    VertexData(VertexData&& other) noexcept = default;
    VertexData& operator=(VertexData&& other) noexcept = default;

    VertexData(const VertexDataFormat& _format) :format(const_cast<VertexDataFormat&>(_format)), vao(0) {
        data.reserve(_format.attributes.size());
        for (size_t i = 0; i < data.capacity(); ++i) { data.emplace_back(nullptr); }
        dataSizes.reserve(_format.attributes.size());
        for (size_t i = 0; i < dataSizes.capacity(); ++i) { dataSizes.emplace_back(0); }
        buffers.push_back(std::make_unique<VertexBufferObject>());
    }
    ~VertexData() {
        Engine::Renderer::deleteVAO(vao);
        for (size_t i = 0; i < data.size(); ++i) { 
            free(data[i]); 
        }
        vector_clear(data);
        vector_clear(dataSizes);
        vector_clear(indices);
    }
    template<typename T> const std::vector<T> getData(size_t attributeIndex) {
        const T* _data = (T*)data[attributeIndex];
        std::vector<T> ret(_data, _data + dataSizes[attributeIndex]);
        return ret;
    }
    template<typename T> void setData(size_t attributeIndex, std::vector<T>& _data, bool addToGPU = false,bool orphan = false) {
        if (buffers.size() == 0)
            buffers.push_back(std::make_unique<VertexBufferObject>());
        if (attributeIndex >= data.size()) return;
        auto& _buffer = *buffers[0];
        dataSizes[attributeIndex] = _data.size();
        free(data[attributeIndex]);
        auto totalSize = (_data.size() * sizeof(T)) + 1;
        data[attributeIndex] = (char*)malloc(totalSize);
        memmove(data[attributeIndex], _data.data(), totalSize);
        if (addToGPU) {
            if (format.interleavingType == VertexAttributeLayout::Interleaved) {
                sendDataToGPU(orphan,-1);
            }else{
                sendDataToGPU(orphan,attributeIndex);
            }
        }
    }
    void setDataIndices(std::vector<ushort>& _data, bool addToGPU = false,bool orphan = false) {
        if (buffers.size() == 1)
            buffers.push_back(std::make_unique<ElementBufferObject>());
        auto& _buffer = *buffers[1];
        indices.swap(_data);
        if (addToGPU) {
            _buffer.generate();
            _buffer.bind();
            !orphan ? _buffer.setData(indices, BufferDataDrawType::Static) : _buffer.setDataOrphan(indices);
        }
    }
    void finalize() {   
        Engine::Renderer::deleteVAO(vao);
        if (Engine::epriv::RenderManager::OPENGL_VERSION >= 30) {
            //build the vao itself
            Engine::Renderer::genAndBindVAO(vao);
            sendDataToGPU(false,-1);
            format.bind(*this);
            Engine::Renderer::bindVAO(0);
        }else{
            sendDataToGPU(false,-1);
            format.bind(*this);
        }
    }
    inline void bind() {
        if (vao) {
            Engine::Renderer::bindVAO(vao);
        }else{ 
            for (auto& buffer : buffers) 
                buffer->bind(); 
            format.bind(*this); 
        }
    }
    inline void unbind() {
        if (vao) { 
            Engine::Renderer::bindVAO(0);
        }else{     
            format.unbind(); 
        }
    }
    void sendDataToGPU(bool orphan,int attributeIndex = -1) {
        auto& _vBuffer = *buffers[0];
        _vBuffer.generate(); _vBuffer.bind();

        char* buffer;
        size_t accumulator = 0;
        size_t size = 0;
        if (format.interleavingType == VertexAttributeLayout::Interleaved) {
            size = (format.attributes[0].stride * dataSizes[0]);
            buffer = (char*)malloc(size);
            for (size_t i = 0; i < dataSizes[0]; ++i) {
                for (size_t j = 0; j < data.size(); ++j) {
                    const auto& sizeofT = format.attributes[j].typeSize;
                    memmove(&buffer[accumulator], &(data[j])[i * sizeofT], sizeofT);
                    accumulator += sizeofT;
                }
            }
            !orphan ? _vBuffer.setData(size, buffer, BufferDataDrawType::Dynamic) : _vBuffer.setDataOrphan(buffer);
        }else{
            if (attributeIndex == -1) {
                for (size_t i = 0; i < data.size(); ++i)
                    size += format.attributes[i].typeSize * dataSizes[i];
                buffer = (char*)malloc(size);
                for (size_t i = 0; i < data.size(); ++i) {
                    auto blockSize = dataSizes[i] * format.attributes[i].typeSize;
                    memmove(&buffer[accumulator], &(data[i])[0], blockSize);
                    accumulator += blockSize;
                }

                !orphan ? _vBuffer.setData(size, buffer, BufferDataDrawType::Dynamic) : _vBuffer.setDataOrphan(buffer);
            }else{
                size += (format.attributes[attributeIndex].typeSize * dataSizes[attributeIndex]);
                buffer = (char*)malloc(size);
                for (size_t i = 0; i < data.size(); ++i) {
                    if (i != attributeIndex) {
                        accumulator += dataSizes[i] * format.attributes[i].typeSize;
                    }else{
                        memmove(&buffer[0], &(data[i])[0], size);
                        break;
                    }
                }
                _vBuffer.setData(size, accumulator, buffer);
            }
        }
        free(buffer);
        if (attributeIndex == -1) {
            auto& _iBuffer = *buffers[1];
            _iBuffer.generate();  _iBuffer.bind();
            _iBuffer.setData(indices, BufferDataDrawType::Static);
        }
    }
};

#endif