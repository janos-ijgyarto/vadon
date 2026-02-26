#ifndef VADONEDITOR_UTILITIES_DATA_TREEMODEL_HPP
#define VADONEDITOR_UTILITIES_DATA_TREEMODEL_HPP
#include <Vadon/Foundation/TypeInfo/TypeInfo.hpp>
#include <QVariant>
namespace VadonEditor::Utilities
{
	QVariant get_base_type_default_value(::Vadon::Foundation::BaseType base_type);
}
#endif