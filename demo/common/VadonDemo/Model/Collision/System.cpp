#include <VadonDemo/Model/Collision/System.hpp>

#include <VadonDemo/Model/Component.hpp>

#include <Vadon/ECS/World/World.hpp>

namespace
{
	bool collision_test(const Vadon::Math::Vector2& pos_a, float radius_a, const Vadon::Math::Vector2& pos_b, float radius_b)
	{
		const float hit_radius = radius_a + radius_b;
		const float hit_radius_sq = hit_radius * hit_radius;

		const Vadon::Math::Vector2 a_to_b = pos_b - pos_a;
		const float dist_sq = Vadon::Math::Vector::dot(a_to_b, a_to_b);

		return dist_sq < hit_radius_sq;
	}
}

namespace VadonDemo::Model
{
	CollisionSystem::CollisionSystem(Core::Core& core)
		: m_core(core)
	{

	}

	void CollisionSystem::register_types()
	{
		Collision::register_component();
	}

	void CollisionSystem::update(Vadon::ECS::World& ecs_world, float /*delta_time*/)
	{
		struct CollisionData
		{
			Vadon::ECS::EntityHandle entity;
			uint8_t layers = 0;
			VadonDemo::Model::CollisionCallback callback = nullptr;

			Vadon::Math::Vector2 position;
			float radius = 0.0f;
			// TODO: some other metadata?
		};
		std::vector<CollisionData> collision_data_vec;

		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto collision_query = component_manager.run_component_query<Transform2D&, Collision&>();
		for (auto collision_it = collision_query.get_iterator(); collision_it.is_valid() == true; collision_it.next())
		{
			auto collision_tuple = collision_it.get_tuple();
			const Transform2D& current_transform = std::get<Transform2D&>(collision_tuple);
			const Collision& current_collision = std::get<Collision&>(collision_tuple);

			CollisionData& collision_data = collision_data_vec.emplace_back();
			collision_data.entity = collision_it.get_entity();
			collision_data.callback = current_collision.callback;
			collision_data.layers = current_collision.layers;

			collision_data.position = current_transform.position;
			collision_data.radius = current_transform.scale * current_collision.radius;
		}

		for (size_t outer_index = 0; outer_index < collision_data_vec.size(); ++outer_index)
		{
			const CollisionData& first_collider = collision_data_vec[outer_index];
			for (size_t inner_index = outer_index + 1; inner_index < collision_data_vec.size(); ++inner_index)
			{
				const CollisionData& second_collider = collision_data_vec[inner_index];
				if ((first_collider.layers & second_collider.layers) == 0)
				{
					continue;
				}

				if (collision_test(first_collider.position, first_collider.radius, second_collider.position, second_collider.radius) == false)
				{
					continue;
				}

				// Collision detected, run callbacks
				if (first_collider.callback != nullptr)
				{
					first_collider.callback(ecs_world, first_collider.entity, second_collider.entity);
				}

				if (second_collider.callback != nullptr)
				{
					second_collider.callback(ecs_world, second_collider.entity, first_collider.entity);
				}
			}
		}
	}
}