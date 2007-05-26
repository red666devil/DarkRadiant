/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if !defined(INCLUDED_IBRUSH_H)
#define INCLUDED_IBRUSH_H

#include "inode.h"
#include "generic/constant.h"
#include "generic/callback.h"
#include "math/Vector3.h"
//#include "itexdef.h"

#if 0
class IBrushFace
{
public:
  virtual const char* GetShader() const = 0;
  virtual void SetShader(const char* name) = 0;
  virtual const TextureProjection& GetTexdef() const = 0;
  virtual void GetTexdef(TextureProjection& projection) const = 0;
  virtual void SetTexdef(const TextureProjection& projection) = 0;
  virtual void GetFlags(ContentsFlagsValue& flags) const = 0;
  virtual void SetFlags(const ContentsFlagsValue& flags) = 0;
  virtual void ShiftTexdef(float s, float t) = 0;
  virtual void ScaleTexdef(float s, float t) = 0;
  virtual void RotateTexdef(float angle) = 0;
  virtual void FitTexture(float s_repeat, float t_repeat) = 0;
  virtual bool isDetail() const = 0;
  virtual void setDetail(bool detail) = 0;
};

class IBrush
{
public:
  STRING_CONSTANT(Name, "IBrush");
  virtual void reserve(std::size_t count) = 0;
  virtual void clear() = 0;
  virtual void copy(const IBrush& other) = 0;
  virtual IBrushFace* addPlane(const Vector3& p0, const Vector3& p1, const Vector3& p2, const char* shader, const TextureProjection& projection) = 0;
  virtual const AABB& localAABB() const = 0;
  virtual void removeEmptyFaces() = 0;
};

class IBrushFaceInstance
{
public:
  virtual IBrushFace& getFace() = 0;
  virtual const IBrushFace& getFace() const = 0;
  virtual bool isSelected() const = 0;
  virtual void setSelected(SelectionSystem::EComponentMode mode, bool select) const = 0;
};

class IBrushInstance
{
public:
  STRING_CONSTANT(Name, "IBrushInstance");
  virtual void forEachFaceInstance(const BrushInstanceVisitor& visitor) = 0;
};
#endif

class TexDef;

class _QERFaceData;
typedef Callback1<const _QERFaceData&> BrushFaceDataCallback;

class BrushCreator
{
public:
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "brush");
  virtual scene::INodePtr createBrush() = 0;
  virtual void Brush_forEachFace(scene::INodePtr brush, const BrushFaceDataCallback& callback) = 0;
  virtual bool Brush_addFace(scene::INodePtr brush, const _QERFaceData& faceData) = 0;
};

class Brush;
class IBrushNode
{
public:
	/** greebo: Retrieves the contained Brush from the BrushNode
	 */
	virtual Brush& getBrush() = 0;
};
typedef boost::shared_ptr<IBrushNode> IBrushNodePtr;

inline bool Node_isBrush(scene::INodePtr node) {
	return boost::dynamic_pointer_cast<IBrushNode>(node) != NULL;
}

#include "modulesystem.h"

template<typename Type>
class GlobalModule;
typedef GlobalModule<BrushCreator> GlobalBrushModule;

template<typename Type>
class GlobalModuleRef;
typedef GlobalModuleRef<BrushCreator> GlobalBrushModuleRef;

inline BrushCreator& GlobalBrushCreator()
{
  return GlobalBrushModule::getTable();
}

#endif
