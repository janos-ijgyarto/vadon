#include <VadonEditor/Core/Data/Enum.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

namespace VadonEditor::Core
{
	QString EnumValueData::find_metadata(const char* key) const
	{
		auto metadata_it = metadata.find(key);
		if (metadata_it == metadata.end())
		{
			return QString();
		}

		return metadata_it->constData();
	}

	QString EnumValueData::get_name() const
	{
		QString name = find_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME);
		if (name.isEmpty() == true)
		{
			name = QString("EnumValue_%1").arg(id.toString(QUuid::StringFormat::WithoutBraces));
		}

		return name;
	}

	const EnumValueData* EnumData::find_value_data(const QUuid& value_uuid) const
	{
		auto value_it = values.find(value_uuid);

		if (value_it == values.end())
		{
			// TODO: warning?
			return nullptr;
		}

		return &value_it.value();
	}

	QString EnumData::find_metadata(const char* key) const
	{
		auto metadata_it = metadata.find(key);
		if (metadata_it == metadata.end())
		{
			return QString();
		}

		return metadata_it->constData();
	}

	QString EnumData::get_name() const
	{
		QString name = find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);
		if (name.isEmpty() == true)
		{
			name = QString("Enum_%1").arg(id.toString(QUuid::StringFormat::WithoutBraces));
		}

		return name;
	}
}