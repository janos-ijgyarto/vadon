#ifndef VADON_RENDER_CANVAS_COMMANDBUFFER_HPP
#define VADON_RENDER_CANVAS_COMMANDBUFFER_HPP
#include <Vadon/Render.hpp>
#include <Vadon/Render/Canvas/Batch.hpp>
#include <Vadon/Render/Canvas/Material.hpp>
#include <Vadon/Render/Canvas/Primitive.hpp>
namespace Vadon::Render::Canvas
{
	class CommandBuffer
	{
	public:
		using DataVector = std::vector<char>;

		// FIXME: allow list versions to make this more efficient?
		VADONRENDER_API void draw_triangle(const Triangle& triangle);
		VADONRENDER_API void draw_rectangle(const Rectangle& rectangle);
		VADONRENDER_API void draw_circle(const Circle& circle);
		VADONRENDER_API void draw_sprite(const Sprite& sprite);

		VADONRENDER_API void add_batch_draw(const BatchDrawCommand& batch_command);

		// NOTE: this follows a D3D-style pattern of setting "render state" which applies to all subsequent operations
		// Might need further revision
		VADONRENDER_API void set_texture(const Texture& texture);
		VADONRENDER_API void set_material(MaterialHandle material_handle);
		VADONRENDER_API void set_render_state(const RenderState& render_state);

		DataVector& get_data() { return m_data; }
		const DataVector& get_data() const { return m_data; }

		size_t get_size() const { return m_data.size(); }
		void clear() { m_data.clear(); }
	protected:
		template<typename T>
		T* allocate()
		{
			size_t prev_size = m_data.size();
			m_data.insert(m_data.end(), sizeof(T), 0);

			return reinterpret_cast<T*>(m_data.data() + prev_size);
		}

		template<typename T>
		void push_data(const T& data)
		{
			const char* data_ptr = reinterpret_cast<const char*>(&data);
			m_data.insert(m_data.end(), data_ptr, data_ptr + sizeof(T));
		}

		DataVector m_data;
	};
}
#endif