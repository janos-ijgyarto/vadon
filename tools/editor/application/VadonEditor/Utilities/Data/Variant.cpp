#include <VadonEditor/Utilities/Data/Variant.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <QColor>
#include <QJsonValue>
#include <QLine>
#include <QUuid>
#include <QVector2D>

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
			return QColor{};
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
		if (value.typeId() == QMetaType::Type::QUuid)
		{
			return Utilities::uuid_to_base64_string(value.toUuid());
		}
		else
		{
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
