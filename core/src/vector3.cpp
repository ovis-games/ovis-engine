#include <fmt/ostream.h>

#include <ovis/core/vector.hpp>

namespace ovis {

std::ostream& operator<<(std::ostream& stream, const Vector3& vector) {
  fmt::print(stream, "{}", vector);
  return stream;
}

std::string VectorToString(Vector3 v) {
  return fmt::format("{}", v);
}

Vector3 LinearInterpolateVector3(Vector3 a, Vector3 b, float t) {
  // TODO: references
  return (1.0f - t) * a + t * b;
}

OVIS_VM_DEFINE_TYPE_BINDING(Core, Vector3) {
  Vector3_type->AddProperty<&Vector3::x>("x");
  Vector3_type->AddProperty<&Vector3::y>("y");
  Vector3_type->AddProperty<&Vector3::z>("z");
  // vector3_type["ZERO"] = sol::property(Vector3::Zero);
  // vector3_type["ONE"] = sol::property(Vector3::One);
  // vector3_type["POSITIVE_X"] = sol::property(Vector3::PositiveX);
  // vector3_type["NEGATIVE_X"] = sol::property(Vector3::NegativeX);
  // vector3_type["POSITIVE_Y"] = sol::property(Vector3::PositiveY);
  // vector3_type["NEGATIVE_Y"] = sol::property(Vector3::NegativeY);
  // vector3_type["POSITIVE_Z"] = sol::property(Vector3::PositiveZ);
  // vector3_type["NEGATIVE_Z"] = sol::property(Vector3::NegativeZ);

  // vector3_type->SetDeserializeFunction(&DeserializeVector3);
  // module->RegisterConstructor<Vector3, float, float, float>("CreateVector3", {"x", "y", "z"}, "vector");
  // module->RegisterConstructor<Vector3, float, float, float>("create_vector3", {"x", "y", "z"}, "vector");
  // vector3_type[sol::meta_function::equal_to] = static_cast<bool (*)(const Vector3&, const Vector3&)>(ovis::operator==);
  // module->RegisterFunction<static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(ovis::operator+)>("vector3_add", {"first vector", "second vector"}, {"vector"});
  // module->RegisterFunction<static_cast<Vector3 (*)(const Vector3&)>(ovis::operator-)>("vector3_negate", {"vector"}, {"vector"});
  // module->RegisterFunction<static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(ovis::operator-)>("vector3_subtract", {"first vector", "second vector"}, {"vector"});
  // module->RegisterFunction<static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(ovis::operator*)>("vector3_multiply", {"first vector", "second vector"}, {"vector"});
  // module->RegisterFunction<static_cast<Vector3 (*)(const Vector3&, float)>(ovis::operator*)>("vector3_multiply_scalar", {"vector", "scalar"}, {"vector"});
  // module->RegisterFunction<&VectorToString>("vector3_to_string", {"vector"}, {"string"});
  // module->RegisterFunction<&ovis::min<Vector3>>("vector3_min", {"first vector", "second vector"}, {"minimum"});
  // module->RegisterFunction<&ovis::max<Vector3>>("vector3_max", {"first vector", "second vector"}, {"maximum"});
  // module->RegisterFunction<&ovis::clamp<Vector3>>("vector3_clamp", {"vector", "min", "max"}, {"clamped vector"});
  // module->RegisterFunction<&SquaredLength<Vector3>>("vector3_squared_length", {"vector"}, {"squared length"});
  // module->RegisterFunction<&Length<Vector3>>("vector3_length", {"vector"}, {"length"});
  // module->RegisterFunction<&Normalize<Vector3>>("vector3_normalize", {"vector"}, {"normalized vector"});
  // module->RegisterFunction<&Dot<Vector3>>("vector3_dot", {"first vector", "second vector"}, {"dot product"});
  // module->RegisterFunction<&Cross>("vector3_cross", {"first vector", "second vector"}, {"cross product"});

  // module->RegisterFunction<&LinearInterpolateVector3>("Linear Interpolate Vector3", {"a", "b", "t"}, {"result"});
}

}
