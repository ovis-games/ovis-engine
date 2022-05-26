#include <ovis/utils/log.hpp>
#include <ovis/rendering/rendering_viewport.hpp>

namespace ovis {

void RenderingViewport::Render() {
  if (!render_passes_sorted_) {
    SortRenderPasses();
  }

  RenderContext render_context;
  if (camera()) {
    const float aspect_ratio = GetAspectRatio();
    if (camera()->aspect_ratio() != aspect_ratio) {
      camera()->SetAspectRatio(aspect_ratio);
    }

    SceneObject* scene_object = camera()->scene_object();
    SDL_assert(scene_object != nullptr);

    Transform* transform = scene_object->GetComponent<Transform>();

    render_context.view_to_clip_space = camera()->projection_matrix();
    render_context.clip_to_view_space = camera()->inverse_projection_matrix();
    render_context.world_to_view_space =
        transform != nullptr ? transform->world_to_local_matrix() : Matrix3x4::IdentityTransformation();
    render_context.view_to_world_space =
        transform != nullptr ? transform->local_to_world_matrix() : Matrix3x4::IdentityTransformation();
    render_context.world_to_clip_space =
        AffineCombine(render_context.view_to_clip_space, render_context.world_to_view_space);
  } else {
    render_context.view_to_clip_space = custom_view_to_clip_space_;
    render_context.clip_to_view_space = custom_clip_to_view_space_;
    render_context.world_to_view_space = custom_world_to_view_space_;
    render_context.view_to_world_space = custom_view_to_world_space_;
    render_context.world_to_clip_space = AffineCombine(custom_view_to_clip_space_, custom_world_to_view_space_);
  }
  for (RenderPass* render_pass : render_pass_order_) {
    render_pass->Render(render_context);
  }
}

Result<RenderPass*> RenderingViewport::AddRenderPass(TypeId render_pass_type) {
  if (GetRenderPass(render_pass_type)) {
    return Error("Render pass {} already added", main_vm->GetType(render_pass_type)->GetReferenceString());
  }

  auto render_pass = ReferencableValue::Create(main_vm, render_pass_type);
  OVIS_CHECK_RESULT(render_pass);

  render_pass->as<RenderPass>().viewport_ = this;
  if (context()) {
    render_pass->as<RenderPass>().graphics_context_ = context();
    render_pass->as<RenderPass>().CreateResources();
  }
  render_passes_.push_back(std::move(*render_pass));
  render_passes_sorted_ = false;

  return &render_passes_.back().as<RenderPass>();
}

void RenderingViewport::SetGraphicsContext(GraphicsContext* graphics_context) {
  if (graphics_context != graphics_context_) {
    if (graphics_context_ != nullptr) {
      for (auto& render_pass : render_passes_) {
        render_pass.as<RenderPass>().ReleaseResourcesWrapper();
        render_pass.as<RenderPass>().graphics_context_ = nullptr;
      }
    }

    if (graphics_context != nullptr) {
      for (auto& render_pass : render_passes_) {
        render_pass.as<RenderPass>().graphics_context_ = graphics_context;
        render_pass.as<RenderPass>().CreateResourcesWrapper();
      }
    }

    graphics_context_ = graphics_context;
  }
}

void RenderingViewport::ClearResources() {
  render_pass_dependencies_.clear();
  render_passes_.clear();
  render_pass_order_.clear();
  render_targets_.clear();
}

RenderTargetTexture2D* RenderingViewport::CreateRenderTarget2D(const std::string& id,
                                                               const RenderTargetTexture2DDescription& description) {
  if (render_targets_.count(id) == 0) {
    return static_cast<RenderTargetTexture2D*>(
        render_targets_
            .emplace(std::make_pair(id, std::make_unique<RenderTargetTexture2D>(graphics_context_, description)))
            .first->second.get());
  } else {
    LogE("Render target '{}' does already exist");
    return nullptr;
  }
}

RenderTarget* RenderingViewport::GetRenderTarget(const std::string& id) {
  const auto render_target = render_targets_.find(id);
  if (render_target == render_targets_.end()) {
    return nullptr;
  } else {
    return render_target->second.get();
  }
}

std::unique_ptr<RenderTargetConfiguration> RenderingViewport::CreateRenderTargetConfiguration(
    std::vector<std::string> color_render_target_ids, std::string depth_render_target_id) {
  RenderTargetConfigurationDescription render_target_config_desc;
  render_target_config_desc.color_attachments.reserve(color_render_target_ids.size());
  for (const auto& render_target_id : color_render_target_ids) {
    render_target_config_desc.color_attachments.push_back(GetRenderTarget(render_target_id));
    SDL_assert(render_target_config_desc.color_attachments.back() != nullptr);
  }
  if (depth_render_target_id.length() > 0) {
    render_target_config_desc.depth_attachment = GetRenderTarget(depth_render_target_id);
  }
  return std::make_unique<RenderTargetConfiguration>(graphics_context_, render_target_config_desc);
}

void RenderingViewport::SortRenderPasses() {
  // First depends on second beeing already rendered
  std::multimap<TypeId, TypeId> dependencies = render_pass_dependencies_;
  std::set<TypeId> render_passes_left_;

  for (const auto& render_pass : render_passes_) {
    render_passes_left_.insert(render_pass.type_id());

    for (auto render_before : render_pass.as<const RenderPass>().render_before_list_) {
      if (!GetRenderPass(render_before)) {
        LogW("Cannot render {0} before {1}, {1} not found!", render_pass.type()->GetReferenceString(),
             main_vm->GetType(render_before)->GetReferenceString());
      } else {
        dependencies.insert(std::make_pair(render_before, render_pass.type_id()));
      }
    }

    for (auto render_after : render_pass.as<const RenderPass>().render_after_list_) {
      if (!GetRenderPass(render_after)) {
        LogW("Cannot render {0} after {1}, {1} not found!", render_pass.type()->GetReferenceString(),
             main_vm->GetType(render_after)->GetReferenceString());
      } else {
        dependencies.insert(std::make_pair(render_pass.type_id(), render_after));
      }
    }
  }

  LogV("Sorting render passes:");
  render_pass_order_.clear();
  render_pass_order_.reserve(render_passes_.size());
  while (render_passes_left_.size() > 0) {
    auto next = std::find_if(render_passes_left_.begin(), render_passes_left_.end(),
                             [&dependencies](TypeId type) { return dependencies.count(type) == 0; });

    SDL_assert(next != render_passes_left_.end());
    LogV(" {}", main_vm->GetType(*next)->GetReferenceString());

    assert(GetRenderPass(*next));
    render_pass_order_.push_back(GetRenderPass(*next));
    for (auto i = dependencies.begin(), e = dependencies.end(); i != e;) {
      if (i->second == *next) {
        i = dependencies.erase(i);
      } else {
        ++i;
      }
    }
    render_passes_left_.erase(next);
  }

  LogV("Render passes sorted!");
  render_passes_sorted_ = true;
}

}  // namespace ovis
