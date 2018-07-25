/*
Apache2 License Notice
Copyright 2017 Alex Barry

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <glm/glm.hpp>

#ifndef SRC_MODEL_INCLUDE_TRANSFORM_INTERFACE_H_
#define SRC_MODEL_INCLUDE_TRANSFORM_INTERFACE_H_

// Transform
// Stores a translation and rotation which is the relationship between the
// scene origin/axis and the device origin/axis
class TransformInterface {
 public:
  virtual ~TransformInterface() {}

  // Access the underlying transform matrix
  virtual glm::mat4 get_transform_vector() const = 0;

  // Translations
  virtual double translation(int index) = 0;
  virtual bool has_translation() const = 0;
  virtual void translate(int index, double amt) = 0;

  // Rotations
  virtual double rotation(int index) = 0;
  virtual bool has_rotation() const = 0;
  virtual void rotate(int index, double amt) = 0;

  // Add transforms together
  virtual void add_transform(TransformInterface *t, bool inverted) = 0;
  virtual void add_transform(TransformInterface *t) = 0;
  // Invert a transform
  virtual void invert() = 0;
  // Clear the Transformation
  virtual void clear() = 0;
  // Print the transform to the logs
  virtual void print() = 0;
};

#endif  // SRC_MODEL_INCLUDE_TRANSFORM_INTERFACE_H_
