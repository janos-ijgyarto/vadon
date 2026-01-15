#ifndef VADONEDITOR_CORE_PROJECT_DATASCHEMA_HPP
#define VADONEDITOR_CORE_PROJECT_DATASCHEMA_HPP
#include <QUuid>
#include <QList>
namespace VadonEditor::Core
{
	struct PropertyData
	{
		QUuid id;
		QUuid type;
		QString name;
	};

	struct TypeData
	{
		QUuid id;
		QString name;
		QList<PropertyData> properties;
	};

	class DataSchema
	{
	public:
		bool load_schema(const QString& schema_file_path);
	private:

	};
}
#endif