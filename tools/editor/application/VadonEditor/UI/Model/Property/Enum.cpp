#include <VadonEditor/UI/Model/Property/Enum.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

namespace VadonEditor::UI
{
	PropertyEnum::PropertyEnum(Core::Application& application, const QUuid& id, const QUuid& enum_type_id, const QUuid& init_value, QWidget* parent)
		: PropertyWidget(id, init_value, parent)
		, m_application(application)
		, m_enum_type_id(enum_type_id)
	{
		m_ui.setupUi(this);

		const Core::DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		const Core::EnumData* enum_data = data_schema.find_enum_data(enum_type_id);
		if (enum_data == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::PropertyEnum::PropertyEnum", "Cannot find enum data");
			return;
		}

		QList<QUuid> value_list = enum_data->values.keys();
		std::sort(value_list.begin(), value_list.end());

		int init_value_index = -1;
		for (int value_index = 0; value_index < value_list.size(); ++value_index)
		{
			const QUuid& current_value = value_list[value_index];
			if (current_value == init_value)
			{
				init_value_index = value_index;
			}

			const Core::EnumValueData* enum_value_data = enum_data->find_value_data(current_value);
			if (enum_value_data == nullptr)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::PropertyEnum::PropertyEnum", "Cannot find enum value data");
				return;
			}

			m_ui.comboBox->addItem(enum_value_data->get_name(), current_value);
		}

		if (init_value_index < 0)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::PropertyEnum::PropertyEnum", "Cannot find init value");
			return;
		}
	}

	void PropertyEnum::set_read_only(bool read_only)
	{
		m_ui.comboBox->setEnabled(read_only == false);
	}

	void PropertyEnum::set_enum_value(const QUuid& value)
	{
		for (int value_index = 0; value_index < m_ui.comboBox->count(); ++value_index)
		{
			const QUuid current_value_id = m_ui.comboBox->itemData(value_index).toUuid();
			if (value == current_value_id)
			{
				m_ui.comboBox->setCurrentIndex(value_index);
				return;
			}
		}

		Q_UNREACHABLE();
	}

	void PropertyEnum::enum_value_selected(int index)
	{
		const QUuid selected_enum_value_id = m_ui.comboBox->itemData(index).toUuid();
		internal_set_value(selected_enum_value_id);
	}
}