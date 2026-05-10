#include <VadonDemo/Model/Enemy/Resource.hpp>

#include <VadonDemo/Model/Component.hpp>
#include <VadonDemo/Model/Enemy/Component.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Model/Resource/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>
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
	void EnemyDefinition::register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using ResourceRegistry = Vadon::Model::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<EnemyDefinition, Vadon::Model::Resource>();

		TypeRegistry::add_property<EnemyDefinition>(VADON_GET_MEMBER_UUID(EnemyDefinition, score_reward), Vadon::Utilities::MemberVariableBind<&EnemyDefinition::score_reward>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(EnemyDefinition))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::EnemyDefinition")
			.add_property(VADON_GET_MEMBER_UUID(EnemyDefinition, score_reward))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Score Reward")
				.commit_property();
	}

	void EnemyMovementDefinition::register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using ResourceRegistry = Vadon::Model::ResourceRegistry;

		ResourceRegistry::register_resource_type<EnemyMovementDefinition, Vadon::Model::Resource>();

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(EnemyMovementDefinition))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::EnemyMovementDefinition");
	}

	void EnemyMovementLookahead::register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using ResourceRegistry = Vadon::Model::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<EnemyMovementLookahead, EnemyMovementDefinition>();

		TypeRegistry::add_property<EnemyMovementLookahead>(VADON_GET_MEMBER_UUID(EnemyMovementLookahead, lookahead_factor), Vadon::Utilities::MemberVariableBind<&EnemyMovementLookahead::lookahead_factor>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(EnemyMovementLookahead))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::EnemyMovementLookahead")
			.add_property(VADON_GET_MEMBER_UUID(EnemyMovementLookahead, lookahead_factor))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Lookahead Factor")
				.commit_property();
	}

	Vadon::Math::Vector2 EnemyMovementLookahead::get_movement_direction(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle enemy, Vadon::ECS::EntityHandle target, float /*delta_time*/) const
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		const auto target_transform = component_manager.get_component<Transform2D>(target);
		VADON_ASSERT(target_transform.is_valid() == true, "Cannot find component!");

		const auto target_velocity = component_manager.get_component<Velocity2D>(target);
		VADON_ASSERT(target_velocity.is_valid() == true, "Cannot find component!");

		const auto enemy_transform = component_manager.get_component<Transform2D>(enemy);
		VADON_ASSERT(enemy_transform.is_valid() == true, "Cannot find component!");

		const auto enemy_velocity = component_manager.get_component<Velocity2D>(enemy);
		VADON_ASSERT(enemy_velocity.is_valid() == true, "Cannot find component!");

		auto enemy_movement = component_manager.get_component<EnemyMovement>(enemy);
		VADON_ASSERT(enemy_movement.is_valid() == true, "Cannot find component!");
		
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

	void EnemyMovementWeaving::register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using ResourceRegistry = Vadon::Model::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<EnemyMovementWeaving, EnemyMovementDefinition>();

		TypeRegistry::add_property<EnemyMovementWeaving>(VADON_GET_MEMBER_UUID(EnemyMovementWeaving, cycle_duration), Vadon::Utilities::MemberVariableBind<&EnemyMovementWeaving::cycle_duration>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<EnemyMovementWeaving>(VADON_GET_MEMBER_UUID(EnemyMovementWeaving, weave_factor), Vadon::Utilities::MemberVariableBind<&EnemyMovementWeaving::weave_factor>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(EnemyMovementWeaving))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::EnemyMovementWeaving")
			.add_property(VADON_GET_MEMBER_UUID(EnemyMovementWeaving, cycle_duration))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Cycle Duration")
				.commit_property()
			.add_property(VADON_GET_MEMBER_UUID(EnemyMovementWeaving, weave_factor))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Weave Factor")
				.commit_property();
	}

	Vadon::Math::Vector2 VadonDemo::Model::EnemyMovementWeaving::get_movement_direction(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle enemy, Vadon::ECS::EntityHandle target, float delta_time) const
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		const auto target_transform = component_manager.get_component<Transform2D>(target);
		VADON_ASSERT(target_transform.is_valid() == true, "Cannot find component!");

		const auto enemy_transform = component_manager.get_component<Transform2D>(enemy);
		VADON_ASSERT(enemy_transform.is_valid() == true, "Cannot find component!");

		auto enemy_movement = component_manager.get_component<EnemyMovement>(enemy);
		VADON_ASSERT(enemy_movement.is_valid() == true, "Cannot find component!");

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

	void EnemyWeaponAttackDefinition::register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using ResourceRegistry = Vadon::Model::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<EnemyWeaponAttackDefinition, Vadon::Model::Resource>();

		TypeRegistry::add_property<EnemyWeaponAttackDefinition>(VADON_GET_MEMBER_UUID(EnemyWeaponAttackDefinition, projectile_count), Vadon::Utilities::MemberVariableBind<&EnemyWeaponAttackDefinition::projectile_count>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<EnemyWeaponAttackDefinition>(VADON_GET_MEMBER_UUID(EnemyWeaponAttackDefinition, reload_time), Vadon::Utilities::MemberVariableBind<&EnemyWeaponAttackDefinition::reload_time>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(EnemyWeaponAttackDefinition))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::EnemyWeaponAttackDefinition")
			.add_property(VADON_GET_MEMBER_UUID(EnemyWeaponAttackDefinition, projectile_count))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Projectile Count")
				.commit_property()
			.add_property(VADON_GET_MEMBER_UUID(EnemyWeaponAttackDefinition, reload_time))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Reload Time")
				.commit_property();
	}

	void EnemyContactDamageDefinition::register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using ResourceRegistry = Vadon::Model::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<EnemyContactDamageDefinition, Vadon::Model::Resource>();

		TypeRegistry::add_property<EnemyContactDamageDefinition>(VADON_GET_MEMBER_UUID(EnemyContactDamageDefinition, damage), Vadon::Utilities::MemberVariableBind<&EnemyContactDamageDefinition::damage>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(EnemyContactDamageDefinition))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::EnemyContactDamageDefinition")
			.add_property(VADON_GET_MEMBER_UUID(EnemyContactDamageDefinition, damage))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Damage")
				.commit_property();
	}
}