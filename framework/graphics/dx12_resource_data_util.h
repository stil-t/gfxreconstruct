/*
** Copyright (c) 2021 LunarG, Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and associated documentation files (the "Software"),
** to deal in the Software without restriction, including without limitation
** the rights to use, copy, modify, merge, publish, distribute, sublicense,
** and/or sell copies of the Software, and to permit persons to whom the
** Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
*/

#ifndef GFXRECON_GRAPHICS_DX12_RESOURCE_DATA_UTIL_H
#define GFXRECON_GRAPHICS_DX12_RESOURCE_DATA_UTIL_H

#include "graphics/dx12_util.h"
#include "util/defines.h"
#include "util/platform.h"

#include <d3d12.h>
#include <vector>

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(graphics)

class Dx12ResourceDataUtil
{
  public:
    Dx12ResourceDataUtil(ID3D12Device* device, uint64_t min_buffer_size);

    virtual ~Dx12ResourceDataUtil() {}

    HRESULT ReadFromResource(ID3D12Resource*                             target_resource,
                             const std::vector<dx12::ResourceStateInfo>& before_states,
                             const std::vector<dx12::ResourceStateInfo>& after_states,
                             std::vector<uint8_t>&                       data,
                             std::vector<uint64_t>&                      subresource_offsets,
                             std::vector<uint64_t>&                      subresource_sizes);

    HRESULT WriteToResource(ID3D12Resource*                             target_resource,
                            const std::vector<dx12::ResourceStateInfo>& before_states,
                            const std::vector<dx12::ResourceStateInfo>& after_states,
                            const std::vector<uint8_t>&                 data,
                            const std::vector<uint64_t>&                subresource_offsets,
                            const std::vector<uint64_t>&                subresource_sizes);

  private:
    enum CopyType : int
    {
        kCopyTypeRead  = 0,
        kCopyTypeWrite = 1
    };

  private:
    // Create a mappable buffer used to copy data to or from another resource via a command list.
    dx12::ID3D12ResourceComPtr GetStagingBuffer(CopyType type, uint64_t required_buffer_size);

    void GetResourceCopyInfo(ID3D12Resource*                                  resource,
                             size_t&                                          subresource_count,
                             std::vector<uint64_t>&                           subresource_offsets,
                             std::vector<uint64_t>&                           subresource_sizes,
                             std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& layouts,
                             uint64_t&                                        total_size);

    // Copy data to or from a mappable resource. Also transitions the resource to after_states.
    bool CopyMappableResource(ID3D12Resource*                             target_resource,
                              const std::vector<dx12::ResourceStateInfo>& before_states,
                              const std::vector<dx12::ResourceStateInfo>& after_states,
                              CopyType                                    copy_type,
                              std::vector<uint8_t>*                       read_data,
                              const std::vector<uint8_t>*                 write_data,
                              const std::vector<uint64_t>&                subresource_offsets,
                              const std::vector<uint64_t>&                subresource_sizes);

    HRESULT ExecuteAndWaitForCommandList();

    // Build and execute a command list that copies data to or from the target_resource.
    HRESULT
    ExecuteCopyCommandList(ID3D12Resource*                                        target_resource,
                           CopyType                                               copy_type,
                           uint64_t                                               copy_size,
                           const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& subresource_layouts,
                           const std::vector<dx12::ResourceStateInfo>&            before_states,
                           const std::vector<dx12::ResourceStateInfo>&            after_states);

    // Build and execute a command list to transition the target resource's subresources from before_states to
    // after_states.
    HRESULT
    ExecuteTransitionCommandList(ID3D12Resource*                             target_resource,
                                 const std::vector<dx12::ResourceStateInfo>& before_states,
                                 const std::vector<dx12::ResourceStateInfo>& after_states);

  private:
    ID3D12Device*                         device_;
    dx12::ID3D12CommandQueueComPtr        command_queue_;
    dx12::ID3D12CommandAllocatorComPtr    command_allocator_;
    dx12::ID3D12GraphicsCommandListComPtr command_list_;
    dx12::ID3D12ResourceComPtr            staging_buffers_[2];
    uint64_t                              staging_buffer_sizes_[2];
    const uint64_t                        min_buffer_size_;

    // Temporary buffers.
    std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> temp_subresource_layouts_;
    std::vector<UINT>                               temp_subresource_row_counts_;
};

GFXRECON_END_NAMESPACE(graphics)
GFXRECON_END_NAMESPACE(gfxrecon)

#endif // GFXRECON_GRAPHICS_DX12_RESOURCE_DATA_UTIL_H