#include <VadonEditor/Utilities/Data/Variant.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <QColor>
#include <QJsonArray>
#include <QJsonValue>
#include <QLine>
#include <QUuid>
#include <QVector2D>

namespace
{
	QJsonArray json_to_math_vec_array(const QJsonValueConstRef& json_value, int dimensions)
	{
		Q_ASSERT_X(json_value.isArray() == true, "json_to_math_vec_array", "Invalid data");
		
		QJsonArray vec_array = json_value.toArray();
		
		Q_ASSERT_X(vec_array.size() == dimensions, "json_to_math_vec_array", "Invalid data");

		return vec_array;
	}
}

namespace VadonEditor::Utilities
{
	::Vadon::Foundation::BaseType get_base_type_from_qt_typeid(int type_id)
	{
		switch (type_id)
		{
		case QMetaType::Type::Int:
			return ::Vadon::Foundation::BaseType::INT32;
		case QMetaType::Type::UInt:
			return ::Vadon::Foundation::BaseType::UINT32;
		case QMetaType::Type::Double:
			return ::Vadon::Foundation::BaseType::FLOAT;
		case QMetaType::Type::Bool:
			return ::Vadon::Foundation::BaseType::BOOL;
		case QMetaType::Type::QString:
			return ::Vadon::Foundation::BaseType::STRING;
		case QMetaType::Type::QVector2D:
			return ::Vadon::Foundation::BaseType::VECTOR2;
		case QMetaType::Type::QPoint:
			return ::Vadon::Foundation::BaseType::VECTOR2I;
		case QMetaType::Type::QVector3D:
			return ::Vadon::Foundation::BaseType::VECTOR3;
		case QMetaType::Type::QLine:
			return ::Vadon::Foundation::BaseType::VECTOR3I;
		case QMetaType::Type::QVector4D:
			return ::Vadon::Foundation::BaseType::VECTOR4;
		case QMetaType::Type::QColor:
			return ::Vadon::Foundation::BaseType::COLORRGBA;
		case QMetaType::Type::QUuid:
			return ::Vadon::Foundation::BaseType::UUID;
		case QMetaType::Type::QVariantList:
			return ::Vadon::Foundation::BaseType::ARRAY;
		case QMetaType::Type::QVariantMap:
			return ::Vadon::Foundation::BaseType::DICTIONARY;
		}

		return ::Vadon::Foundation::BaseType::INVALID;
	}

	int get_qt_typeid_from_base_type(::Vadon::Foundation::BaseType base_type)
	{
		switch (base_type)
		{
		case ::Vadon::Foundation::BaseType::INT32:
			return QMetaType::Type::Int;
		case ::Vadon::Foundation::BaseType::UINT32:
			return QMetaType::Type::UInt;
		case ::Vadon::Foundation::BaseType::FLOAT:
			return QMetaType::Type::Float;
		case ::Vadon::Foundation::BaseType::BOOL:
			return QMetaType::Type::Bool;
		case ::Vadon::Foundation::BaseType::STRING:
			return QMetaType::Type::QString;
		case ::Vadon::Foundation::BaseType::VECTOR2:
			return QMetaType::Type::QVector2D;
		case ::Vadon::Foundation::BaseType::VECTOR2I:
			return QMetaType::Type::QPoint;
		case ::Vadon::Foundation::BaseType::VECTOR3:
			return QMetaType::Type::QVector3D;
		case ::Vadon::Foundation::BaseType::VECTOR3I:
			return QMetaType::Type::QLine;
		case ::Vadon::Foundation::BaseType::VECTOR4:
			return QMetaType::Type::QVector4D;
		case ::Vadon::Foundation::BaseType::COLORRGBA:
			return QMetaType::Type::QColor;
		case ::Vadon::Foundation::BaseType::UUID:
			return QMetaType::Type::QUuid;
		case ::Vadon::Foundation::BaseType::ARRAY:
			return QMetaType::Type::QVariantList;
		case ::Vadon::Foundation::BaseType::DICTIONARY:
			return QMetaType::Type::QVariantMap;
		}

		return QMetaType::Type::UnknownType;
	}

	QVariant get_base_type_default_value(::Vadon::Foundation::BaseType base_type)
	{
		switch (base_type)
		{
		case ::Vadon::Foundation::BaseType::INT32:
			return 0;
		case ::Vadon::Foundation::BaseType::UINT32:
			return 0u;
		case ::Vadon::Foundation::BaseType::FLOAT:
			return 0.0f;
		case ::Vadon::Foundation::BaseType::BOOL:
			return false;
		case ::Vadon::Foundation::BaseType::STRING:
			return "";
		case ::Vadon::Foundation::BaseType::VECTOR2:
			return QVector2D{};
		case ::Vadon::Foundation::BaseType::VECTOR2I:
			return QPoint{};
		case ::Vadon::Foundation::BaseType::VECTOR3:
			return QVector3D{};
		case ::Vadon::Foundation::BaseType::VECTOR3I:
			return QLine{};
		case ::Vadon::Foundation::BaseType::VECTOR4:
			return QVector4D{};
		case ::Vadon::Foundation::BaseType::COLORRGBA:
			return QColor(0, 0, 0, 0);
		case ::Vadon::Foundation::BaseType::UUID:
			return QUuid{};
		case ::Vadon::Foundation::BaseType::ARRAY:
			return QVariantList{};
		case ::Vadon::Foundation::BaseType::DICTIONARY:
			return QVariantHash{};
		}

		return QVariant();
	}

