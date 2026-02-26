#include <VadonEditor/Utilities/Data/Variant.hpp>

#include <QColor>
#include <QLine>
#include <QUuid>
#include <QVector2D>

namespace VadonEditor::Utilities
{
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
}
