#pragma once

#include <box2d/b2_draw.h>


namespace ovis {

// class Physics2DDebugLayer : public PrimitiveRenderer, public b2Draw {
//  public:
//   Physics2DDebugLayer();

//   void Render(const RenderContext& render_context) override;

//   void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
//   void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
//   void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override;
//   void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override;
//   void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;
//   void DrawTransform(const b2Transform& xf) override;
//   void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override;

//   inline void SetTransformLineLength(float transform_line_length) { transform_line_length_ = transform_line_length; }
//   inline float GetTransformLineLength() { return transform_line_length_; }

//  private:
//   float transform_line_length_ = 1.0f;
// };

}  // namespace ovis