	QVariant get_variant_from_json(int type_id, const QJsonValueConstRef& json_value)
	{
		switch (type_id)
		{
		case QMetaType::Type::Int:
		case QMetaType::Type::UInt:
		case QMetaType::Type::Float:
		case QMetaType::Type::Double:
		{
			Q_ASSERT_X(json_value.isDouble() == true, "VadonEditor::Utilities::get_variant_from_json", "Invalid type");
			switch (type_id)
			{
			case QMetaType::Type::Int:
				return QVariant(json_value.toInt());
			case QMetaType::Type::UInt:
				return QVariant(static_cast<unsigned int>(json_value.toInt()));
			case QMetaType::Type::Float:
				return QVariant(static_cast<float>(json_value.toDouble()));
			case QMetaType::Type::Double:
				return json_value.toDouble();
			default:
				Q_UNREACHABLE();
			}
		}
		break;
		case QMetaType::Type::Bool:
			return json_value.toBool();
		case QMetaType::Type::QVector2D:
		{
			const QJsonArray vec2_array = json_to_math_vec_array(json_value, 2);
			return QVector2D(static_cast<float>(vec2_array[0].toDouble()), static_cast<float>(vec2_array[1].toDouble()));
		}
		case QMetaType::Type::QPoint:
		{
			const QJsonArray vec2i_array = json_to_math_vec_array(json_value, 2);
			return QPoint(vec2i_array[0].toInt(), vec2i_array[1].toInt());
		}
		case QMetaType::Type::QVector3D:
		{
			const QJsonArray vec3_array = json_to_math_vec_array(json_value, 3);
			return QVector3D(static_cast<float>(vec3_array[0].toDouble()), static_cast<float>(vec3_array[1].toDouble()),
				static_cast<float>(vec3_array[2].toDouble()));
		}
		case QMetaType::Type::QLine:
		{
			const QJsonArray vec3i_array = json_to_math_vec_array(json_value, 3);
			return QLine(vec3i_array[0].toInt(), vec3i_array[1].toInt(), vec3i_array[2].toInt(), 0);
		}
		case QMetaType::Type::QVector4D:
		{
			const QJsonArray vec4_array = json_to_math_vec_array(json_value, 4);
			return QVector4D(static_cast<float>(vec4_array[0].toDouble()), static_cast<float>(vec4_array[1].toDouble()),
				static_cast<float>(vec4_array[2].toDouble()), static_cast<float>(vec4_array[3].toDouble()));
		}
		case QMetaType::Type::QColor:
		{
			// Colors are stored as #AARRGGBB strings
			Q_ASSERT_X(json_value.isString() == true, "VadonEditor::Utilities::get_variant_from_json", "Invalid type");
			return QColor::fromString(json_value.toString());
		}
		case QMetaType::Type::QUuid:
		{
			// UUIDs are stored as base64 strings
			Q_ASSERT_X(json_value.isString() == true, "VadonEditor::Utilities::get_variant_from_json", "Invalid type");
			return Utilities::base64_string_to_uuid(json_value.toString());
		}
		case QMetaType::Type::QString:
		{
			Q_ASSERT_X(json_value.isString() == true, "VadonEditor::Utilities::get_variant_from_json", "Invalid type");
			return json_value.toString();
		}
		// TODO: add deserialization for Vector2D, etc.?
		}

		Q_UNREACHABLE_RETURN(QVariant());
	}

	QJsonValue save_variant_to_json(const QVariant& value)
	{
		switch (value.typeId())
		{
		case QMetaType::Type::QUuid:
			return Utilities::uuid_to_base64_string(value.toUuid());
		case QMetaType::Type::QVector2D:
		{
			const QVector2D vec2_value = value.value<QVector2D>();
			return QJsonArray{ vec2_value.x(), vec2_value.y() };
		}
		case QMetaType::Type::QPoint:
		{
			const QPoint point_value = value.value<QPoint>();
			return QJsonArray{ point_value.x(), point_value.y() };
		}
		case QMetaType::Type::QVector3D:
		{
			const QVector3D vec3_value = value.value<QVector3D>();
			return QJsonArray{ vec3_value.x(), vec3_value.y(), vec3_value.z() };
		}
		case QMetaType::Type::QLine:
		{
			const QLine line_value = value.value<QLine>();
			return QJsonArray{ line_value.x1(), line_value.y1(), line_value.x2() };
		}
		case QMetaType::Type::QVector4D:
		{
			const QVector4D vec4_value = value.value<QVector4D>();
			return QJsonArray{ vec4_value.x(), vec4_value.y(), vec4_value.z(), vec4_value.w() };
		}
		case QMetaType::Type::QColor:
		{
			const QColor color_value = value.value<QColor>();
			return color_value.name(QColor::NameFormat::HexArgb);
		}
		default:
			return QJsonValue::fromVariant(value);
		}
	}

	QVariant get_variant_from_json_variant(int type_id, const QVariant& json_variant)
	{
		switch (type_id)
		{
		case QMetaType::Type::Int:
		case QMetaType::Type::UInt:
		case QMetaType::Type::Float:
		case QMetaType::Type::Double:
		{
			Q_ASSERT_X(json_variant.typeId() == QJsonValue::Type::Double, "VadonEditor::Utilities::get_variant_from_json_variant", "Invalid type");
			switch (type_id)
			{
			case QMetaType::Type::Int:
				return QVariant(json_variant.toInt());
			case QMetaType::Type::UInt:
				return QVariant(static_cast<unsigned int>(json_variant.toInt()));
			case QMetaType::Type::Float:
				return QVariant(static_cast<float>(json_variant.toDouble()));
			case QMetaType::Type::Double:
				return json_variant.toDouble();
			default:
				Q_UNREACHABLE();
			}
		}
		break;
		default:
			// Return the value as-is
			return json_variant;
		}
	}

	QVariant save_variant_to_json_variant(const QVariant& value)
	{
		// NOTE: any other conversions?
		return value;
	}
}
