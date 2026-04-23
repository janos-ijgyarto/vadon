#ifndef VADONEDITOR_UTILITIES_DATA_TREEMODEL_HPP
#define VADONEDITOR_UTILITIES_DATA_TREEMODEL_HPP
#include <Vadon/Foundation/TypeInfo/TypeInfo.hpp>
#include <QVariant>

class QJsonValue;
class QJsonValueConstRef;
namespace VadonEditor::Utilities
{
	::Vadon::Foundation::BaseType get_base_type_from_qt_typeid(int type_id);
	int get_qt_typeid_from_base_type(::Vadon::Foundation::BaseType base_type);

	QVariant get_base_type_default_value(::Vadon::Foundation::BaseType base_type);

	QVariant get_variant_from_json(int type_id, const QJsonValueConstRef& json_value);
	QJsonValue save_variant_to_json(const QVariant& value);

	QVariant get_variant_from_json_variant(int type_id, const QVariant& json_variant);
	QVariant save_variant_to_json_variant(const QVariant& value);
}
#endif