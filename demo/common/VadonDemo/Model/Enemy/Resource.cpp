#include <VadonDemo/Model/Enemy/Resource.hpp>

#include <VadonDemo/Model/Component.hpp>
#include <VadonDemo/Model/Enemy/Component.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Scene/Resource/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

#include <numbers>

namespace
{
	bool solve_quadratic(float a, float b, float c, float& root_a, float& root_b)
	{
		// If a is 0, then equation is
		// not quadratic, but linear
		if (a == 0) {
			return false;
		}

		float d = b * b - 4 * a * c;
		float sqrt_val = std::sqrtf(std::abs(d));

		if (d > 0) 
		{
			root_a = (-b + sqrt_val) / (2 * a);
			root_b = (-b - sqrt_val) / (2 * a);
			return true;
		}
		else if (d == 0) 
		{
			root_a = -b / (2 * a);
			root_b = root_a;
			return true;
		}

		return false;
	}

	bool lead_target(const Vadon::Math::Vector2& position, float intercept_speed, const Vadon::Math::Vector2& target_pos, const Vadon::Math::Vector2& target_velocity, Vadon::Math::Vector2& target) 
	{ 
		Vadon::Math::Vector2 diff = target_pos - position;
		float root1, root2;   
		if (solve_quadratic(Vadon::Math::Vector::dot(target_velocity, target_velocity) - std::sqrtf(intercept_speed), 2.f * Vadon::Math::Vector::dot(diff, target_velocity), Vadon::Math::Vector::dot(diff, diff), root1, root2))
		{ 
			target = target_pos + root2 * target_velocity; 
			return true; 
		} else 
		{ 
			return false;
		} 
	}
}

namespace VadonDemo::Model
{
	void EnemyDefinition::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<EnemyDefinition, Vadon::Scene::Resource>();

		TypeRegistry::add_property<EnemyDefinition>("score_reward", Vadon::Utilities::MemberVariableBind<&EnemyDefinition::score_reward>().bind_member_getter().bind_member_setter());
	}

	void EnemyMovementDefinition::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;

		ResourceRegistry::register_resource_type<EnemyMovementDefinition, Vadon::Scene::Resource>();
	}

	void EnemyMovementLookahead::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<EnemyMovementLookahead, EnemyMovementDefinition>();

		TypeRegistry::add_property<EnemyMovementLookahead>("lookahead_factor", Vadon::Utilities::MemberVariableBind<&EnemyMovementLookahead::lookahead_factor>().bind_member_getter().bind_member_setter());
	}

	Vadon::Math::Vector2 EnemyMovementLookahead::get_movement_direction(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle enemy, Vadon::ECS::EntityHandle target, float /*delta_time*/) const
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto target_component_tuple = component_manager.get_component_tuple<Transform2D, Velocity2D>(target);

		const Transform2D* target_transform = std::get<Transform2D*>(target_component_tuple);
		VADON_ASSERT(target_transform != nullptr, "Cannot find component!");

		const Velocity2D* target_velocity = std::get<Velocity2D*>(target_component_tuple);
		VADON_ASSERT(target_velocity != nullptr, "Cannot find component!");

		auto enemy_component_tuple = component_manager.get_component_tuple<Transform2D, Velocity2D, EnemyMovement>(enemy);

		const Transform2D* enemy_transform = std::get<Transform2D*>(enemy_component_tuple);
		VADON_ASSERT(enemy_transform != nullptr, "Cannot find component!");

		const Velocity2D* enemy_velocity = std::get<Velocity2D*>(enemy_component_tuple);
		VADON_ASSERT(enemy_velocity != nullptr, "Cannot find component!");

		EnemyMovement* enemy_movement = std::get<EnemyMovement*>(enemy_component_tuple);
		VADON_ASSERT(enemy_movement != nullptr, "Cannot find component!");
		
		const Vadon::Math::Vector2 enemy_to_target = target_transform->position - enemy_transform->position;

		if (Vadon::Math::Vector::length_squared(target_velocity->velocity) > 0.001f)
		{
			Vadon::Math::Vector2 intercept_pos = Vadon::Math::Vector2_Zero;
			if (lead_target(enemy_transform->position, enemy_velocity->top_speed, target_transform->position, target_velocity->velocity, intercept_pos))
			{
				return Vadon::Math::Vector::normalize((Vadon::Math::Vector::normalize(intercept_pos) * lookahead_factor) + ((1.0f - lookahead_factor) * enemy_to_target));
			}
		}

		// Unable to lead target, just chase directly
		return Vadon::Math::Vector::normalize(enemy_to_target);
	}

	void EnemyMovementWeaving::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<EnemyMovementWeaving, EnemyMovementDefinition>();

		TypeRegistry::add_property<EnemyMovementWeaving>("cycle_duration", Vadon::Utilities::MemberVariableBind<&EnemyMovementWeaving::cycle_duration>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<EnemyMovementWeaving>("weave_factor", Vadon::Utilities::MemberVariableBind<&EnemyMovementWeaving::weave_factor>().bind_member_getter().bind_member_setter());
	}

	Vadon::Math::Vector2 VadonDemo::Model::EnemyMovementWeaving::get_movement_direction(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle enemy, Vadon::ECS::EntityHandle target, float delta_time) const
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		const Transform2D* target_transform = component_manager.get_component<Transform2D>(target);
		VADON_ASSERT(target_transform != nullptr, "Cannot find component!");

		auto enemy_component_tuple = component_manager.get_component_tuple<Transform2D, EnemyMovement>(enemy);

		const Transform2D* enemy_transform = std::get<Transform2D*>(enemy_component_tuple);
		VADON_ASSERT(enemy_transform != nullptr, "Cannot find component!");

		EnemyMovement* enemy_movement = std::get<EnemyMovement*>(enemy_component_tuple);
		VADON_ASSERT(enemy_movement != nullptr, "Cannot find component!");

		const Vadon::Math::Vector2 enemy_to_target = Vadon::Math::Vector::normalize(target_transform->position - enemy_transform->position);

		const Vadon::Math::Vector2 right_vector = { enemy_to_target.y, -enemy_to_target.x };
		const float norm_cycle_factor = enemy_movement->data / cycle_duration;

		enemy_movement->data += delta_time;
		if (enemy_movement->data > cycle_duration)
		{
			enemy_movement->data -= cycle_duration;
		}

		Vadon::Math::Vector2 weave_component = right_vector * (std::cosf(norm_cycle_factor * std::numbers::pi_v<float> * 2) > 0 ? 1.0f : -1.0f);
		const float clamped_weave_factor = std::clamp(weave_factor, 0.0f, 0.99f);

		return Vadon::Math::Vector::normalize((enemy_to_target * (1.0f - clamped_weave_factor)) + (weave_component * clamped_weave_factor));
	}

	void EnemyWeaponAttackDefinition::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;

		ResourceRegistry::register_resource_type<EnemyWeaponAttackDefinition, Vadon::Scene::Resource>();
	}

	void EnemyContactDamageDefinition::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<EnemyContactDamageDefinition, Vadon::Scene::Resource>();

		TypeRegistry::add_property<EnemyContactDamageDefinition>("damage", Vadon::Utilities::MemberVariableBind<&EnemyContactDamageDefinition::damage>().bind_member_getter().bind_member_setter());
	}
}