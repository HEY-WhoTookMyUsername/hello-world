#pragma once
#include <Graphics/ResourceTypes.hpp>

namespace Graphics
{
	/*
		Framebuffer/Rendertarget object
		Can have textures attached to it and then render to them
	*/
	class FramebufferRes
	{
	public:
		virtual ~FramebufferRes() = default;
		static Ref<FramebufferRes> Create(class OpenGL* gl);
	public:
		virtual bool AttachTexture(Ref<class TextureRes> tex) = 0;
		virtual bool IsComplete() const = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		virtual uint32 Handle() const = 0;
		// The size of the Framebuffer texture
		virtual Vector2i GetSize() const = 0;
	};

	typedef Ref<FramebufferRes> Framebuffer;

	DEFINE_RESOURCE_TYPE(Framebuffer, FramebufferRes);
}