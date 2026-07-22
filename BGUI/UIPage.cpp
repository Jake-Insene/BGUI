#include "BGUI/UIPage.hpp"

#include <engine/engine.h>
#include <input/input.h>
#include <math/projection.h>
#include <resource/texture.h>
#include <resource/font.h>


namespace BGUI
{

UIPage::UIPage(Mem::Allocator* allocator, GPU::TextureFormat render_attachment_format) :
data{
    .allocator = allocator,
    .batcher = UIElementBatch(allocator, render_attachment_format),
}
{
}

UIPage::~UIPage()
{
}

void UIPage::batch(Widget* widget)
{
    if (!widget)
    {
        return;
    }

    (void)widget->measure();
    widget->layout(Vector2());

    f32 width = widget->get_global_rect().size.width;
    f32 height = widget->get_global_rect().size.height;
    
    Mat4 ortho_projection = Projection::orthographic(0.0f, width, 0.0f, height, -1.0f, 1.0f);

    data.batcher.begin(ortho_projection);
    widget->draw(data.batcher);
    data.batcher.end();
}

void UIPage::flush_pass(Basic::FrameContext& context, Basic::RenderGraph& graph)
{
    Slice batches = data.batcher.get_batches();
    Slice primitives = data.batcher.get_vertices();

    Basic::TransientAllocation transient = context.allocate_transient_vertex(
        primitives.len * sizeof(UIElementBatch::Vertex));
    Mem::copy(transient.mapped, Mem::to_const_bytes(primitives));

    graph.add_raster_pass(
        [](Basic::PassBuilder& builder)
        {
            builder.write_render_attachment(
                Basic::RenderTargetHandle::backbuffer(),
                GPU::LoadOp::Clear,
                GPU::StoreOp::Store,
                GPU::ClearValue::rgba(0, 0, 0, 1));
        },
        [batches, transient](Basic::PassResources& resources)
        {
            for(UIElementBatch::Batch& batch : batches)
            {
                GPU::DescriptorSetID set = resources.context.allocate_descriptor_set(batch.set_layout);
                
                const GPU::WriteDescriptorInfo write_infos[] =
                {
                    GPU::WriteDescriptorInfo::combined_texture_sampler(set, 0, 0, Slice(&batch.texture, 1)),
                };

                GPU::descriptor_set_update_descriptors(resources.context.data.render_device->get_device(),
                    { .write_infos = write_infos });

                GPU::command_buffer_bind_pipeline(resources.command_buffer,
                    GPU::PipelineBindPoint::Graphics, batch.pipeline);

                GPU::command_buffer_bind_descriptor_sets(resources.command_buffer,
                    GPU::PipelineBindPoint::Graphics, batch.pipeline_layout, 0, Slice(&set, 1));

                GPU::command_buffer_constant_block(resources.command_buffer,
                    batch.pipeline_layout, GPU::ShaderStage::Vertex, 0, sizeof(UIElementBatch::BatchBlock),
                    reinterpret_cast<MemoryAddress>(&batch.block));

                GPU::command_buffer_bind_vertex_buffers(resources.command_buffer,
                    0, Slice(&resources.global_device_vertex_buffer, 1),
                    Slice(&transient.offset, 1));

                GPU::command_buffer_draw(resources.command_buffer,
                    batch.vertex_count, 1, batch.vb_offset, 0);
            }
        }
    );
}

}